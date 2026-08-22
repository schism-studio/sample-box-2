#include "PluginEditor.h"

namespace Antigravity
{
    PluginEditor::PluginEditor(PluginProcessor& p)
        : AudioProcessorEditor(&p),
          processor(p),
          settingsComponent(p)
    {
        setLookAndFeel(&customLookAndFeel);

        //==== Title label ====================================================
        titleLabel.setText("Sample Pack Visual Browser", juce::dontSendNotification);
        titleLabel.setFont(juce::Font(22.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centredLeft);
        titleLabel.setColour(juce::Label::textColourId, LookAndFeel::getAccentColor());
        addAndMakeVisible(titleLabel);

        //==== File count label ===============================================
        fileCountLabel.setText("No samples indexed", juce::dontSendNotification);
        fileCountLabel.setFont(juce::Font(13.0f));
        fileCountLabel.setJustificationType(juce::Justification::centredRight);
        fileCountLabel.setColour(juce::Label::textColourId, LookAndFeel::getTextDimmedColor());
        addAndMakeVisible(fileCountLabel);

        //==== Settings component =============================================
        addAndMakeVisible(settingsComponent);

        //==== Subscribe to scanner so fileCountLabel can be updated ==========
        processor.getScanner().addListener(this);

        // Reflect any already-indexed files from a previous session
        auto& files = processor.getIndexedFiles();
        if (!files.empty())
            fileCountLabel.setText(juce::String(files.size()) + " samples indexed",
                                   juce::dontSendNotification);

        //==== Window constraints =============================================
        setResizable(true, true);
        setResizeLimits(700, 450, 2000, 1400);
        setSize(1000, 650);
    }

    PluginEditor::~PluginEditor()
    {
        processor.getScanner().removeListener(this);
        setLookAndFeel(nullptr);
    }

    void PluginEditor::paint(juce::Graphics& g)
    {
        // Dark dashboard base
        g.fillAll(LookAndFeel::getDarkBackground());

        // Subtle header separator line
        g.setColour(LookAndFeel::getComponentBackground());
        g.fillRect(0, 54, getWidth(), 1);

        // Settings strip separator
        g.fillRect(0, getHeight() - 80, getWidth(), 1);

        // Centre content area placeholder (M3: replaced by OpenGL carousel)
        auto contentArea = getLocalBounds().reduced(0).withTrimmedTop(56).withTrimmedBottom(82);
        g.setColour(LookAndFeel::getMidBackground());
        g.fillRect(contentArea);
        g.setColour(LookAndFeel::getTextDimmedColor());
        g.setFont(juce::Font(16.0f));
        g.drawText("M3: 3D Carousel renders here", contentArea, juce::Justification::centred);
    }

    void PluginEditor::resized()
    {
        auto bounds = getLocalBounds();

        //---- Header strip (top 54px) ----------------------------------------
        auto header = bounds.removeFromTop(54).reduced(14, 0);
        titleLabel.setBounds(header.removeFromLeft(400));
        fileCountLabel.setBounds(header);

        //---- Settings strip (bottom 80px) -----------------------------------
        settingsComponent.setBounds(bounds.removeFromBottom(80).reduced(12, 6));
    }

    //===========================================================================
    void PluginEditor::scanProgressUpdated(float /*progress*/, int filesFound)
    {
        fileCountLabel.setText("Scanning... " + juce::String(filesFound) + " found",
                               juce::dontSendNotification);
    }

    void PluginEditor::scanFinished(const std::vector<SampleFileInfo>& results)
    {
        fileCountLabel.setText(juce::String(results.size()) + " samples indexed",
                               juce::dontSendNotification);
    }
}
