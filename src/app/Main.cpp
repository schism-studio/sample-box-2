#include "MainWindow.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace
{
class SampleBoxApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Sample Box"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }

    void initialise(const juce::String&) override
    {
        window = std::make_unique<samplebox::MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        window.reset();
    }

private:
    std::unique_ptr<samplebox::MainWindow> window;
};
}

START_JUCE_APPLICATION(SampleBoxApplication)
