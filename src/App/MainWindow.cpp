#include "MainWindow.h"

namespace Antigravity
{
    MainWindow::MainWindow(const juce::String& name)
        : DocumentWindow(name,
                         LookAndFeel::getDarkBackground(),
                         DocumentWindow::allButtons)
    {
        setLookAndFeel(&customLookAndFeel);

        setUsingNativeTitleBar(true);
        setResizable(true, false);
        setResizeLimits(700, 450, 2000, 1400);

        // Host MainView as the window's content component
        // MainView owns its own scanner and settings in Standalone mode
        setContentOwned(&mainView, false);
        mainView.setSize(1000, 650);

        centreWithSize(1000, 650);
        setVisible(true);
    }

    void MainWindow::closeButtonPressed()
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
}
