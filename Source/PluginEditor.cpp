#include "PluginEditor.h"
#include "PluginProcessor.h"

// DualThumbSlider Implementation
// DualThumbSlider Implementation
DualThumbSlider::DualThumbSlider()
{
    setSize(200, 30);
}

void DualThumbSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float cornerRadius = bounds.getHeight() * 0.5f;

    // Background track
    g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
    g.fillRect(bounds);

    // Thumb positions
    float leftPos  = getPositionFromValue(leftValue);
    float rightPos = getPositionFromValue(rightValue);

    // Thin highlighted bar between thumbs
    float barHeight = bounds.getHeight() * 0.2f;
    auto rangeRect = juce::Rectangle<float>(std::min(leftPos, rightPos),
                                            bounds.getCentreY() - barHeight * 0.5f,
                                            std::abs(rightPos - leftPos),
                                            barHeight);

    // Gradient always: blue → red
    juce::ColourGradient grad(juce::Colours::lightblue.withAlpha(0.5f), leftPos, bounds.getCentreY(),
                              juce::Colours::lightcoral.withAlpha(0.5f), rightPos, bounds.getCentreY(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(rangeRect, barHeight * 0.5f);

    // ---- Thumbs ----
    auto leftThumb  = getLeftThumbBounds();
    auto rightThumb = getRightThumbBounds();

    // Decide orientation: inward-pointing
    bool leftIsLeftmost = (leftPos <= rightPos);

    // Left thumb (blue)
    juce::Path leftTri;
    if (leftIsLeftmost)
    {
        // point right (inwards)
        leftTri.addTriangle(leftThumb.getX(), leftThumb.getY(),
                            leftThumb.getX(), leftThumb.getBottom(),
                            leftThumb.getRight(), leftThumb.getCentreY());
    }
    else
    {
        // point left (inwards)
        leftTri.addTriangle(leftThumb.getRight(), leftThumb.getY(),
                            leftThumb.getRight(), leftThumb.getBottom(),
                            leftThumb.getX(), leftThumb.getCentreY());
    }
    g.setColour(juce::Colours::lightblue.withAlpha(0.6f));
    g.fillPath(leftTri);

    // Right thumb (red)
    juce::Path rightTri;
    if (leftIsLeftmost)
    {
        // point left (inwards)
        rightTri.addTriangle(rightThumb.getRight(), rightThumb.getY(),
                             rightThumb.getRight(), rightThumb.getBottom(),
                             rightThumb.getX(), rightThumb.getCentreY());
    }
    else
    {
        // point right (inwards)
        rightTri.addTriangle(rightThumb.getX(), rightThumb.getY(),
                             rightThumb.getX(), rightThumb.getBottom(),
                             rightThumb.getRight(), rightThumb.getCentreY());
    }
    g.setColour(juce::Colours::lightcoral.withAlpha(0.6f));
    g.fillPath(rightTri);
}

void DualThumbSlider::mouseDown(const juce::MouseEvent& e)
{
    auto leftThumb = getLeftThumbBounds();
    auto rightThumb = getRightThumbBounds();

    float distToLeft  = leftThumb.getCentre().getDistanceFrom(e.position);
    float distToRight = rightThumb.getCentre().getDistanceFrom(e.position);

    currentDragMode = (distToLeft < distToRight) ? DragMode::Left : DragMode::Right;
}

void DualThumbSlider::mouseDrag(const juce::MouseEvent& e)
{
    float newValue = getValueFromPosition(e.position.x);
    newValue = juce::jlimit(0.0f, 1.0f, newValue);

    if (e.mods.isShiftDown())
    {
        // Move both thumbs together preserving offset
        float offset = rightValue - leftValue;

        if (currentDragMode == DragMode::Left)
        {
            setLeftValue(newValue);
            setRightValue(juce::jlimit(0.0f, 1.0f, newValue + offset));
        }
        else if (currentDragMode == DragMode::Right)
        {
            setRightValue(newValue);
            setLeftValue(juce::jlimit(0.0f, 1.0f, newValue - offset));
        }

        if (onLeftValueChanged)  onLeftValueChanged(leftValue);
        if (onRightValueChanged) onRightValueChanged(rightValue);
    }
    else
    {
        if (currentDragMode == DragMode::Left)
        {
            setLeftValue(newValue);
            if (onLeftValueChanged) onLeftValueChanged(leftValue);
        }
        else if (currentDragMode == DragMode::Right)
        {
            setRightValue(newValue);
            if (onRightValueChanged) onRightValueChanged(rightValue);
        }
    }
}

void DualThumbSlider::mouseMove(const juce::MouseEvent& e)
{
    auto leftThumb = getLeftThumbBounds();
    auto rightThumb = getRightThumbBounds();

    bool overLeftThumb  = leftThumb.contains(e.position);
    bool overRightThumb = rightThumb.contains(e.position);

    if (overLeftThumb || overRightThumb)
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    else
        setMouseCursor(juce::MouseCursor::NormalCursor);
}

void DualThumbSlider::setLeftValue(float newValue)
{
    leftValue = juce::jlimit(0.0f, 1.0f, newValue);
    repaint();
}

void DualThumbSlider::setRightValue(float newValue)
{
    rightValue = juce::jlimit(0.0f, 1.0f, newValue);
    repaint();
}

juce::Rectangle<float> DualThumbSlider::getLeftThumbBounds() const
{
    auto bounds = getLocalBounds().toFloat();
    float thumbWidth  = bounds.getHeight() * 0.5f;
    float thumbHeight = bounds.getHeight() * 0.8f;

    float minX = bounds.getX();
    float maxX = bounds.getRight() - thumbWidth;

    float x = getPositionFromValue(leftValue) - thumbWidth * 0.5f;
    x = juce::jlimit(minX, maxX, x);

    float y = bounds.getCentreY() - thumbHeight * 0.5f;
    return { x, y, thumbWidth, thumbHeight };
}

juce::Rectangle<float> DualThumbSlider::getRightThumbBounds() const
{
    auto bounds = getLocalBounds().toFloat();
    float thumbWidth  = bounds.getHeight() * 0.5f;
    float thumbHeight = bounds.getHeight() * 0.8f;

    float minX = bounds.getX();
    float maxX = bounds.getRight() - thumbWidth;

    float x = getPositionFromValue(rightValue) - thumbWidth * 0.5f;
    x = juce::jlimit(minX, maxX, x);

    float y = bounds.getCentreY() - thumbHeight * 0.5f;
    return { x, y, thumbWidth, thumbHeight };
}

float DualThumbSlider::getValueFromPosition(float x) const
{
    auto bounds = getLocalBounds().toFloat();
    return (x - bounds.getX()) / bounds.getWidth();
}

float DualThumbSlider::getPositionFromValue(float value) const
{
    auto bounds = getLocalBounds().toFloat();
    return bounds.getX() + value * bounds.getWidth();
}

// Main Editor Implementation
NewPluginTemplateAudioProcessorEditor::NewPluginTemplateAudioProcessorEditor (NewPluginTemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    std::array<juce::String, params::totalParams> labels = {
        "Attack", "Release", "Brightness", "Hardness", "Depth",
        "Roughness", "Boominess", "Warmth", "Sharpness"
    };

    for (int i = 0; i < params::totalParams; ++i)
    {
        mSliders[i] = std::make_unique<DualThumbSlider>();
        addAndMakeVisible(*mSliders[i]);

        mLabels[i] = std::make_unique<juce::Label>();
        mLabels[i]->setText(labels[i], juce::dontSendNotification);
        mLabels[i]->setJustificationType(juce::Justification::centredRight);
        mLabels[i]->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(*mLabels[i]);

        // Create hidden sliders for parameter attachments (these handle host automation)
        mLeftSliders[i] = std::make_unique<juce::Slider>();
        mRightSliders[i] = std::make_unique<juce::Slider>();

        // Don't add them as visible - they're just for parameter connections
        mLeftSliders[i]->setVisible(false);
        mRightSliders[i]->setVisible(false);
        addChildComponent(*mLeftSliders[i]);  // Use addChildComponent instead of setVisible(false)
        addChildComponent(*mRightSliders[i]);

        // Create parameter attachments using the hidden sliders
        int leftParamIndex = i; // 0-8 for left channel
        int rightParamIndex = i + 9; // 9-17 for right channel

        mLeftAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(),
            audioProcessor.getParameterID(leftParamIndex),
            *mLeftSliders[i]);

        mRightAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(),
            audioProcessor.getParameterID(rightParamIndex),
            *mRightSliders[i]);

        // Set up callbacks to sync between hidden sliders and visual dual-thumb sliders
        mLeftSliders[i]->onValueChange = [this, i]() {
            mSliders[i]->setLeftValue((float)mLeftSliders[i]->getValue());
        };

        mRightSliders[i]->onValueChange = [this, i]() {
            mSliders[i]->setRightValue((float)mRightSliders[i]->getValue());
        };

        // Set up dual-thumb slider callbacks to update the hidden sliders (which update parameters)
        mSliders[i]->onLeftValueChanged = [this, i](float newValue) {
            mLeftSliders[i]->setValue(newValue, juce::dontSendNotification);
            // Manually notify the parameter since we bypassed the slider's notification
            auto* param = audioProcessor.getValueTreeState().getParameter(audioProcessor.getParameterID(i));
            if (param != nullptr)
                param->setValueNotifyingHost(newValue);
        };

        mSliders[i]->onRightValueChanged = [this, i](float newValue) {
            mRightSliders[i]->setValue(newValue, juce::dontSendNotification);
            // Manually notify the parameter
            auto* param = audioProcessor.getValueTreeState().getParameter(audioProcessor.getParameterID(i + 9));
            if (param != nullptr)
                param->setValueNotifyingHost(newValue);
        };

        // Initialize slider values from current parameter values
        float leftVal = audioProcessor.getParameterValue(leftParamIndex);
        float rightVal = audioProcessor.getParameterValue(rightParamIndex);

        mLeftSliders[i]->setValue(leftVal, juce::dontSendNotification);
        mRightSliders[i]->setValue(rightVal, juce::dontSendNotification);
        mSliders[i]->setLeftValue(leftVal);
        mSliders[i]->setRightValue(rightVal);
    }

    addAndMakeVisible(mVisualizer);

    // On load, check if processor has queued audio
    if (audioProcessor.mAudioBufferQueue)
    {
        auto latest = audioProcessor.mAudioBufferQueue->getLatestDataWithoutMovingFIFOHeads();
        if (latest.getNumSamples() > 0)
            mVisualizer.setBuffer(latest);
    }

    setSize (700, 400); // Slightly larger for stereo view

    startTimerHz(30); // 30 Hz timer for polling audio buffer
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

void NewPluginTemplateAudioProcessorEditor::timerCallback()
{
    // Check if there's new audio data
    if (audioProcessor.mAudioBufferQueue && audioProcessor.mAudioBufferQueue->getNumReady() > 0)
    {
        auto latestBuffer = audioProcessor.mAudioBufferQueue->getLatestOnly();
        if (latestBuffer.getNumSamples() > 0)
        {
            mVisualizer.setBuffer(latestBuffer);
        }
    }

    // playhead updates
    if (audioProcessor.mPlayheadQueue && audioProcessor.mPlayheadQueue->getNumReady() > 0)
    {
        std::vector<std::pair<int,float>> collected;

        while (audioProcessor.mPlayheadQueue->getNumReady() > 0)
        {
            auto pair = audioProcessor.mPlayheadQueue->pop();
            if (pair.first >= 0 && pair.second >= 0.0f)
                collected.push_back(pair);
        }

        mVisualizer.setPlayheads(collected);
    }

}

NewPluginTemplateAudioProcessorEditor::~NewPluginTemplateAudioProcessorEditor()
{
    stopTimer();

    // Clear all callbacks first to break potential cycles
    for (int i = 0; i < params::totalParams; ++i)
    {
        if (mSliders[i])
        {
            mSliders[i]->onLeftValueChanged = nullptr;
            mSliders[i]->onRightValueChanged = nullptr;
        }

        if (mLeftSliders[i])
            mLeftSliders[i]->onValueChange = nullptr;
        if (mRightSliders[i])
            mRightSliders[i]->onValueChange = nullptr;
    }

    // Reset attachments before sliders
    for (int i = 0; i < params::totalParams; ++i)
    {
        mLeftAttachments[i].reset();
        mRightAttachments[i].reset();
    }
}