#include "PluginEditor.h"
#include "PluginProcessor.h"

NewPluginTemplateAudioProcessorEditor::NewPluginTemplateAudioProcessorEditor (NewPluginTemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    std::array<juce::String, params::totalParams> labels = {
        "Attack", "Release", "Brightness", "Hardness", "Depth",
        "Roughness", "Boominess", "Warmth", "Sharpness"
    };

    for (int i = 0; i < params::totalParams; ++i)
    {
        mSliders[i] = std::make_unique<BWSlider>();
        addAndMakeVisible(*mSliders[i]);

        mLabels[i] = std::make_unique<juce::Label>();
        mLabels[i]->setText(labels[i], juce::dontSendNotification);
        mLabels[i]->setJustificationType(juce::Justification::centredRight);
        mLabels[i]->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(*mLabels[i]);

        mSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(), audioProcessor.getParameterID(i), *mSliders[i]);
    }

    addAndMakeVisible(mVisualizer);


    // --- On load, check if processor has queued audio ---
    if (audioProcessor.mAudioBufferQueue) // assumes processor holds a DynamicLockFreeQueue<juce::AudioBuffer<float>, N>
    {
        auto latest = audioProcessor.mAudioBufferQueue->getLatestDataWithoutMovingFIFOHeads();
        if (latest.getNumSamples() > 0) // sanity check
            mVisualizer.setBuffer(latest);
    }


    setSize (600, 300);
}

void NewPluginTemplateAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void NewPluginTemplateAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    auto left = area.removeFromLeft(area.getWidth() * 0.4f);
    auto right = area;

    mVisualizer.setBounds(right.reduced(10));

    auto sliderColumn = left.withTop(mVisualizer.getY())
                            .withBottom(mVisualizer.getBottom());

    int totalSpacing = 5 * (params::totalParams - 1);
    int sliderHeight = (sliderColumn.getHeight() - totalSpacing) / params::totalParams;

    for (int i = 0; i < params::totalParams; ++i)
    {
        auto row = sliderColumn.removeFromTop(sliderHeight);
        auto labelArea = row.removeFromLeft(100);

        mLabels[i]->setBounds(labelArea.reduced(5));
        mSliders[i]->setBounds(row.reduced(5));

        if (i < params::totalParams - 1)
            sliderColumn.removeFromTop(5);
    }
}

void NewPluginTemplateAudioProcessorEditor::updateVisualizer(const juce::AudioSampleBuffer& buffer)
{
    mVisualizer.setBuffer(buffer);
}
