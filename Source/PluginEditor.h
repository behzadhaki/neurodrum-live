#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "SpectrogramVisualizer.h"

// ================== Black & White Slider ==================
class BWSlider : public juce::Slider
{
public:
    BWSlider() : juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox) {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float radius = bounds.getHeight() * 0.5f;

        // background bar
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(bounds, radius);

        // active fill
        auto proportion = (float) juce::jlimit(0.0, 1.0, (double) getValue());
        auto activeWidth = bounds.getWidth() * proportion;
        juce::Rectangle<float> activeRect(bounds.getX(), bounds.getY(), activeWidth, bounds.getHeight());

        g.setColour(juce::Colours::white);
        g.fillRoundedRectangle(activeRect, radius);

        // thumb
        float thumbX = bounds.getX() + activeWidth;
        float thumbRadius = bounds.getHeight() * 0.8f;
        float center = bounds.getCentreY() - thumbRadius * 0.5f;

        g.setColour(juce::Colours::white);
        g.fillEllipse(thumbX - thumbRadius * 0.5f, center, thumbRadius, thumbRadius);

        g.setColour(juce::Colours::black);
        g.fillEllipse(thumbX - 4, bounds.getCentreY() - 4, 8, 8);
    }
};

// ================== Editor ==================
class NewPluginTemplateAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NewPluginTemplateAudioProcessorEditor (NewPluginTemplateAudioProcessor&);
    ~NewPluginTemplateAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    void updateVisualizer(const juce::AudioSampleBuffer& buffer);

private:
    NewPluginTemplateAudioProcessor& audioProcessor;

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

    std::unique_ptr<BWSlider> mSliders[params::totalParams];
    std::unique_ptr<juce::Label> mLabels[params::totalParams];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mSliderAttachments[params::totalParams];

    SpectrogramVisualizer mVisualizer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginTemplateAudioProcessorEditor)
};
