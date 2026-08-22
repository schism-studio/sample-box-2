#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Data/DirectoryScanner.h"

namespace Antigravity
{
    class PluginProcessor : public juce::AudioProcessor,
                            public DirectoryScanner::Listener
    {
    public:
        PluginProcessor();
        ~PluginProcessor() override;

        //==============================================================================
        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return "Sample Pack Visual Browser"; }
        bool acceptsMidi() const override  { return false; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }

        int  getNumPrograms() override    { return 1; }
        int  getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}

        void getStateInformation(juce::MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

        //==============================================================================
        // Custom library path & scanner API
        juce::String getSampleLibraryPath() const;
        void setSampleLibraryPath(const juce::String& path);

        DirectoryScanner& getScanner() { return scanner; }
        const std::vector<SampleFileInfo>& getIndexedFiles() const { return indexedFiles; }
        void updateIndexedFiles(const std::vector<SampleFileInfo>& newFiles);

        // DirectoryScanner::Listener — forwards scan results to indexed cache
        void scanProgressUpdated(float /*progress*/, int /*filesFound*/) override {}
        void scanFinished(const std::vector<SampleFileInfo>& results) override;

    private:
        void loadSettings();

        std::unique_ptr<juce::PropertiesFile>    settings;
        DirectoryScanner                          scanner;
        std::vector<SampleFileInfo>               indexedFiles;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
    };
}
