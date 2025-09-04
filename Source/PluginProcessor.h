#pragma once

#include "Parameters.h"
#include "onnxruntime_c_api.h"
#include "InferenceThreadJob.h"

using namespace juce;

class NewPluginTemplateAudioProcessor : public PluginHelpers::ProcessorBase
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

    void generateSample();
    void play();
    const File getModelFile();

    // Parameter access methods
    AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    String getParameterID(int paramIndex);
    float getParameterValue(int paramIndex);

    juce::Synthesiser mSampler;

private:
    // Parameter tree state for automation
    AudioProcessorValueTreeState parameters;

    // Parameter IDs
    static const String ATTACK_ID;
    static const String RELEASE_ID;
    static const String BRIGHTNESS_ID;
    static const String HARDNESS_ID;
    static const String DEPTH_ID;
    static const String ROUGHNESS_ID;
    static const String BOOMINESS_ID;
    static const String WARMTH_ID;
    static const String SHARPNESS_ID;

    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::unique_ptr<ThreadPool> mThreadPool;
    mutable CriticalSection mMutex;

    File mModelFile;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginTemplateAudioProcessor)
};