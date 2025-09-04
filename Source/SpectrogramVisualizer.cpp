#include "SpectrogramVisualizer.h"

SpectrogramVisualizer::SpectrogramVisualizer() {}

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

    std::vector<float> fftData(fftSize * 2, 0.0f);
    auto numChannels = buffer.getNumChannels();

    // take first fftSize samples (mix down to mono)
    for (int i = 0; i < fftSize; ++i)
    {
        float s = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            s += buffer.getSample(ch, i);
        fftData[i] = s / (float) numChannels;
    }

    window.multiplyWithWindowingTable(fftData.data(), fftSize);
    fft.performRealOnlyForwardTransform(fftData.data());

    magnitudeSpectrum.resize(fftSize / 2);
    for (int bin = 0; bin < fftSize / 2; ++bin)
    {
        float re = fftData[2 * bin];
        float im = fftData[2 * bin + 1];
        float mag = std::sqrt(re * re + im * im);

        float db = juce::Decibels::gainToDecibels(mag + 1e-6f, -60.0f);
        float norm = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);

        magnitudeSpectrum[bin] = juce::jlimit(0.0f, 1.0f, norm);
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
    lastWaveformArea = waveformArea; // store for hit detection
    auto spectrumArea = bounds;

    // --- Draw waveform (top 1/3) ---
    g.saveState();
    g.reduceClipRegion(waveformArea.toNearestInt());

    g.setColour(juce::Colours::white);
    juce::Path waveformPath;
    waveformPath.startNewSubPath(waveformArea.getX(), waveformArea.getCentreY());

    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    for (int x = 0; x < waveformArea.getWidth(); ++x)
    {
        int sampleIndex = juce::jmap(x, 0, (int)waveformArea.getWidth() - 1, 0, numSamples - 1);

        float s = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            s += buffer.getSample(ch, sampleIndex);
        s /= (float) numChannels;

        float y = juce::jmap(s, -1.0f, 1.0f, waveformArea.getBottom(), waveformArea.getY());
        waveformPath.lineTo(waveformArea.getX() + x, y);
    }

    g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
    g.restoreState();

    // --- Draw spectrum (bottom 2/3) ---
    if (!magnitudeSpectrum.empty())
    {
        g.saveState();
        g.reduceClipRegion(spectrumArea.toNearestInt());

        juce::Path spectrumPath;
        spectrumPath.startNewSubPath(spectrumArea.getX(), spectrumArea.getBottom());

        for (size_t i = 0; i < magnitudeSpectrum.size(); ++i)
        {
            float x = juce::jmap((float)i, 0.0f, (float)(magnitudeSpectrum.size() - 1),
                                 spectrumArea.getX(), spectrumArea.getRight());
            float y = juce::jmap(magnitudeSpectrum[i], 0.0f, 1.0f,
                                 spectrumArea.getBottom(), spectrumArea.getY());
            spectrumPath.lineTo(x, y);
        }

        spectrumPath.lineTo(spectrumArea.getRight(), spectrumArea.getBottom());
        spectrumPath.closeSubPath();

        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.fillPath(spectrumPath);

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
            wavFormat.createWriterFor(stream.get(), 16000.0, // sample rate (match InferenceJob)
                                      (unsigned int) buffer.getNumChannels(),
                                      16, {}, 0));
        if (writer)
        {
            stream.release(); // writer owns it now
            writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        }
    }
    return tempFile;
}
