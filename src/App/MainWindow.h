#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "../UI/MainView.h"
#include "../UI/LookAndFeel.h"

namespace Antigravity
{
    // ==========================================================================
    // MainWindow
    //
    // A native OS window (DocumentWindow) that hosts MainView as its content.
    // This is the Standalone app entry point — no AudioProcessor involved.
    // ==========================================================================
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String& name);
        ~MainWindow() override = default;

        void closeButtonPressed() override;

    private:
        LookAndFeel customLookAndFeel;
        MainView    mainView;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };
}
