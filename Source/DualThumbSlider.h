// DualThumbSlider.h - Inheriting from juce::Slider for APVTS compatibility

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class DualThumbSlider : public juce::Slider
{
public:
    DualThumbSlider() : juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox) {}
    // In DualThumbSlider.h, add destructor:
    ~DualThumbSlider() override
    {
        // onLeftValueChanged = nullptr;
        onRightValueChanged = nullptr;
    }

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    // Set/get the secondary (right channel) value
    void setRightValue(float newValue, juce::NotificationType notification = juce::sendNotificationAsync);
    float getRightValue() const { return rightValue; }

    // Callback for right value changes
    std::function<void(float)> onRightValueChanged;

private:
    float rightValue = 0.0f;

    enum class DragMode { None, Left, Right };
    DragMode currentDragMode = DragMode::None;

    juce::Rectangle<float> getLeftThumbBounds() const;
    juce::Rectangle<float> getRightThumbBounds() const;
    float getValueFromPosition(float x) const;
    float getPositionFromValue(float value) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualThumbSlider)
};

// DualThumbSlider.cpp - Implementation
void DualThumbSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float cornerRadius = bounds.getHeight() * 0.5f;

    // Background bar
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // Left channel active fill (blue) - top half
    float leftPos = getPositionFromValue((float)getValue());
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
        // Use the parent slider's setValue for left channel (this handles APVTS automatically)
        setValue(newValue, juce::sendNotificationAsync);
    }
    else if (currentDragMode == DragMode::Right)
    {
        setRightValue(newValue, juce::sendNotificationAsync);
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

void DualThumbSlider::setRightValue(float newValue, juce::NotificationType notification)
{
    rightValue = juce::jlimit(0.0f, 1.0f, newValue);
    repaint();

    if (notification != juce::dontSendNotification && onRightValueChanged)
        onRightValueChanged(rightValue);
}

juce::Rectangle<float> DualThumbSlider::getLeftThumbBounds() const
{
    auto bounds = getLocalBounds().toFloat();
    float thumbSize = bounds.getHeight() * 0.4f;
    float x = getPositionFromValue((float)getValue()) - thumbSize * 0.5f;
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