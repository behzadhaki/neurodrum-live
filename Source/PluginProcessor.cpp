#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AudioBufferSampler.h"
#include "ModelPathConfig.h"

// Parameter ID constants - Left Channel
const String NewPluginTemplateAudioProcessor::ATTACK_L_ID = "attackL";
const String NewPluginTemplateAudioProcessor::RELEASE_L_ID = "releaseL";
const String NewPluginTemplateAudioProcessor::BRIGHTNESS_L_ID = "brightnessL";
const String NewPluginTemplateAudioProcessor::HARDNESS_L_ID = "hardnessL";
const String NewPluginTemplateAudioProcessor::DEPTH_L_ID = "depthL";
const String NewPluginTemplateAudioProcessor::ROUGHNESS_L_ID = "roughnessL";
const String NewPluginTemplateAudioProcessor::BOOMINESS_L_ID = "boominessL";
const String NewPluginTemplateAudioProcessor::WARMTH_L_ID = "warmthL";
const String NewPluginTemplateAudioProcessor::SHARPNESS_L_ID = "sharpnessL";

// Parameter ID constants - Right Channel
const String NewPluginTemplateAudioProcessor::ATTACK_R_ID = "attackR";
const String NewPluginTemplateAudioProcessor::RELEASE_R_ID = "releaseR";
const String NewPluginTemplateAudioProcessor::BRIGHTNESS_R_ID = "brightnessR";
const String NewPluginTemplateAudioProcessor::HARDNESS_R_ID = "hardnessR";
const String NewPluginTemplateAudioProcessor::DEPTH_R_ID = "depthR";
const String NewPluginTemplateAudioProcessor::ROUGHNESS_R_ID = "roughnessR";
const String NewPluginTemplateAudioProcessor::BOOMINESS_R_ID = "boominessR";
const String NewPluginTemplateAudioProcessor::WARMTH_R_ID = "warmthR";
const String NewPluginTemplateAudioProcessor::SHARPNESS_R_ID = "sharpnessR";

NewPluginTemplateAudioProcessor::NewPluginTemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
        : ProcessorBase (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                                 .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                                 .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
), parameters(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    mSampler.addVoice(new AudioBufferSamplerVoice());
    mSampler.addVoice(new AudioBufferSamplerVoice());

    mThreadPool = std::make_unique<ThreadPool>(1);

#ifdef _WIN32
    String vst3Dir = String(std::getenv("APPDATA")) + "\\VST3\\";
    mModelFile = File(vst3Dir + "log_kicks_full.onnx");
#else
    File pluginBundle = File::getSpecialLocation(File::currentExecutableFile);
    while (pluginBundle.exists() && !pluginBundle.getFileName().endsWith(".vst3")) {
        pluginBundle = pluginBundle.getParentDirectory();
    }
    mModelFile = pluginBundle.getChildFile("Contents").getChildFile("Resources").getChildFile("log_kicks_full.onnx");
#endif

    mParamQueue = std::make_unique<DynamicLockFreeQueue<std::vector<float>, 32>>();
    mAudioBufferQueue = std::make_unique<DynamicLockFreeQueue<juce::AudioBuffer<float>, 8>>();

    std::cout << "Model file path: " << mModelFile.getFullPathName() << std::endl;

    if (!mModelFile.existsAsFile())
    {
        std::cerr << "Model file does not exist at: " << mModelFile.getFullPathName() << std::endl;
    } else
    {
        std::cout << "Model file exists at: " << mModelFile.getFullPathName() << std::endl;
    }

    // Start parameter polling - now with 18 parameters
    startParamPolling();
}

NewPluginTemplateAudioProcessor::~NewPluginTemplateAudioProcessor()
{
    mIsBeingDestroyed = true;
    stopTimer();

    if (mThreadPool)
    {
        mThreadPool->removeAllJobs(true, 1000);
        mThreadPool.reset();
    }
}


AudioProcessorValueTreeState::ParameterLayout NewPluginTemplateAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // Left Channel Parameters
    params.push_back(std::make_unique<AudioParameterFloat>(ATTACK_L_ID, "Attack L", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<AudioParameterFloat>(RELEASE_L_ID, "Release L", 0.0f, 1.0f, 0.9f));
    params.push_back(std::make_unique<AudioParameterFloat>(BRIGHTNESS_L_ID, "Brightness L", 0.0f, 1.0f, 0.46533436f));
    params.push_back(std::make_unique<AudioParameterFloat>(HARDNESS_L_ID, "Hardness L", 0.0f, 1.0f, 0.6132435f));
    params.push_back(std::make_unique<AudioParameterFloat>(DEPTH_L_ID, "Depth L", 0.0f, 1.0f, 0.6906892f));
    params.push_back(std::make_unique<AudioParameterFloat>(ROUGHNESS_L_ID, "Roughness L", 0.0f, 1.0f, 0.5227648f));
    params.push_back(std::make_unique<AudioParameterFloat>(BOOMINESS_L_ID, "Boominess L", 0.0f, 1.0f, 0.6955591f));
    params.push_back(std::make_unique<AudioParameterFloat>(WARMTH_L_ID, "Warmth L", 0.0f, 1.0f, 0.733622f));
    params.push_back(std::make_unique<AudioParameterFloat>(SHARPNESS_L_ID, "Sharpness L", 0.0f, 1.0f, 0.4321724f));

    // Right Channel Parameters
    params.push_back(std::make_unique<AudioParameterFloat>(ATTACK_R_ID, "Attack R", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<AudioParameterFloat>(RELEASE_R_ID, "Release R", 0.0f, 1.0f, 0.9f));
    params.push_back(std::make_unique<AudioParameterFloat>(BRIGHTNESS_R_ID, "Brightness R", 0.0f, 1.0f, 0.46533436f));
    params.push_back(std::make_unique<AudioParameterFloat>(HARDNESS_R_ID, "Hardness R", 0.0f, 1.0f, 0.6132435f));
    params.push_back(std::make_unique<AudioParameterFloat>(DEPTH_R_ID, "Depth R", 0.0f, 1.0f, 0.6906892f));
    params.push_back(std::make_unique<AudioParameterFloat>(ROUGHNESS_R_ID, "Roughness R", 0.0f, 1.0f, 0.5227648f));
    params.push_back(std::make_unique<AudioParameterFloat>(BOOMINESS_R_ID, "Boominess R", 0.0f, 1.0f, 0.6955591f));
    params.push_back(std::make_unique<AudioParameterFloat>(WARMTH_R_ID, "Warmth R", 0.0f, 1.0f, 0.733622f));
    params.push_back(std::make_unique<AudioParameterFloat>(SHARPNESS_R_ID, "Sharpness R", 0.0f, 1.0f, 0.4321724f));

    return { params.begin(), params.end() };
}

String NewPluginTemplateAudioProcessor::getParameterID(int paramIndex)
{
    switch (paramIndex)
    {
        // Left Channel
        case 0: return ATTACK_L_ID;
        case 1: return RELEASE_L_ID;
        case 2: return BRIGHTNESS_L_ID;
        case 3: return HARDNESS_L_ID;
        case 4: return DEPTH_L_ID;
        case 5: return ROUGHNESS_L_ID;
        case 6: return BOOMINESS_L_ID;
        case 7: return WARMTH_L_ID;
        case 8: return SHARPNESS_L_ID;

        // Right Channel
        case 9: return ATTACK_R_ID;
        case 10: return RELEASE_R_ID;
        case 11: return BRIGHTNESS_R_ID;
        case 12: return HARDNESS_R_ID;
        case 13: return DEPTH_R_ID;
        case 14: return ROUGHNESS_R_ID;
        case 15: return BOOMINESS_R_ID;
        case 16: return WARMTH_R_ID;
        case 17: return SHARPNESS_R_ID;
        default: return "";
    }
}

float NewPluginTemplateAudioProcessor::getParameterValue(int paramIndex)
{
    String paramID = getParameterID(paramIndex);
    if (paramID.isNotEmpty())
    {
        return parameters.getRawParameterValue(paramID)->load();
    }
    return 0.0f;
}

//==============================================================================
const juce::String NewPluginTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewPluginTemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewPluginTemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewPluginTemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewPluginTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewPluginTemplateAudioProcessor::getNumPrograms()
{
    return 1;
}

int NewPluginTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewPluginTemplateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewPluginTemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewPluginTemplateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewPluginTemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
}

void NewPluginTemplateAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewPluginTemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NewPluginTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool NewPluginTemplateAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NewPluginTemplateAudioProcessor::createEditor()
{
    return new NewPluginTemplateAudioProcessorEditor (*this);
}

//==============================================================================
void NewPluginTemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NewPluginTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewPluginTemplateAudioProcessor();
}

void NewPluginTemplateAudioProcessor::play()
{
    mSampler.noteOn(1, 60, 1.f);
}

void NewPluginTemplateAudioProcessor::startParamPolling()
{
    mLastParams.resize(18, -1.0f); // init with invalid values for 18 parameters
    startTimerHz(50); // 50 Hz = every 20 ms
}

void NewPluginTemplateAudioProcessor::timerCallback()
{
    std::vector<float> current(18);
    for (int i = 0; i < 18; ++i)
        current[i] = getParameterValue(i);

    if (current != mLastParams)
    {
        mLastParams = current;
        if (mParamQueue)
            mParamQueue->push(current);

        if (mThreadPool)
            mThreadPool->addJob(new InferenceThreadJob(*this), true);
    }
}

const File NewPluginTemplateAudioProcessor::getModelFile()
{
    return mModelFile;
}