#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AudioBufferSampler.h"
#include "ModelPathConfig.h"



NewPluginTemplateAudioProcessor::NewPluginTemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : ProcessorBase (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    mSampler.addVoice(new AudioBufferSamplerVoice());
    mSampler.addVoice(new AudioBufferSamplerVoice());
    
    mThreadPool = std::make_unique<ThreadPool>(1);

    std::cout << "MODEL_RELATIVE_PATH: " << MODEL_RELATIVE_PATH << std::endl;

    // check if the model file exists
    mModelFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                   .getChildFile(MODEL_RELATIVE_PATH);

    std::cout << "Model file path: " << mModelFile.getFullPathName() << std::endl;
    if (!mModelFile.existsAsFile())
    {
        std::cerr << "❌ Model file does not exist at: " << mModelFile.getFullPathName() << std::endl;
    } else
    {
        std::cout << "✅ Model file exists at: " << mModelFile.getFullPathName() << std::endl;

    }
}

NewPluginTemplateAudioProcessor::~NewPluginTemplateAudioProcessor()
{
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewPluginTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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

void NewPluginTemplateAudioProcessor::loadFile()
{
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    chooser.launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file == juce::File{})
            return;
        
        mModelFile = file;

    });
}

void NewPluginTemplateAudioProcessor::generateSample()
{
    juce::ScopedLock irCalculationlock(mMutex);
    if (mThreadPool)
    {
        mThreadPool->removeAllJobs(true, 1000);
    }

    mThreadPool->addJob(new InferenceThreadJob(*this), true);
}

const File NewPluginTemplateAudioProcessor::getModelFile()
{
    return mModelFile;
}


