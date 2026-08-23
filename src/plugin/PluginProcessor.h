#pragma once

#include "../indexing/LibraryScanner.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

#include <memory>

namespace samplebox
{
// Deliberately a pass-through processor: Sample Box is a sample *browser*,
// not an instrument or effect. It exists as a VST3 purely so the browser
// UI (MainPanel) can live inside a DAW's plugin chain and drag samples
// straight into the arrangement. Audio in equals audio out, unmodified.
class PluginProcessor final : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Sample Box"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::String getSampleLibraryPath() const;
    void setSampleLibraryPath(const juce::String& path);

    LibraryScanner& getScanner() { return scanner; }

private:
    void loadSettings();

    std::unique_ptr<juce::PropertiesFile> settings;
    LibraryScanner scanner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
}
