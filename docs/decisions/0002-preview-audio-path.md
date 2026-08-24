# 0002: Preview Audio Path

## Context

ADR 0001 recorded that single-sample audition "carries over from the
original Ableton Extension design without modification", and
`PluginProcessor::processBlock` stated the same thing directly:

```cpp
// Pass-through: no processing. Sample previewing happens through the
// editor's own PreviewEngine, not the plugin's audio bus.
```

That cannot work in a VST3, and the reason is structural rather than a
matter of polish. In the standalone application the process owns the audio
device, so a `PreviewEngine` driven by an `AudioDeviceManager` reaches the
hardware directly. Inside a plug-in the **host** owns the device. A plug-in
that opens its own device is either refused outright (WASAPI exclusive mode
is already held by the host), or succeeds in shared mode and produces a
second, independent clock — preview audio drifting against the session with
its own unrelated buffer size and latency.

There is no configuration of a second device that avoids this. The only
audio a VST3 can emit is audio it writes into the buffer the host hands to
`processBlock`.

This was found during the Phase 1 audit, before any audition UI was built.
`PreviewEngine` exists and compiles but has never been instantiated, so no
working behaviour depends on the old arrangement.

## Decisions

- **`PreviewEngine` ownership moves from the editor to `PluginProcessor`.**
  The processor is the only object that sees the host's audio callback, and
  it is also the object with the correct lifetime: a host may close and
  reopen the editor freely, and audition must not stop because someone
  closed the plug-in window. This mirrors the reasoning that already put
  `LibraryScanner` on the processor.

- **`processBlock` mixes preview into the main output bus.** The plug-in
  stops being a strict pass-through. Input is still copied to output
  unmodified; preview is summed on top.

- **The standalone application drives the same `PreviewEngine` through its
  own `AudioDeviceManager` and `AudioSourcePlayer`.** One engine
  implementation, two ways of being pulled. `PreviewEngine` stays a plain
  `juce::AudioSource` so neither host cares how the other works.

- **Preview is suppressed when `isNonRealtime()` is true.** Otherwise an
  offline bounce or track freeze prints whatever happened to be auditioning
  into the render.

- **The audio thread does no file I/O.** `AudioFormatManager::createReaderFor`
  opens and header-parses a file and must stay on the message thread. The
  prepared source is handed to the audio thread only once it is ready to
  play, and `AudioTransportSource::setSource` is never called from
  `processBlock`.

- **Retrigger applies a short fade rather than a hard cut.** The audition
  button is designed for rapid re-rolls, so choking the previous sample
  needs a 5–10 ms fade-out to avoid a click on every press.

## Consequences

Preview audio leaves through the plug-in's output, which means it is subject
to everything downstream: track fader, mute, solo, and any plug-in below
Sample Box in the chain. Auditioning through a channel carrying an EQ and a
limiter is not hearing the raw sample. This is inherent to being a VST3, not
a defect — but it should be surfaced in the UI rather than left to surprise
people, and it is the main argument for the placement guidance below.

Recommended placement is therefore a **dedicated audio track**, not the
master bus. The track fader becomes a natural preview level control, the
track can be soloed to hear a sample against silence, and the browser stays
out of the mix path of the whole project. The master bus works, since the
plug-in passes audio through unaltered, but it couples a browser to the
final mix bus for no benefit.

Some hosts stop calling `processBlock` while the transport is stopped
(Reaper exposes this as a setting; Live keeps its engine running). Where that
happens, preview will stall. This needs confirming per host rather than
assuming, and there is no in-plug-in workaround if a host declines to call
us.

`isBusesLayoutSupported` currently requires the input channel set to equal
the output channel set, which is correct for a pass-through but means the
plug-in cannot be instantiated with no input at all. Left unchanged for now;
revisit only if a host proves awkward about it.

## Status

Accepted, and supersedes the audition portion of ADR 0001. Implementation
follows in subsequent commits, as the riskiest step in the Phase 1 sequence:
it changes what the plug-in does to the audio bus, so it is to be verified
by loading the VST3 in a DAW before any audition UI is layered on top.
