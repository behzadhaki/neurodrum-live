#include "PluginEditor.h"
#include "PluginProcessor.h"

NewPluginTemplateAudioProcessorEditor::NewPluginTemplateAudioProcessorEditor (NewPluginTemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Color themes per parameter
    std::array<std::tuple<juce::Colour, juce::Colour, juce::Colour>, params::totalParams> sliderColours = {{
        { juce::Colours::red,       juce::Colours::yellow,     juce::Colours::yellow },   // attack
        { juce::Colours::blue,      juce::Colours::lightblue,  juce::Colours::blue },     // release
        { juce::Colours::orange,    juce::Colours::red,        juce::Colours::orange },   // brightness
        { juce::Colours::purple,    juce::Colours::violet,     juce::Colours::purple },   // hardness
        { juce::Colours::green,     juce::Colours::teal,       juce::Colours::green },    // depth
        { juce::Colours::brown,     juce::Colours::orange,     juce::Colours::brown },    // roughness
        { juce::Colours::pink,      juce::Colours::deeppink,   juce::Colours::pink },     // boominess
        { juce::Colours::lightgreen,juce::Colours::darkgreen,  juce::Colours::lightgreen }, // warmth
        { juce::Colours::cyan,      juce::Colours::blue,       juce::Colours::cyan }      // sharpness
    }};

    std::array<juce::String, params::totalParams> labels = {
        "Attack", "Release", "Brightness", "Hardness", "Depth",
        "Roughness", "Boominess", "Warmth", "Sharpness"
    };

    for (int i = 0; i < params::totalParams; ++i)
    {
        auto [start, end, dot] = sliderColours[i];

        // Slider
        mSliders[i] = std::make_unique<GradientSlider>(start, end, dot);
        addAndMakeVisible(*mSliders[i]);

        // Label
        mLabels[i] = std::make_unique<juce::Label>();
        mLabels[i]->setText(labels[i], juce::dontSendNotification);
        mLabels[i]->setJustificationType(juce::Justification::centredRight);
        mLabels[i]->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(*mLabels[i]);

        // APVTS attachment
        mSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(), audioProcessor.getParameterID(i), *mSliders[i]);
    }

    addAndMakeVisible(mVisualizer);

    setSize (600, 300);
}

void NewPluginTemplateAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // plugin background
}

void NewPluginTemplateAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // Left: sliders+labels | Right: visualizer
    auto left = area.removeFromLeft(area.getWidth() * 0.45f);
    auto right = area;

    // Visualizer bounds
    mVisualizer.setBounds(right.reduced(10));

    // Align sliders with visualizer height
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
            sliderColumn.removeFromTop(5); // spacing
    }
}


void NewPluginTemplateAudioProcessorEditor::updateVisualizer(const juce::AudioSampleBuffer& buffer)
{
    mVisualizer.setBuffer(buffer);
}
