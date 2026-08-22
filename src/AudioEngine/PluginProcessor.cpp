#include "PluginProcessor.h"
#include "../UI/PluginEditor.h"

namespace Antigravity
{
    PluginProcessor::PluginProcessor()
        : AudioProcessor(BusesProperties()
                         .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {
        scanner.addListener(this);
        loadSettings();
    }

    PluginProcessor::~PluginProcessor()
    {
        scanner.removeListener(this);
        scanner.stopScan();
    }

    //==============================================================================
    void PluginProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/) {}

    void PluginProcessor::releaseResources() {}

    bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
         && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
    }

    void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
    {
        juce::ignoreUnused(midi);
        juce::ScopedNoDenormals noDenormals;

        // Clear any extra output channels beyond our inputs
        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        // M2: Preview voice will render into buffer here
    }

    //==============================================================================
    juce::AudioProcessorEditor* PluginProcessor::createEditor()
    {
        return new PluginEditor(*this);
    }

    //==============================================================================
    void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
    {
        juce::ignoreUnused(destData);
        // M2+: serialize parameter state here
    }

    void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        juce::ignoreUnused(data, sizeInBytes);
    }

    //==============================================================================
    // Persistent path helpers
    //==============================================================================

    void PluginProcessor::loadSettings()
    {
        juce::PropertiesFile::Options opts;
        opts.applicationName     = "SamplePackVisualBrowser";
        opts.filenameSuffix      = "xml";
        opts.osxLibrarySubFolder = "Application Support";
        opts.folderName          = "SamplePackVisualBrowser";

        settings = std::make_unique<juce::PropertiesFile>(opts);

        // If a previously saved path exists, kick off a background scan immediately
        juce::String savedPath = settings->getValue("sampleLibraryPath", "");
        if (savedPath.isNotEmpty())
        {
            juce::File root(savedPath);
            if (root.isDirectory())
                scanner.startScan(root);
        }
    }

    juce::String PluginProcessor::getSampleLibraryPath() const
    {
        if (settings != nullptr)
            return settings->getValue("sampleLibraryPath", "");
        return {};
    }

    void PluginProcessor::setSampleLibraryPath(const juce::String& path)
    {
        if (settings != nullptr)
        {
            settings->setValue("sampleLibraryPath", path);
            settings->saveIfNeeded();
        }

        juce::File root(path);
        if (root.isDirectory())
            scanner.startScan(root);
    }

    void PluginProcessor::updateIndexedFiles(const std::vector<SampleFileInfo>& newFiles)
    {
        indexedFiles = newFiles;
    }

    void PluginProcessor::scanFinished(const std::vector<SampleFileInfo>& results)
    {
        updateIndexedFiles(results);
    }
}

//==============================================================================
// Required JUCE factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Antigravity::PluginProcessor();
}
