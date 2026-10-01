#include "PreviewEngine.h"

#include <algorithm>

namespace samplebox
{
namespace
{
// 5 milliseconds fade-out window for pop-free choking and stopping
constexpr double kFadeTimeSeconds = 0.005;
}

PreviewEngine::PreviewEngine()
{
    formatManager.registerBasicFormats();
}

PreviewEngine::~PreviewEngine()
{
    transportSource.setSource(nullptr);
}

bool PreviewEngine::play(const std::filesystem::path& file)
{
    // Choke existing playback
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    isFadingOut.store(false, std::memory_order_relaxed);
    currentGain = 1.0f;

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(juce::File(file.string())));
    if (reader == nullptr)
        return false;

    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
    transportSource.setSource(newSource.get(), 0, nullptr, newSource->getAudioFormatReader()->sampleRate);
    readerSource = std::move(newSource);

    transportSource.setPosition(0.0);
    transportSource.start();
    return true;
}

void PreviewEngine::stop()
{
    if (!isPlaying())
        return;

    // Trigger rapid 5ms fade-out on audio thread
    isFadingOut.store(true, std::memory_order_release);
}

bool PreviewEngine::isPlaying() const
{
    return transportSource.isPlaying();
}

void PreviewEngine::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    const auto fadeSamples = static_cast<float>(currentSampleRate * kFadeTimeSeconds);
    fadeOutStep = fadeSamples > 0.0f ? (1.0f / fadeSamples) : 1.0f;

    transportSource.prepareToPlay(samplesPerBlockExpected, currentSampleRate);
}

void PreviewEngine::releaseResources()
{
    transportSource.releaseResources();
}

void PreviewEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);

    if (bufferToFill.buffer == nullptr || bufferToFill.numSamples <= 0)
        return;

    bufferToFill.buffer->applyGain(bufferToFill.startSample, bufferToFill.numSamples,
                                   volume.load(std::memory_order_relaxed));

    if (isFadingOut.load(std::memory_order_acquire))
    {
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            currentGain = std::max(0.0f, currentGain - fadeOutStep);

            for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
            {
                auto* channelData = bufferToFill.buffer->getWritePointer(ch, bufferToFill.startSample);
                channelData[sample] *= currentGain;
            }

            if (currentGain <= 0.0f)
            {
                // Completed fade-out: silence remaining samples in block and stop transport
                for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
                {
                    bufferToFill.buffer->clear(ch, bufferToFill.startSample + sample, bufferToFill.numSamples - sample);
                }

                transportSource.stop();
                isFadingOut.store(false, std::memory_order_release);
                currentGain = 1.0f;
                break;
            }
        }
    }
}
}
