#include <juce_gui_basics/juce_gui_basics.h>

class SampleBoxWindow final : public juce::DocumentWindow
{
public:
    SampleBoxWindow()
        : DocumentWindow("Sample Box", juce::Colours::black, DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new juce::Component(), true);
        centreWithSize(1200, 760);
        setResizable(true, true);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class SampleBoxApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Sample Box"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }

    void initialise(const juce::String&) override
    {
        window = std::make_unique<SampleBoxWindow>();
    }

    void shutdown() override
    {
        window.reset();
    }

private:
    std::unique_ptr<SampleBoxWindow> window;
};

START_JUCE_APPLICATION(SampleBoxApplication)
