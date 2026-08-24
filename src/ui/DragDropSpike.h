#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace samplebox
{
// TEMPORARY DIAGNOSTIC — not a product feature. Delete this file, its .cpp,
// the SampleBox_UI source entry, the SAMPLEBOX_DRAG_SPIKE option, and the
// guarded member in MainPanel once the question below is answered.
//
// The question: can a VST3 plugin editor start a native OS file drag that a
// DAW will accept as a file drop?
//
// This matters because native multi-file drag is Sample Box's entire export
// path (ADR 0001), and there is no fallback — ADR 0003 removed the pop-out
// window, so there is no separate top-level window to drag from instead. A
// plugin editor is a child window owned by the host, and a host is free to
// consume the mouse gesture, refuse the drag, or ignore the drop on its own
// arrangement. This is host-specific behaviour that cannot be established by
// reading JUCE's source, only by trying it.
//
// The spike deliberately does NOT touch the library scanner, the artwork
// cache, or the carousel. It synthesizes its own throwaway .wav files in the
// temp directory so that a failure here can only mean the drag mechanism
// itself, and so it works before a library path has ever been set. The
// generated files are audible sine tones, so dropping one into a DAW and
// hearing it proves the file arrived intact rather than merely that a drop
// event occurred.
//
// It reproduces exactly the call FileDragExporter makes, rather than calling
// FileDragExporter, to avoid giving SampleBox_UI a dependency on
// SampleBox_Export that would have to be unpicked when this is deleted. If
// that call is ever changed, change it here too or this stops being a valid
// test of the real path.
class DragDropSpike final : public juce::Component
{
public:
    DragDropSpike();
    ~DragDropSpike() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // A press-and-drag surface. An external drag has to begin inside a live
    // mouse gesture, so this cannot be a button click.
    class Pad final : public juce::Component
    {
    public:
        Pad(juce::String labelText, int fileCount, DragDropSpike& owner);

        void paint(juce::Graphics&) override;
        void mouseDown(const juce::MouseEvent&) override;
        void mouseDrag(const juce::MouseEvent&) override;
        void mouseUp(const juce::MouseEvent&) override;
        void mouseEnter(const juce::MouseEvent&) override;
        void mouseExit(const juce::MouseEvent&) override;

    private:
        juce::String text;
        int numFiles;
        DragDropSpike& spike;
        bool dragStartedThisGesture = false;
        bool hovered = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Pad)
    };

    void beginDrag(Pad& source, int fileCount);
    void setStatus(const juce::String& text, juce::Colour colour);

    // Creates `count` short sine .wav files under the temp directory,
    // reusing them if they already exist. Returns an empty array on failure.
    juce::StringArray ensureTempFiles(int count);

    juce::File spikeDirectory;
    juce::Label titleLabel;
    juce::Label statusLabel;
    Pad singleFilePad { "drag 1 file", 1, *this };
    Pad multiFilePad { "drag 3 files", 3, *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DragDropSpike)
};
}
