// PluginEditor.h
#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "SpectrogramVisualizer.h"

class DualThumbSlider : public juce::Component
{
public:
    DualThumbSlider();
    ~DualThumbSlider() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    void setLeftValue(float newValue);
    void setRightValue(float newValue);

    float getLeftValue() const { return leftValue; }
    float getRightValue() const { return rightValue; }

    std::function<void(float)> onLeftValueChanged;
    std::function<void(float)> onRightValueChanged;

private:
    float leftValue = 0.0f;
    float rightValue = 0.0f;

    enum class DragMode { None, Left, Right };
    DragMode currentDragMode = DragMode::None;

    juce::Rectangle<float> getLeftThumbBounds() const;
    juce::Rectangle<float> getRightThumbBounds() const;
    float getValueFromPosition(float x) const;
    float getPositionFromValue(float value) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualThumbSlider)
};


class NewPluginTemplateAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::DragAndDropContainer,
                                              public juce::Timer
{
public:
    NewPluginTemplateAudioProcessorEditor (NewPluginTemplateAudioProcessor&);
    ~NewPluginTemplateAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void updateVisualizer(const juce::AudioSampleBuffer& buffer);

private:
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

    NewPluginTemplateAudioProcessor& audioProcessor;

    SpectrogramVisualizer mVisualizer;  // Move this up

    // Visual components
    std::unique_ptr<DualThumbSlider> mSliders[params::totalParams];
    std::unique_ptr<juce::Label> mLabels[params::totalParams];
    std::unique_ptr<juce::Label> mFooterLabel;

    // Hidden sliders
    std::unique_ptr<juce::Slider> mLeftSliders[params::totalParams];
    std::unique_ptr<juce::Slider> mRightSliders[params::totalParams];

    // THESE MUST BE DECLARED LAST (destroyed first)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mLeftAttachments[params::totalParams];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mRightAttachments[params::totalParams];


    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginTemplateAudioProcessorEditor)
};