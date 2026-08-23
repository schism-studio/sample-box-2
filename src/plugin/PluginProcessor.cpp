#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace samplebox
{
PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    loadSettings();
}

PluginProcessor::~PluginProcessor() = default;

void PluginProcessor::prepareToPlay(double, int) {}

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

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Pass-through: no processing. Sample previewing happens through the
    // editor's own PreviewEngine, not the plugin's audio bus.
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

void PluginProcessor::loadSettings()
{
    juce::PropertiesFile::Options opts;
    opts.applicationName = "SampleBox";
    opts.filenameSuffix = "xml";
    opts.folderName = "SampleBox";
    opts.osxLibrarySubFolder = "Application Support";

    settings = std::make_unique<juce::PropertiesFile>(opts);
}

juce::String PluginProcessor::getSampleLibraryPath() const
{
    return settings != nullptr ? settings->getValue("sampleLibraryPath", "") : juce::String();
}

void PluginProcessor::setSampleLibraryPath(const juce::String& path)
{
    if (settings != nullptr)
    {
        settings->setValue("sampleLibraryPath", path);
        settings->saveIfNeeded();
    }
}
}

//==============================================================================
// Required JUCE factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new samplebox::PluginProcessor();
}
