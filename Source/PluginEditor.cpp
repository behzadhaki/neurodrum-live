/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewPluginTemplateAudioProcessorEditor::NewPluginTemplateAudioProcessorEditor (NewPluginTemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    mPlayButton.onClick = [&]() { audioProcessor.play(); };
    // mGenerateButton.onClick = [&]() { audioProcessor.generateSample(); };

    mPlayButton.setButtonText("Play");
    mGenerateButton.setButtonText("Generate");

    addAndMakeVisible(mPlayButton);
    addAndMakeVisible(mGenerateButton);
    addAndMakeVisible(mVisualizer);

    for (int i = 0 ; i < params::totalParams; ++i) {
        mSliders[i].setSliderStyle(Slider::SliderStyle::LinearHorizontal);
        mSliders[i].setRange(Range<double> {0.0, 1.0}, 0.001);
        addAndMakeVisible(mSliders[i]);

        // Create parameter attachments to connect sliders to parameters
        String paramID = audioProcessor.getParameterID(i);
        mSliderAttachments[i] = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(), paramID, mSliders[i]);
    }

    setSize (600, 400);
}

NewPluginTemplateAudioProcessorEditor::~NewPluginTemplateAudioProcessorEditor()
{
}

//==============================================================================
void NewPluginTemplateAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (30.0f);
    auto bounds = getLocalBounds();
    g.drawFittedText ("NEURO DRUM LIVE", bounds.removeFromTop(0.15f*getHeight()), juce::Justification::centred, 1);
}

void NewPluginTemplateAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    mGenerateButton.setBoundsRelative(0.225f, 0.8f, 0.25f, 0.125f);
    mPlayButton.setBoundsRelative(0.525f, 0.8f, 0.25f, 0.125f);

    Rectangle<float> boundsSliders { 0.075f, 0.125f, 0.75f, 0.8f };
    Rectangle<float> boundsLabels { 0.825f, 0.125f, 0.125f, 0.8f };

    mVisualizer.setBoundsRelative(0.05f, 0.05f, 0.9f, 0.2f); // top section
    // Sliders remain below

    for (int i = 0 ; i < params::totalParams; ++i) {
        Rectangle<float> sliderRect = boundsSliders.removeFromTop(0.07f);
        Rectangle<float> labelRect = boundsLabels.removeFromTop(0.07f);
        mSliders[i].setBoundsRelative(sliderRect);
    }
}

void NewPluginTemplateAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    // Parameter changes are now handled automatically by the AudioProcessorValueTreeState
    // attachments, so this method is no longer needed for parameter updates
}

void NewPluginTemplateAudioProcessorEditor::updateVisualizer(const juce::AudioSampleBuffer& buffer)
{
    mVisualizer.setBuffer(buffer);
}
