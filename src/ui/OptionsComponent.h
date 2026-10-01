#pragma once
#include "../core/AppOptions.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace samplebox
{
// Options tab content. Knows nothing about hosts: the owner supplies onChanged.
class OptionsComponent final : public juce::Component
{
public:
    OptionsComponent();

    // Updates the controls without firing onChanged.
    void setOptions(const AppOptions& newOptions);

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void(const AppOptions&)> onChanged;

private:
    void notify() { if (onChanged) onChanged(options); }

    AppOptions options;
    juce::Label viewLabel, volumeLabel;
    juce::ComboBox viewCombo;
    juce::ToggleButton titlesToggle { "Show pack titles on covers" };
    juce::Slider volumeSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsComponent)
};
}
