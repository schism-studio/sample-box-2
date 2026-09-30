#include "DragDropSpike.h"
#include "Theme.h"

#include <cmath>

namespace samplebox
{
namespace
{
constexpr double kSampleRate = 44100.0;
constexpr double kToneSeconds = 0.4;
constexpr int kBitsPerSample = 16;
constexpr int kNumChannels = 1;

// Writes a minimal 16-bit mono PCM .wav by hand.
//
// Using juce::WavAudioFormat would be the obvious choice, but it lives in
// juce_audio_formats, which SampleBox_UI does not link. Adding that dependency
// for a diagnostic that is going to be deleted would mean editing the target's
// link libraries and then remembering to edit them back. A 44-byte header is
// cheaper than that.
bool writeSineWav(const juce::File& destination, double frequencyHz)
{
    const auto numFrames = (int) (kSampleRate * kToneSeconds);
    const auto dataBytes = numFrames * kNumChannels * (kBitsPerSample / 8);

    juce::MemoryOutputStream out;

    // juce::OutputStream::writeInt/writeShort are little-endian, which is what
    // RIFF wants.
    out.write("RIFF", 4);
    out.writeInt(36 + dataBytes);
    out.write("WAVE", 4);

    out.write("fmt ", 4);
    out.writeInt(16);
    out.writeShort(1); // PCM, uncompressed
    out.writeShort((short) kNumChannels);
    out.writeInt((int) kSampleRate);
    out.writeInt((int) kSampleRate * kNumChannels * (kBitsPerSample / 8)); // byte rate
    out.writeShort((short) (kNumChannels * (kBitsPerSample / 8)));         // block align
    out.writeShort((short) kBitsPerSample);

    out.write("data", 4);
    out.writeInt(dataBytes);

    // Short fades at both ends: a raw sine that starts and stops at non-zero
    // amplitude clicks, and a click would be indistinguishable from a
    // corrupted drop when listening for whether the file survived the drag.
    const auto fadeFrames = (int) (kSampleRate * 0.01);

    for (int frame = 0; frame < numFrames; ++frame)
    {
        auto gain = 0.4;

        if (frame < fadeFrames)
            gain *= (double) frame / (double) fadeFrames;
        else if (frame > numFrames - fadeFrames)
            gain *= (double) (numFrames - frame) / (double) fadeFrames;

        const auto phase = juce::MathConstants<double>::twoPi * frequencyHz
                         * ((double) frame / kSampleRate);
        const auto value = std::sin(phase) * gain;

        out.writeShort((short) juce::jlimit(-32767, 32767, (int) (value * 32767.0)));
    }

    return destination.replaceWithData(out.getData(), out.getDataSize());
}
}

DragDropSpike::DragDropSpike()
{
    spikeDirectory = juce::File::getSpecialLocation(juce::File::tempDirectory)
                         .getChildFile("SampleBoxDragSpike");

    titleLabel.setText("DRAG-OUT SPIKE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, theme::accent);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // Which host this is running in is the single most useful fact when
    // reporting a result, and the least reliable thing to remember afterwards.
    const auto hostKind = juce::JUCEApplicationBase::isStandaloneApp()
                              ? juce::String("standalone")
                              : juce::String("plug-in");

    statusLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    statusLabel.setJustificationType(juce::Justification::topLeft);
    statusLabel.setMinimumHorizontalScale(1.0f);
    addAndMakeVisible(statusLabel);

    setStatus(hostKind + ": ready", juce::Colours::grey);

    addAndMakeVisible(singleFilePad);
    addAndMakeVisible(multiFilePad);
}

DragDropSpike::~DragDropSpike() = default;

void DragDropSpike::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();

    graphics.setColour(theme::surface);
    graphics.fillRoundedRectangle(bounds, 6.0f);

    graphics.setColour(theme::accent.withAlpha(0.5f));
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
}

void DragDropSpike::resized()
{
    auto bounds = getLocalBounds().reduced(8);

    titleLabel.setBounds(bounds.removeFromTop(14));
    bounds.removeFromTop(4);

    auto padRow = bounds.removeFromTop(30);
    singleFilePad.setBounds(padRow.removeFromLeft(padRow.getWidth() / 2 - 3));
    multiFilePad.setBounds(padRow.removeFromRight(padRow.getWidth() - 3));

    bounds.removeFromTop(4);
    statusLabel.setBounds(bounds);
}

void DragDropSpike::setStatus(const juce::String& text, juce::Colour colour)
{
    statusLabel.setColour(juce::Label::textColourId, colour);
    statusLabel.setText(text, juce::dontSendNotification);
}

juce::StringArray DragDropSpike::ensureTempFiles(int count)
{
    juce::StringArray paths;

    if (!spikeDirectory.isDirectory() && !spikeDirectory.createDirectory().wasOk())
        return {};

    // Distinct pitches so a multi-file drop can be told apart by ear, and so a
    // host that silently drops only the first file is obvious.
    static constexpr double frequencies[] = { 440.0, 554.37, 659.25 };

    for (int i = 0; i < count; ++i)
    {
        const auto file = spikeDirectory.getChildFile("spike_tone_" + juce::String(i + 1) + ".wav");

        if (!file.existsAsFile()
            && !writeSineWav(file, frequencies[i % (int) juce::numElementsInArray(frequencies)]))
            return {};

        paths.add(file.getFullPathName());
    }

    return paths;
}

void DragDropSpike::beginDrag(Pad& source, int fileCount)
{
    const auto paths = ensureTempFiles(fileCount);

    if (paths.isEmpty())
    {
        setStatus("could not write temp .wav files to\n" + spikeDirectory.getFullPathName(),
                  juce::Colours::orangered);
        return;
    }

    setStatus("drag started, " + juce::String(paths.size()) + " file(s)...", juce::Colours::yellow);

    // The completion callback can arrive after this component is gone: a host
    // may close the plugin editor while an OS drag loop is still running. Same
    // hazard as defect D2, same fix.
    const auto safeThis = juce::Component::SafePointer<DragDropSpike>(this);

    // Identical to FileDragExporter::beginDrag. `canMoveFiles` is false so a
    // host can never move or delete the source, and passing a source component
    // is not cosmetic: with it, JUCE runs the Windows drag loop on a background
    // thread. Without it, DoDragDrop blocks the calling thread for the whole
    // gesture — and inside a plugin that thread is the *host's* message thread,
    // which is how a drag turns into an apparently hung DAW.
    const auto started = juce::DragAndDropContainer::performExternalDragDropOfFiles(
        paths,
        false,
        &source,
        [safeThis, count = paths.size()] {
            if (safeThis == nullptr)
                return;

            safeThis->setStatus("drag finished (" + juce::String(count)
                                    + " file(s) offered). Did the drop land?",
                                juce::Colours::limegreen);
        });

    if (!started)
        setStatus("performExternalDragDropOfFiles returned false\n(drag never started)",
                  juce::Colours::orangered);
}

//==============================================================================
DragDropSpike::Pad::Pad(juce::String labelText, int fileCount, DragDropSpike& owner)
    : text(std::move(labelText)), numFiles(fileCount), spike(owner)
{
}

void DragDropSpike::Pad::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);

    graphics.setColour(theme::accent.withAlpha(hovered ? 0.35f : 0.18f));
    graphics.fillRoundedRectangle(bounds, 4.0f);

    graphics.setColour(theme::accent);
    graphics.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    graphics.setColour(juce::Colours::white);
    graphics.setFont(juce::Font(juce::FontOptions(11.0f)));
    graphics.drawText(text, getLocalBounds(), juce::Justification::centred, false);
}

void DragDropSpike::Pad::mouseDown(const juce::MouseEvent&)
{
    dragStartedThisGesture = false;
}

void DragDropSpike::Pad::mouseDrag(const juce::MouseEvent& event)
{
    // One drag per gesture: mouseDrag fires repeatedly, and starting a second
    // OS drag while one is in flight is undefined at best.
    if (dragStartedThisGesture)
        return;

    // A few pixels of slop, so a click with a twitchy hand is not a drag.
    if (event.getDistanceFromDragStart() < 4)
        return;

    dragStartedThisGesture = true;
    spike.beginDrag(*this, numFiles);
}

void DragDropSpike::Pad::mouseUp(const juce::MouseEvent&)
{
    dragStartedThisGesture = false;
}

void DragDropSpike::Pad::mouseEnter(const juce::MouseEvent&)
{
    hovered = true;
    repaint();
}

void DragDropSpike::Pad::mouseExit(const juce::MouseEvent&)
{
    hovered = false;
    repaint();
}
}
