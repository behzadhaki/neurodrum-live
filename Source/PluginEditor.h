#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

// ================== Gradient Slider ==================
class GradientSlider : public juce::Slider
{
public:
    GradientSlider(juce::Colour start, juce::Colour end, juce::Colour dot)
        : juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox),
          colourStart(start), colourEnd(end), dotColour(dot)
    {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float radius = bounds.getHeight() * 0.5f;

        // background bar
        g.setColour(juce::Colours::lightgrey);
        g.fillRoundedRectangle(bounds, radius);

        // active gradient
        auto proportion = (float) juce::jlimit(0.0, 1.0, (double) getValue());
        auto activeWidth = bounds.getWidth() * proportion;
        juce::Rectangle<float> activeRect(bounds.getX(), bounds.getY(), activeWidth, bounds.getHeight());

        juce::ColourGradient grad(colourStart, activeRect.getX(), activeRect.getCentreY(),
                                  colourEnd, activeRect.getRight(), activeRect.getCentreY(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(activeRect, radius);

        // thumb
        float thumbX = bounds.getX() + activeWidth;
        float thumbRadius = bounds.getHeight() * 0.8f;
        float center = bounds.getCentreY() - thumbRadius * 0.5f;

        g.setColour(juce::Colours::white);
        g.fillEllipse(thumbX - thumbRadius * 0.5f, center, thumbRadius, thumbRadius);

        g.setColour(dotColour);
        g.fillEllipse(thumbX - 4, bounds.getCentreY() - 4, 8, 8);
    }

private:
    juce::Colour colourStart, colourEnd, dotColour;
};

// ================== Gradient Visualizer ==================
class GradientVisualizer : public juce::Component
{
public:
    void setBuffer(const juce::AudioSampleBuffer& newBuffer)
    {
        buffer.makeCopyOf(newBuffer);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float corner = 12.0f;

        // rounded background with slightly brighter black
        g.setColour(juce::Colour::fromRGB(20, 20, 20)); // brighter black
        g.fillRoundedRectangle(bounds, corner);

        if (buffer.getNumSamples() == 0)
            return;

        int numSamples = buffer.getNumSamples();
        int channel = 0;

        juce::Path waveform;
        waveform.startNewSubPath(bounds.getX(), bounds.getCentreY());

        for (int i = 0; i < numSamples; ++i)
        {
            float x = bounds.getX() + (i / (float) numSamples) * bounds.getWidth();
            float y = juce::jmap(buffer.getSample(channel, i), -1.0f, 1.0f,
                                 bounds.getBottom(), bounds.getY());
            waveform.lineTo(x, y);
        }

        juce::ColourGradient grad(juce::Colours::green, bounds.getX(), bounds.getCentreY(),
                                  juce::Colours::red, bounds.getRight(), bounds.getCentreY(), false);
        grad.addColour(0.5, juce::Colours::orange);
        grad.addColour(0.8, juce::Colours::pink);

        g.setGradientFill(grad);
        g.strokePath(waveform, juce::PathStrokeType(2.0f));
    }

private:
    juce::AudioSampleBuffer buffer;
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

    // sliders
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

    std::unique_ptr<GradientSlider> mSliders[params::totalParams];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mSliderAttachments[params::totalParams];

    GradientVisualizer mVisualizer;
    std::unique_ptr<juce::Label> mLabels[params::totalParams];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginTemplateAudioProcessorEditor)
};
