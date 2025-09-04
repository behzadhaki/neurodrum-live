#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AudioBufferSampler.h"
#include "ModelPathConfig.h"

// Parameter ID constants
const String NewPluginTemplateAudioProcessor::ATTACK_ID = "attack";
const String NewPluginTemplateAudioProcessor::RELEASE_ID = "release";
const String NewPluginTemplateAudioProcessor::BRIGHTNESS_ID = "brightness";
const String NewPluginTemplateAudioProcessor::HARDNESS_ID = "hardness";
const String NewPluginTemplateAudioProcessor::DEPTH_ID = "depth";
const String NewPluginTemplateAudioProcessor::ROUGHNESS_ID = "roughness";
const String NewPluginTemplateAudioProcessor::BOOMINESS_ID = "boominess";
const String NewPluginTemplateAudioProcessor::WARMTH_ID = "warmth";
const String NewPluginTemplateAudioProcessor::SHARPNESS_ID = "sharpness";

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

    std::cout << "Model file path: " << mModelFile.getFullPathName() << std::endl;

    if (!mModelFile.existsAsFile())
    {
        std::cerr << "Model file does not exist at: " << mModelFile.getFullPathName() << std::endl;
    } else
    {
        std::cout << "Model file exists at: " << mModelFile.getFullPathName() << std::endl;
    }

    // 🔄 Start parameter polling
    startParamPolling();
}

NewPluginTemplateAudioProcessor::~NewPluginTemplateAudioProcessor()
{
}

AudioProcessorValueTreeState::ParameterLayout NewPluginTemplateAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back(std::make_unique<AudioParameterFloat>(ATTACK_ID, "Attack", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<AudioParameterFloat>(RELEASE_ID, "Release", 0.0f, 1.0f, 0.9f));
    params.push_back(std::make_unique<AudioParameterFloat>(BRIGHTNESS_ID, "Brightness", 0.0f, 1.0f, 0.46533436f));
    params.push_back(std::make_unique<AudioParameterFloat>(HARDNESS_ID, "Hardness", 0.0f, 1.0f, 0.6132435f));
    params.push_back(std::make_unique<AudioParameterFloat>(DEPTH_ID, "Depth", 0.0f, 1.0f, 0.6906892f));
    params.push_back(std::make_unique<AudioParameterFloat>(ROUGHNESS_ID, "Roughness", 0.0f, 1.0f, 0.5227648f));
    params.push_back(std::make_unique<AudioParameterFloat>(BOOMINESS_ID, "Boominess", 0.0f, 1.0f, 0.6955591f));
    params.push_back(std::make_unique<AudioParameterFloat>(WARMTH_ID, "Warmth", 0.0f, 1.0f, 0.733622f));
    params.push_back(std::make_unique<AudioParameterFloat>(SHARPNESS_ID, "Sharpness", 0.0f, 1.0f, 0.4321724f));

    return { params.begin(), params.end() };
}

String NewPluginTemplateAudioProcessor::getParameterID(int paramIndex)
{
    switch (paramIndex)
    {
        case 0: return ATTACK_ID;
        case 1: return RELEASE_ID;
        case 2: return BRIGHTNESS_ID;
        case 3: return HARDNESS_ID;
        case 4: return DEPTH_ID;
        case 5: return ROUGHNESS_ID;
        case 6: return BOOMINESS_ID;
        case 7: return WARMTH_ID;
        case 8: return SHARPNESS_ID;
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
}

void NewPluginTemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewPluginTemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
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
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewPluginTemplateAudioProcessor::createEditor()
{
    return new NewPluginTemplateAudioProcessorEditor (*this);
}

//==============================================================================
void NewPluginTemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the parameter state
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NewPluginTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the parameter state
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
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
    mLastParams.resize(9, -1.0f); // init with invalid values
    startTimerHz(50); // 50 Hz = every 20 ms
}

void NewPluginTemplateAudioProcessor::timerCallback()
{
    std::vector<float> current(9);
    for (int i = 0; i < 9; ++i)
        current[i] = getParameterValue(i);

    if (current != mLastParams)
    {
        mLastParams = current;
        mParamQueue.push(current);

        if (mThreadPool)
            mThreadPool->addJob(new InferenceThreadJob(*this), true);
    }
}


// void NewPluginTemplateAudioProcessor::generateSample()
// {
//     juce::ScopedLock irCalculationlock(mMutex);
//     if (mThreadPool)
//     {
//         mThreadPool->removeAllJobs(true, 1000);
//     }
//
//     mThreadPool->addJob(new InferenceThreadJob(*this), true);
// }

const File NewPluginTemplateAudioProcessor::getModelFile()
{
    return mModelFile;
}