#pragma once

#include "Parameters.h"
#include "onnxruntime_c_api.h"
#include "InferenceThreadJob.h"
#include "LockFreeQueue.h"

using namespace juce;

class NewPluginTemplateAudioProcessor : public PluginHelpers::ProcessorBase,
                                        private juce::Timer
{
public:
    NewPluginTemplateAudioProcessor();
    ~NewPluginTemplateAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void play();
    const File getModelFile();

    // Parameter access methods
    AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    String getParameterID(int paramIndex);
    float getParameterValue(int paramIndex);

    juce::Synthesiser mSampler;

    // Param polling
    void timerCallback() override;
    void startParamPolling();

    // Lockfree queue with latest params (now stereo - 18 params total)
    std::unique_ptr<DynamicLockFreeQueue<std::vector<float>, 32>> mParamQueue;
    std::unique_ptr<DynamicLockFreeQueue<juce::AudioBuffer<float>, 8>> mAudioBufferQueue;
    std::unique_ptr<DynamicLockFreeQueue<std::pair<int,float>, 128>> mPlayheadQueue;

    std::atomic<bool> mIsBeingDestroyed{false};

private:
    // Parameter tree state for automation
    AudioProcessorValueTreeState parameters;

    // Parameter IDs - Left Channel
    static const String ATTACK_L_ID;
    static const String RELEASE_L_ID;
    static const String BRIGHTNESS_L_ID;
    static const String HARDNESS_L_ID;
    static const String DEPTH_L_ID;
    static const String ROUGHNESS_L_ID;
    static const String BOOMINESS_L_ID;
    static const String WARMTH_L_ID;
    static const String SHARPNESS_L_ID;

    // Parameter IDs - Right Channel
    static const String ATTACK_R_ID;
    static const String RELEASE_R_ID;
    static const String BRIGHTNESS_R_ID;
    static const String HARDNESS_R_ID;
    static const String DEPTH_R_ID;
    static const String ROUGHNESS_R_ID;
    static const String BOOMINESS_R_ID;
    static const String WARMTH_R_ID;
    static const String SHARPNESS_R_ID;

    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::unique_ptr<ThreadPool> mThreadPool;
    mutable CriticalSection mMutex;

    File mModelFile;

    // last snapshot to detect changes (now 18 params)
    std::vector<float> mLastParams;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginTemplateAudioProcessor)
};