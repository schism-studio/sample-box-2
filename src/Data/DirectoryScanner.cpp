#include "DirectoryScanner.h"

namespace Antigravity
{
    DirectoryScanner::DirectoryScanner()
        : juce::Thread("DirectoryScannerThread")
    {
        // Register basic audio formats
        formatManager.registerBasicFormats();
    }

    DirectoryScanner::~DirectoryScanner()
    {
        stopScan();
    }

    void DirectoryScanner::startScan(const juce::File& rootDirectory)
    {
        stopScan();
        rootDir = rootDirectory;
        scanResults.clear();
        startThread(juce::Priority::background);
    }

    void DirectoryScanner::stopScan()
    {
        signalThreadShouldExit();
        stopThread(5000);
    }

    int DirectoryScanner::countAudioFiles(const juce::File& dir)
    {
        int count = 0;
        juce::Array<juce::File> children;
        dir.findChildFiles(children, juce::File::findFiles, true);
        
        for (const auto& file : children)
        {
            if (threadShouldExit())
                return 0;

            auto ext = file.getFileExtension().toLowerCase();
            if (ext == ".wav" || ext == ".aif" || ext == ".aiff" || ext == ".mp3" || ext == ".flac")
                count++;
        }
        return count;
    }

    void DirectoryScanner::run()
    {
        if (!rootDir.isDirectory())
        {
            juce::MessageManager::callAsync([this]() {
                listeners.call(&Listener::scanFinished, scanResults);
            });
            return;
        }

        int totalFiles = countAudioFiles(rootDir);
        int filesChecked = 0;

        if (totalFiles > 0)
        {
            scanDir(rootDir, filesChecked, totalFiles);
        }

        if (!threadShouldExit())
        {
            // Post results on the Message Thread
            auto resultsCopy = scanResults;
            juce::MessageManager::callAsync([this, resultsCopy]() {
                listeners.call(&Listener::scanFinished, resultsCopy);
            });
        }
    }

    void DirectoryScanner::scanDir(const juce::File& dir, int& filesChecked, int& totalFiles)
    {
        juce::Array<juce::File> filesAndDirs;
        dir.findChildFiles(filesAndDirs, juce::File::findFilesAndDirectories, false);

        for (const auto& child : filesAndDirs)
        {
            if (threadShouldExit())
                return;

            if (child.isDirectory())
            {
                scanDir(child, filesChecked, totalFiles);
            }
            else
            {
                auto ext = child.getFileExtension().toLowerCase();
                if (ext == ".wav" || ext == ".aif" || ext == ".aiff" || ext == ".mp3" || ext == ".flac")
                {
                    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(child));
                    if (reader != nullptr)
                    {
                        SampleFileInfo info;
                        info.filePath = child.getFullPathName();
                        info.fileName = child.getFileNameWithoutExtension();
                        // Pack name is the immediate parent directory name
                        info.packName = child.getParentDirectory().getFileName();
                        info.durationSeconds = (double)reader->lengthInSamples / reader->sampleRate;
                        info.sampleRate = (int)reader->sampleRate;
                        info.numChannels = (int)reader->numChannels;

                        scanResults.push_back(info);
                    }

                    filesChecked++;
                    float progress = (float)filesChecked / (float)totalFiles;
                    int count = (int)scanResults.size();

                    juce::MessageManager::callAsync([this, progress, count]() {
                        listeners.call(&Listener::scanProgressUpdated, progress, count);
                    });
                }
            }
        }
    }
}
