#include "PluginEditor.h"
#include "PluginProcessor.h"

// DualThumbSlider Implementation
DualThumbSlider::DualThumbSlider()
{
    setSize(200, 30);
}

void DualThumbSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float cornerRadius = bounds.getHeight() * 0.5f;

    // Background bar
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // Left channel active fill (blue)
    float leftPos = getPositionFromValue(leftValue);
    juce::Rectangle<float> leftRect(bounds.getX(), bounds.getY(), leftPos, bounds.getHeight() * 0.5f);
    juce::ColourGradient leftGrad(juce::Colours::lightblue.withAlpha(0.7f), leftRect.getX(), leftRect.getCentreY(),
                                  juce::Colours::blue.withAlpha(0.7f), leftRect.getRight(), leftRect.getCentreY(), false);
    g.setGradientFill(leftGrad);
    g.fillRoundedRectangle(leftRect, cornerRadius);

    // Right channel active fill (red) - bottom half
    float rightPos = getPositionFromValue(rightValue);
    juce::Rectangle<float> rightRect(bounds.getX(), bounds.getCentreY(), rightPos, bounds.getHeight() * 0.5f);
    juce::ColourGradient rightGrad(juce::Colours::lightcoral.withAlpha(0.7f), rightRect.getX(), rightRect.getCentreY(),
                                   juce::Colours::red.withAlpha(0.7f), rightRect.getRight(), rightRect.getCentreY(), false);
    g.setGradientFill(rightGrad);
    g.fillRoundedRectangle(rightRect, cornerRadius);

    // Left thumb (blue) - top position
    auto leftThumb = getLeftThumbBounds();
    g.setColour(juce::Colours::lightblue);
    g.fillEllipse(leftThumb);
    g.setColour(juce::Colours::blue);
    g.drawEllipse(leftThumb, 2.0f);

    // Right thumb (red) - bottom position
    auto rightThumb = getRightThumbBounds();
    g.setColour(juce::Colours::lightcoral);
    g.fillEllipse(rightThumb);
    g.setColour(juce::Colours::red);
    g.drawEllipse(rightThumb, 2.0f);
}

void DualThumbSlider::mouseDown(const juce::MouseEvent& e)
{
    auto leftThumb = getLeftThumbBounds();
    auto rightThumb = getRightThumbBounds();

    // Check which thumb is closer to the click
    float distToLeft = leftThumb.getCentre().getDistanceFrom(e.position);
    float distToRight = rightThumb.getCentre().getDistanceFrom(e.position);

    if (distToLeft < distToRight)
        currentDragMode = DragMode::Left;
    else
        currentDragMode = DragMode::Right;
}

void DualThumbSlider::mouseDrag(const juce::MouseEvent& e)
{
    float newValue = getValueFromPosition(e.position.x);
    newValue = juce::jlimit(0.0f, 1.0f, newValue);

    if (currentDragMode == DragMode::Left)
    {
        setLeftValue(newValue);
        if (onLeftValueChanged)
            onLeftValueChanged(leftValue);
    }
    else if (currentDragMode == DragMode::Right)
    {
        setRightValue(newValue);
        if (onRightValueChanged)
            onRightValueChanged(rightValue);
    }
}

void DualThumbSlider::mouseMove(const juce::MouseEvent& e)
{
    auto leftThumb = getLeftThumbBounds();
    auto rightThumb = getRightThumbBounds();

    bool overLeftThumb = leftThumb.contains(e.position);
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
    float thumbSize = bounds.getHeight() * 0.4f;
    float x = getPositionFromValue(leftValue) - thumbSize * 0.5f;
    float y = bounds.getY() + bounds.getHeight() * 0.1f;

    return juce::Rectangle<float>(x, y, thumbSize, thumbSize);
}

juce::Rectangle<float> DualThumbSlider::getRightThumbBounds() const
{
    auto bounds = getLocalBounds().toFloat();
    float thumbSize = bounds.getHeight() * 0.4f;
    float x = getPositionFromValue(rightValue) - thumbSize * 0.5f;
    float y = bounds.getY() + bounds.getHeight() * 0.5f;

    return juce::Rectangle<float>(x, y, thumbSize, thumbSize);
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

    // Add L/R channel indicators
    g.setColour(juce::Colours::lightblue);
    g.setFont(12.0f);
    g.drawText("L", 10, 10, 20, 20, juce::Justification::centred);

    g.setColour(juce::Colours::lightcoral);
    g.drawText("R", 10, 35, 20, 20, juce::Justification::centred);
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