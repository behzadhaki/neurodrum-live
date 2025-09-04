#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

class AudioVisualizer : public juce::Component
{
public:
    AudioVisualizer() {}

    void setBuffer(const juce::AudioSampleBuffer& newBuffer)
    {
        buffer.makeCopyOf(newBuffer);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        if (buffer.getNumSamples() == 0)
            return;

        g.setColour(juce::Colours::limegreen);

        auto bounds = getLocalBounds().toFloat();
        int numSamples = buffer.getNumSamples();
        int channel = 0; // just draw first channel

        juce::Path path;
        path.startNewSubPath(bounds.getX(), bounds.getCentreY());

        for (int i = 0; i < numSamples; ++i)
        {
            float x = bounds.getX() + (i / (float) numSamples) * bounds.getWidth();
            float y = juce::jmap(buffer.getSample(channel, i), -1.0f, 1.0f,
                                 bounds.getBottom(), bounds.getY());
            path.lineTo(x, y);
        }

        g.strokePath(path, juce::PathStrokeType(1.5f));
    }

private:
    juce::AudioSampleBuffer buffer;
};
