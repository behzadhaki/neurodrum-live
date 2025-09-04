#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_formats/juce_audio_formats.h>

class SpectrogramVisualizer : public juce::Component
{
public:
    SpectrogramVisualizer();
    ~SpectrogramVisualizer() override;

    void setBuffer(const juce::AudioSampleBuffer& newBuffer);
    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;


private:
    void computeFFT();
    juce::File writeBufferToTempWav();

    juce::AudioSampleBuffer buffer;
    std::vector<float> magnitudeSpectrum;

    juce::Rectangle<float> lastWaveformArea; // keep track

    static constexpr int fftOrder = 9;
    static constexpr int fftSize = 1 << fftOrder;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramVisualizer)
};
