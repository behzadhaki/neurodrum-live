#pragma once

#include "PluginProcessor.h"

class NewPluginTemplateAudioProcessorEditor  : public juce::AudioProcessorEditor, public Slider::Listener
{
public:
    NewPluginTemplateAudioProcessorEditor (NewPluginTemplateAudioProcessor&);
    ~NewPluginTemplateAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged (Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewPluginTemplateAudioProcessor& audioProcessor;

    juce::TextButton mPlayButton;
    juce::TextButton mGenerateButton;

    enum params
    {
        attack = 0,
        release,
        brightness,
        hardness,
        depth,
        roughness,
        boominess,
        warmth,
        sharpness,
        totalParams
    };

    Slider mSliders[params::totalParams];
    Label mLabels[params::totalParams];
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mSliderAttachments[params::totalParams];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginTemplateAudioProcessorEditor)
};