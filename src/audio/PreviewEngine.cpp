#include "PreviewEngine.h"

namespace samplebox
{
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
    // Instant choke: tear down whatever is currently playing before
    // starting the new preview, so previews never overlap.
    stop();

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
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();
}

bool PreviewEngine::isPlaying() const
{
    return transportSource.isPlaying();
}

void PreviewEngine::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PreviewEngine::releaseResources()
{
    transportSource.releaseResources();
}

void PreviewEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);
}
}
