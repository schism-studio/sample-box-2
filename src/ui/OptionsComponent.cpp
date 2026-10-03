#include "OptionsComponent.h"
#include "Theme.h"

namespace samplebox
{
OptionsComponent::OptionsComponent()
{
    viewLabel.setText("Default view", juce::dontSendNotification);
    volumeLabel.setText("Preview volume", juce::dontSendNotification);

    viewCombo.addItem("List (under construction)", 1);
    viewCombo.addItem("Grid (under construction)", 2);
    viewCombo.addItem("Carousel", 3);
    viewCombo.onChange = [this]
    {
        options.defaultViewMode = static_cast<PackViewMode>(viewCombo.getSelectedId() - 1);
        notify();
    };

    titlesToggle.setColour(juce::ToggleButton::textColourId, theme::textPrimary);
    titlesToggle.onClick = [this]
    {
        options.showCoverTitles = titlesToggle.getToggleState();
        notify();
    };

    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 22);
    volumeSlider.setTextBoxIsEditable(false);
    volumeSlider.textFromValueFunction = [](double v)
    {
        return juce::String(juce::roundToInt(v * 100.0)) + "%";
    };
    volumeSlider.onValueChange = [this]
    {
        options.previewVolume = static_cast<float>(volumeSlider.getValue());
        notify();
    };

    addAndMakeVisible(viewLabel);
    addAndMakeVisible(volumeLabel);
    addAndMakeVisible(viewCombo);
    addAndMakeVisible(titlesToggle);
    addAndMakeVisible(volumeSlider);

    setOptions(options);
}

void OptionsComponent::setOptions(const AppOptions& newOptions)
{
    options = newOptions;
    viewCombo.setSelectedId(static_cast<int>(options.defaultViewMode) + 1, juce::dontSendNotification);
    titlesToggle.setToggleState(options.showCoverTitles, juce::dontSendNotification);
    volumeSlider.setValue(static_cast<double>(options.previewVolume), juce::dontSendNotification);
}

void OptionsComponent::paint(juce::Graphics& g)
{
    g.fillAll(theme::background);
}

void OptionsComponent::resized()
{
    auto area = getLocalBounds().reduced(24, 16);
    constexpr int rowH = 32, labelW = 160, gap = 12, controlW = 360;

    auto row = [&](juce::Label& label, juce::Component& control)
    {
        auto r = area.removeFromTop(rowH);
        label.setBounds(r.removeFromLeft(labelW));
        control.setBounds(r.removeFromLeft(juce::jmin(r.getWidth(), controlW)));
        area.removeFromTop(gap);
    };

    row(viewLabel, viewCombo);

    auto toggleRow = area.removeFromTop(rowH);
    toggleRow.removeFromLeft(labelW);
    titlesToggle.setBounds(toggleRow.removeFromLeft(controlW));
    area.removeFromTop(gap);

    row(volumeLabel, volumeSlider);
}
}