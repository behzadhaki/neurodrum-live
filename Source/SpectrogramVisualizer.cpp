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

    // Separate spectrum data for each channel
    std::vector<float> leftMagnitudeSpectrum;
    std::vector<float> rightMagnitudeSpectrum;

    juce::Rectangle<float> lastWaveformArea;

    static constexpr int fftOrder = 9;
    static constexpr int fftSize = 1 << fftOrder;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramVisualizer)
};

// SpectrogramVisualizer.cpp - Implementation
SpectrogramVisualizer::SpectrogramVisualizer() {}

SpectrogramVisualizer::~SpectrogramVisualizer()
{
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void SpectrogramVisualizer::setBuffer(const juce::AudioSampleBuffer& newBuffer)
{
    buffer.makeCopyOf(newBuffer);
    computeFFT();
    repaint();
}



void SpectrogramVisualizer::computeFFT()
{
    if (buffer.getNumSamples() < fftSize)
        return;

    int numChannels = std::min(buffer.getNumChannels(), 2); // Support up to stereo

    if (numChannels >= 1)
    {
        // Compute FFT for left channel (or mono)
        std::vector<float> fftDataLeft(fftSize * 2, 0.0f);

        for (int i = 0; i < fftSize; ++i)
            fftDataLeft[i] = buffer.getSample(0, i);

        window.multiplyWithWindowingTable(fftDataLeft.data(), fftSize);
        fft.performRealOnlyForwardTransform(fftDataLeft.data());

        leftMagnitudeSpectrum.resize(fftSize / 2);
        for (int bin = 0; bin < fftSize / 2; ++bin)
        {
            float re = fftDataLeft[2 * bin];
            float im = fftDataLeft[2 * bin + 1];
            float mag = std::sqrt(re * re + im * im);

            float db = juce::Decibels::gainToDecibels(mag + 1e-6f, -60.0f);
            float norm = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);

            leftMagnitudeSpectrum[bin] = juce::jlimit(0.0f, 1.0f, norm);
        }
    }

    if (numChannels >= 2)
    {
        // Compute FFT for right channel
        std::vector<float> fftDataRight(fftSize * 2, 0.0f);

        for (int i = 0; i < fftSize; ++i)
            fftDataRight[i] = buffer.getSample(1, i);

        window.multiplyWithWindowingTable(fftDataRight.data(), fftSize);
        fft.performRealOnlyForwardTransform(fftDataRight.data());

        rightMagnitudeSpectrum.resize(fftSize / 2);
        for (int bin = 0; bin < fftSize / 2; ++bin)
        {
            float re = fftDataRight[2 * bin];
            float im = fftDataRight[2 * bin + 1];
            float mag = std::sqrt(re * re + im * im);

            float db = juce::Decibels::gainToDecibels(mag + 1e-6f, -60.0f);
            float norm = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);

            rightMagnitudeSpectrum[bin] = juce::jlimit(0.0f, 1.0f, norm);
        }
    }
    else
    {
        // Mono - copy left to right
        rightMagnitudeSpectrum = leftMagnitudeSpectrum;
    }
}

void SpectrogramVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colours::black.brighter(0.1f));
    g.fillRoundedRectangle(bounds, 8.0f);

    if (buffer.getNumSamples() == 0)
        return;

    // Layout
    auto waveformArea = bounds.removeFromTop(bounds.getHeight() * 0.33f);
    lastWaveformArea = waveformArea;
    auto spectrumArea = bounds;

    int numChannels = std::min(buffer.getNumChannels(), 2);

    // --- Draw stereo waveforms (top 1/3) ---
    g.saveState();
    g.reduceClipRegion(waveformArea.toNearestInt());

    // Left channel waveform (blue)
    g.setColour(juce::Colours::lightblue.withAlpha(0.8f));
    juce::Path leftWaveformPath;
    leftWaveformPath.startNewSubPath(waveformArea.getX(), waveformArea.getCentreY());

    int numSamples = buffer.getNumSamples();
    for (int x = 0; x < waveformArea.getWidth(); ++x)
    {
        int sampleIndex = juce::jmap(x, 0, (int)waveformArea.getWidth() - 1, 0, numSamples - 1);
        float sample = buffer.getSample(0, sampleIndex);
        float y = juce::jmap(sample, -1.0f, 1.0f, waveformArea.getBottom(), waveformArea.getY());
        leftWaveformPath.lineTo(waveformArea.getX() + x, y);
    }
    g.strokePath(leftWaveformPath, juce::PathStrokeType(2.0f));

    // Right channel waveform (red) - if stereo
    if (numChannels >= 2)
    {
        g.setColour(juce::Colours::lightcoral.withAlpha(0.8f));
        juce::Path rightWaveformPath;
        rightWaveformPath.startNewSubPath(waveformArea.getX(), waveformArea.getCentreY());

        for (int x = 0; x < waveformArea.getWidth(); ++x)
        {
            int sampleIndex = juce::jmap(x, 0, (int)waveformArea.getWidth() - 1, 0, numSamples - 1);
            float sample = buffer.getSample(1, sampleIndex);
            float y = juce::jmap(sample, -1.0f, 1.0f, waveformArea.getBottom(), waveformArea.getY());
            rightWaveformPath.lineTo(waveformArea.getX() + x, y);
        }
        g.strokePath(rightWaveformPath, juce::PathStrokeType(2.0f));
    }

    g.restoreState();

    // --- Draw stereo spectrums (bottom 2/3) ---
    if (!leftMagnitudeSpectrum.empty())
    {
        g.saveState();
        g.reduceClipRegion(spectrumArea.toNearestInt());

        // Left channel spectrum (blue)
        juce::Path leftSpectrumPath;
        leftSpectrumPath.startNewSubPath(spectrumArea.getX(), spectrumArea.getBottom());

        for (size_t i = 0; i < leftMagnitudeSpectrum.size(); ++i)
        {
            float x = juce::jmap((float)i, 0.0f, (float)(leftMagnitudeSpectrum.size() - 1),
                                 spectrumArea.getX(), spectrumArea.getRight());
            float y = juce::jmap(leftMagnitudeSpectrum[i], 0.0f, 1.0f,
                                 spectrumArea.getBottom(), spectrumArea.getY());
            leftSpectrumPath.lineTo(x, y);
        }

        leftSpectrumPath.lineTo(spectrumArea.getRight(), spectrumArea.getBottom());
        leftSpectrumPath.closeSubPath();

        g.setColour(juce::Colours::lightblue.withAlpha(0.6f));
        g.fillPath(leftSpectrumPath);

        // Right channel spectrum (red) - if stereo
        if (numChannels >= 2 && !rightMagnitudeSpectrum.empty())
        {
            juce::Path rightSpectrumPath;
            rightSpectrumPath.startNewSubPath(spectrumArea.getX(), spectrumArea.getBottom());

            for (size_t i = 0; i < rightMagnitudeSpectrum.size(); ++i)
            {
                float x = juce::jmap((float)i, 0.0f, (float)(rightMagnitudeSpectrum.size() - 1),
                                     spectrumArea.getX(), spectrumArea.getRight());
                float y = juce::jmap(rightMagnitudeSpectrum[i], 0.0f, 1.0f,
                                     spectrumArea.getBottom(), spectrumArea.getY());
                rightSpectrumPath.lineTo(x, y);
            }

            rightSpectrumPath.lineTo(spectrumArea.getRight(), spectrumArea.getBottom());
            rightSpectrumPath.closeSubPath();

            g.setColour(juce::Colours::lightcoral.withAlpha(0.5f));
            g.fillPath(rightSpectrumPath);
        }

        g.restoreState();
    }
}

void SpectrogramVisualizer::mouseDown(const juce::MouseEvent& e)
{
    if (lastWaveformArea.contains(e.position) && buffer.getNumSamples() > 0)
    {
        juce::File temp = writeBufferToTempWav();

        if (auto* dnd = findParentComponentOfClass<juce::DragAndDropContainer>())
        {
            juce::StringArray files;
            files.add(temp.getFullPathName());
            dnd->performExternalDragDropOfFiles(files, false);
        }
    }
}

void SpectrogramVisualizer::mouseMove(const juce::MouseEvent& e)
{
    if (lastWaveformArea.contains(e.position) && buffer.getNumSamples() > 0)
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    else
        setMouseCursor(juce::MouseCursor::NormalCursor);
}

void SpectrogramVisualizer::mouseExit(const juce::MouseEvent& e)
{
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void SpectrogramVisualizer::mouseDrag(const juce::MouseEvent& e)
{
    // optionally: nothing needed, startDragging fires on mouseDown
}

juce::File SpectrogramVisualizer::writeBufferToTempWav()
{
    auto tempFile = juce::File::createTempFile("generated_audio.wav");
    tempFile.deleteFile();

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::FileOutputStream> stream(tempFile.createOutputStream());

    if (stream)
    {
        std::unique_ptr<juce::AudioFormatWriter> writer(
            wavFormat.createWriterFor(stream.get(), 16000.0,
                                      (unsigned int) buffer.getNumChannels(),
                                      16, {}, 0));
        if (writer)
        {
            stream.release();
            writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        }
    }
    return tempFile;
}