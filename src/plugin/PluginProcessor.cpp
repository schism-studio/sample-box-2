#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace samplebox
{
PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

PluginProcessor::~PluginProcessor() = default;

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    previewEngine.prepareToPlay(samplesPerBlock, sampleRate);
    previewBuffer.setSize(juce::jmax(2, getTotalNumOutputChannels()), samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    previewEngine.releaseResources();
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();

    if (output != juce::AudioChannelSet::mono()
        && output != juce::AudioChannelSet::stereo())
        return false;

    return input.isDisabled()
        || input == output;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused(midi);
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Preview is mixed in here rather than played through a separate device,
    // because the host owns the audio device in a VST3 - there is no other
    // path that reaches an output. See docs/decisions/0002-preview-audio-path.md.
    if (!isNonRealtime())
    {
        previewBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
        previewBuffer.clear();
        juce::AudioSourceChannelInfo info(&previewBuffer, 0, buffer.getNumSamples());
        previewEngine.getNextAudioBlock(info);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom(ch, 0, previewBuffer, ch, 0, buffer.getNumSamples());
    }
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

void PluginProcessor::ensureSettings()
{
    if (settings != nullptr)
        return;

    juce::PropertiesFile::Options opts;
    opts.applicationName = "SampleBox";
    opts.filenameSuffix = "xml";
    opts.folderName = "SampleBox";
    opts.osxLibrarySubFolder = "Application Support";

    settings = std::make_unique<juce::PropertiesFile>(opts);
}

juce::String PluginProcessor::getSampleLibraryPath() const
{
    const_cast<PluginProcessor*>(this)->ensureSettings();
    return settings->getValue("sampleLibraryPath", "");
}

void PluginProcessor::setSampleLibraryPath(const juce::String& path)
{
    ensureSettings();
    settings->setValue("sampleLibraryPath", path);
    settings->saveIfNeeded();
}
}

//==============================================================================
// Required JUCE factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new samplebox::PluginProcessor();
}
