#include "SidebarBrowser.h"
#include "Theme.h"

namespace samplebox
{
SidebarBrowser::SidebarBrowser()
{
    listBox.setRowHeight(28);
    addAndMakeVisible(listBox);
}

void SidebarBrowser::setLibrary(LibrarySnapshot snapshot)
{
    library = std::move(snapshot);
    listBox.updateContent();
    listBox.repaint();
}

void SidebarBrowser::resized()
{
    listBox.setBounds(getLocalBounds());
}

int SidebarBrowser::getNumRows()
{
    return (int) library.packs.size();
}

void SidebarBrowser::paintListBoxItem(int rowNumber, juce::Graphics& graphics, int width, int height,
                                       bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= (int) library.packs.size())
        return;

    graphics.fillAll(rowIsSelected ? theme::accent.withAlpha(0.25f) : theme::background);
    graphics.setColour(theme::textPrimary);
    graphics.setFont(juce::Font(14.0f));
    graphics.drawText(library.packs[(std::size_t) rowNumber].title,
                       juce::Rectangle<int>(0, 0, width, height).reduced(8, 0),
                       juce::Justification::centredLeft);
}

void SidebarBrowser::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (row < 0 || row >= (int) library.packs.size())
        return;

    if (onPackSelected)
        onPackSelected(library.packs[(std::size_t) row]);
}
}
