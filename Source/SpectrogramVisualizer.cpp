#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "SpectrogramVisualizer.h"

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

    int numChannels = std::min(buffer.getNumChannels(), 2);

    auto computeChannel = [&](int channel, std::vector<float>& outSpectrum)
    {
        std::vector<float> fftData(fftSize * 2, 0.0f);

        for (int i = 0; i < fftSize; ++i)
            fftData[i] = buffer.getSample(channel, i);

        window.multiplyWithWindowingTable(fftData.data(), fftSize);
        fft.performRealOnlyForwardTransform(fftData.data());

        outSpectrum.resize(fftSize / 2);
        for (int bin = 0; bin < fftSize / 2; ++bin)
        {
            float re = fftData[2 * bin];
            float im = fftData[2 * bin + 1];
            float mag = std::sqrt(re * re + im * im);

            // Convert to dB
            float db = juce::Decibels::gainToDecibels(mag + 1e-6f, -60.0f);
            // Map into 0..1 (relative to -60..0dB window)
            float norm = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);

            outSpectrum[bin] = std::max(0.0f, norm);
        }

        // --- Rescale so the max = 1 ---
        float maxVal = *std::max_element(outSpectrum.begin(), outSpectrum.end());
        if (maxVal > 0.0f)
        {
            for (auto& v : outSpectrum)
                v /= maxVal;
        }
    };

    if (numChannels >= 1)
        computeChannel(0, leftMagnitudeSpectrum);

    if (numChannels >= 2)
        computeChannel(1, rightMagnitudeSpectrum);
    else
        rightMagnitudeSpectrum = leftMagnitudeSpectrum;
}

void SpectrogramVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colours::black.brighter(0.1f));
    g.fillRoundedRectangle(bounds, 8.0f);

    if (buffer.getNumSamples() == 0)
        return;

    // Layout
    auto waveformArea = bounds.removeFromTop(bounds.getHeight() * 0.66f);
    lastWaveformArea = waveformArea;
    auto spectrumArea = bounds;

    int numChannels = std::min(buffer.getNumChannels(), 2);

    // --- Draw stereo waveforms (top 2/3) ---
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

        g.setColour(juce::Colours::lightblue.withAlpha(0.5f));
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

    // --- Draw playhead ---
    if (!playheads.empty() && buffer.getNumSamples() > 0)
    {
        for (size_t idx = 0; idx < playheads.size(); ++idx)
        {
            float normalized = playheads[idx].second;
            if (normalized < 0.0f) continue; // skip sentinel

            float x = juce::jmap(normalized,
                                 0.0f, 1.0f,
                                 lastWaveformArea.getX(), lastWaveformArea.getRight());

            float shade = juce::jmap((float)idx,
                                     0.0f, (float)playheads.size() - 1,
                                     0.9f, 0.3f);
            g.setColour(juce::Colour(shade, shade, shade, 0.9f));
            g.drawLine(x, lastWaveformArea.getY(), x, lastWaveformArea.getBottom(), 2.0f);
        }
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

void SpectrogramVisualizer::setPlayheads(const std::vector<std::pair<int,float>>& newPlayheads)
{
    playheads = newPlayheads;
    repaint();
}
