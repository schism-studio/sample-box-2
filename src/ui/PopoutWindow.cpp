#include "PopoutWindow.h"
#include "Theme.h"

namespace samplebox
{
PopoutWindow::PopoutWindow(const juce::String& name, juce::Component& contentToDisplay)
    : DocumentWindow(name, theme::background, DocumentWindow::closeButton | DocumentWindow::minimiseButton)
{
    setUsingNativeTitleBar(true);
    setResizable(true, false);
    setContentNonOwned(&contentToDisplay, true);
    centreWithSize(contentToDisplay.getWidth(), contentToDisplay.getHeight());
    setVisible(true);
}

void PopoutWindow::closeButtonPressed()
{
    if (onClosed)
        onClosed();
}
}
