#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <vector>

namespace Antigravity
{
    struct SampleFileInfo
    {
        juce::String filePath;
        juce::String fileName;
        juce::String packName;
        double durationSeconds = 0.0;
        int sampleRate = 0;
        int numChannels = 0;
    };

    class DirectoryScanner : public juce::Thread
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;
            virtual void scanProgressUpdated(float progress, int filesFound) = 0;
            virtual void scanFinished(const std::vector<SampleFileInfo>& results) = 0;
        };

        DirectoryScanner();
        ~DirectoryScanner() override;

        void startScan(const juce::File& rootDirectory);
        void stopScan();

        bool isCurrentlyScanning() const { return isThreadRunning(); }
        
        void addListener(Listener* l) { listeners.add(l); }
        void removeListener(Listener* l) { listeners.remove(l); }

        void run() override;

    private:
        juce::File rootDir;
        juce::AudioFormatManager formatManager;
        std::vector<SampleFileInfo> scanResults;
        
        juce::ListenerList<Listener> listeners;

        void scanDir(const juce::File& dir, int& filesChecked, int& totalFiles);
        int countAudioFiles(const juce::File& dir);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryScanner)
    };
}
