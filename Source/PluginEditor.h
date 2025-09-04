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
        float cornerRadius = bounds.getHeight() * 0.5f;

        // --- background bar ---
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(bounds, cornerRadius);

        // --- active fill with gradient ---
        auto proportion = (float) juce::jlimit(0.0, 1.0, (double) getValue());
        auto activeWidth = bounds.getWidth() * proportion;
        juce::Rectangle<float> activeRect(bounds.getX(), bounds.getY(), activeWidth, bounds.getHeight());

        juce::ColourGradient grad(juce::Colours::white, activeRect.getX(), activeRect.getCentreY(),
                                  juce::Colours::black, activeRect.getRight(), activeRect.getCentreY(),
                                  false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(activeRect, cornerRadius);

        // --- thumb (oval, fills vertical space) ---
        float thumbX = activeRect.getRight();
        float thumbWidth  = bounds.getHeight() * 0.6f;     // oval width
        float thumbHeight = bounds.getHeight() * 1.2f;     // oval taller
        float thumbY = bounds.getCentreY() - thumbHeight * 0.5f;

        g.setColour(juce::Colours::lightgrey);
        g.fillEllipse(thumbX - thumbWidth * 0.5f, thumbY, thumbWidth, thumbHeight);

        // inner marker dot
        // g.setColour(juce::Colours::black);
        // g.fillEllipse(thumbX - 4, bounds.getCentreY() - 4, 8, 8);
    }
};


// ================== Editor ==================
class NewPluginTemplateAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::DragAndDropContainer
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
