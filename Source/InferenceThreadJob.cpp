/*
  ==============================================================================

    InferenceThreadJob.cpp
    Created: 2 Oct 2021 1:54:57pm
    Author:  Andrew Fyfe
    Updated: 2025-09-04 (integrated lockfree param queue + GUI visualizer)

  ==============================================================================
*/

#include "InferenceThreadJob.h"
#include <onnxruntime_cxx_api.h>
#include "AudioBufferSampler.h"
#include "PluginEditor.h"

InferenceThreadJob::InferenceThreadJob(NewPluginTemplateAudioProcessor& processor)
: ThreadPoolJob("InferenceThreadPoolJob"), mProcessor(processor)
{
}

InferenceThreadJob::~InferenceThreadJob()
{
}

auto InferenceThreadJob::runJob() -> JobStatus
{
    if (shouldExit())
        return JobStatus::jobNeedsRunningAgain;

    // === Load ONNX model session ===
    Ort::Env env{OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, "InferenceThreadJob"};
    Ort::SessionOptions options_ort;

    File modelFile = mProcessor.getModelFile();
    String juceModelPath = modelFile.getFullPathName();

   #ifdef _WIN32
    std::wstring wide_path = juceModelPath.toWideCharPointer();
    Ort::Session session_(env, wide_path.c_str(), options_ort);
   #else
    std::string model_path = juceModelPath.toStdString();
    Ort::Session session_(env, model_path.c_str(), options_ort);
   #endif

    if (!modelFile.existsAsFile() || modelFile.getFileExtension() != ".onnx")
    {
        DBG("invalid model file format");
        return JobStatus::jobHasFinished;
    }

    // === Get latest params from queue ===
    std::vector<float> params;
    if (mProcessor.mParamQueue && mProcessor.mParamQueue->getNumReady() > 0)
    {
        params = mProcessor.mParamQueue->getLatestOnly();
    }

    if (params.empty())
        return JobStatus::jobHasFinished;


    const float attackVal     = params[0];
    const float releaseVal    = params[1];
    const float brightnessVal = params[2];
    const float hardnessVal   = params[3];
    const float depthVal      = params[4];
    const float roughnessVal  = params[5];
    const float boominessVal  = params[6];
    const float warmthVal     = params[7];
    const float sharpnessVal  = params[8];

    if (shouldExit())
        return JobStatus::jobNeedsRunningAgain;

    // === Prepare input/output tensors ===
    Ort::Value env_tensor_{nullptr};
    std::array<int64_t, 3> env_shape_{16, 16000, 1};

    Ort::Value params_tensor_{nullptr};
    std::array<int64_t, 2> params_shape_{16, 7};

    Ort::Value is_train_tensor_{nullptr};
    std::array<int64_t, 1> is_train_shape_{1};

    Ort::Value output_tensor_{nullptr};
    std::array<int64_t, 2> output_shape_{16, 16000};

    std::vector<float> input_env(16 * 16000);
    std::vector<float> input_params(16 * 7);
    bool input_is_train = false;
    std::vector<float> results_(16 * 16000);
    std::vector<Ort::Value> input_tensors;

    // === Fill params (brightness..sharpness) ===
    const std::array<float, 7> param_vals { brightnessVal,
                                            hardnessVal,
                                            depthVal,
                                            roughnessVal,
                                            boominessVal,
                                            warmthVal,
                                            sharpnessVal };

    int counter_params = 0;
    while (counter_params < (int) input_params.size()) {
        for (int i = 0; i < (int) param_vals.size(); ++i) {
            input_params[counter_params + i] = param_vals[i];
        }
        counter_params += (int) param_vals.size();
    }

    // === Fill envelope (attack + release) ===
    const int size = 16000;
    const float endVal = 1.f;
    float currentVal = 0.f;
    int counter_env = 0;
    float stepAttack  = (attackVal > 0.0f ? endVal / (attackVal * size) : endVal);
    float stepRelease = (releaseVal > 0.0f ? endVal / (releaseVal * size) : endVal);

    std::fill(input_env.begin(), input_env.end(), 0.f);

    while (counter_env < (int) input_env.size()) {
        for (int i = 0; i < size; ++i)
        {
            if (i < attackVal * size) {
                currentVal += stepAttack;
                input_env[counter_env + i] = std::min(currentVal, 1.f);
            } else {
                currentVal -= stepRelease;
                input_env[counter_env + i] = std::max(currentVal, 0.f);
            }
        }
        currentVal = 0.f;
        counter_env += size;
    }

    // === Create tensors ===
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    env_tensor_    = Ort::Value::CreateTensor<float>(memory_info, input_env.data(), input_env.size(), env_shape_.data(), env_shape_.size());
    params_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input_params.data(), input_params.size(), params_shape_.data(), params_shape_.size());
    is_train_tensor_ = Ort::Value::CreateTensor<bool>(memory_info, &input_is_train, 1, is_train_shape_.data(), is_train_shape_.size());
    output_tensor_   = Ort::Value::CreateTensor<float>(memory_info, results_.data(), results_.size(), output_shape_.data(), output_shape_.size());

    const char* input_names[]  = {"cond_placeholder:0", "input_placeholder:0", "is_train:0"};
    const char* output_names[] = {"Squeeze:0"};

    input_tensors.push_back(std::move(params_tensor_));
    input_tensors.push_back(std::move(env_tensor_));
    input_tensors.push_back(std::move(is_train_tensor_));

    session_.Run(Ort::RunOptions{nullptr}, input_names, input_tensors.data(), 3, output_names, &output_tensor_, 1);

    // === Build Audio Buffer from results ===
    juce::AudioBuffer<float> buffer;
    buffer.setSize(1, size); // mono buffer
    for (int i = 0; i < size; ++i)
        buffer.setSample(0, i, results_[i]);

    const double fs = 16000.0;
    juce::BigInteger range;
    range.setRange(0, 128, true);

    mProcessor.mSampler.addSound(new AudioBufferSamplerSound("Sample", buffer, fs, range, 60, 0.1, 0.1, 10.0));

    // === Push buffer into audio queue for GUI & playback ===
    if (mProcessor.mAudioBufferQueue)
        mProcessor.mAudioBufferQueue->push(buffer);

    // === Send to GUI visualizer ===
    MessageManager::callAsync([buf = buffer, &processor = mProcessor]() mutable {
        if (auto* editor = dynamic_cast<NewPluginTemplateAudioProcessorEditor*>(processor.getActiveEditor()))
        {
            editor->updateVisualizer(buf);
        }
    });


    DBG("inference complete");
    return JobStatus::jobHasFinished;
}
