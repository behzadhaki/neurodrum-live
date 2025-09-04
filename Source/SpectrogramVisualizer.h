#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>

class SpectrogramVisualizer : public juce::Component
{
public:
    SpectrogramVisualizer();
    ~SpectrogramVisualizer() override = default;

    void setBuffer(const juce::AudioSampleBuffer& newBuffer);
    void paint(juce::Graphics& g) override;

private:
    void computeFFT();

    juce::AudioSampleBuffer buffer;
    std::vector<float> magnitudeSpectrum;

    static constexpr int fftOrder = 9;          // 512-point FFT
    static constexpr int fftSize = 1 << fftOrder;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramVisualizer)
};
