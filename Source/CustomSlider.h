#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class CustomSlider : public juce::Slider
{
public:
    CustomSlider() : juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox) {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float radius = bounds.getHeight() * 0.5f;

        // track background
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(bounds, radius);

        // active fill
        auto proportion = (float) juce::jlimit(0.0, 1.0, (double) getValue());
        auto activeWidth = bounds.getWidth() * proportion;
        juce::Rectangle<float> activeRect(bounds.getX(), bounds.getY(), activeWidth, bounds.getHeight());

        g.setColour(juce::Colours::skyblue);
        g.fillRoundedRectangle(activeRect, radius);

        // thumb
        g.setColour(juce::Colours::white);
        float thumbX = bounds.getX() + activeWidth;
        g.fillEllipse(thumbX - radius, bounds.getY(), bounds.getHeight(), bounds.getHeight());
    }
};
