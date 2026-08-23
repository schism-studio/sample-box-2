# 0001: VST3 Export Redesign

## Context

Sample Box began as an Ableton Live Extension prototype (Node.js backend +
WebGL/Three.js WebView, driving Ableton's Live Object Model) before
pivoting to a cross-DAW VST3 plugin. A VST3 has no object-model access in
any host, which forced a redesign of the features that previously depended
on Live's API.

## Decisions

- **Full-pack drag export uses curated random sampling, not a
  folder-mirrored hierarchy.** The original design (`LiveApiBridge` /
  `HierarchyBuilder` creating Group/Audio tracks via LOM) cannot be reused
  as-is outside Live. Dragging a pack's cover art now selects one random
  eligible file per subfolder, topping up with extra random files if the
  pack has fewer subfolders than the target, producing a tweakable 8–16
  sample set exported via plain native OS multi-file drag
  (`src/export/CuratedSampleSelector`, `src/export/FileDragExporter`), so
  behaviour stays consistent across DAWs.
- **Ableton-specific group-track hierarchy is demoted to a separate
  setting.** Since most users are expected to be on Ableton, the richer
  folder-mirrored Group/Audio Track export is kept as an Ableton-only mode
  (`src/export/AbletonHierarchyExporter`) rather than a universal VST3
  requirement.
- **The left library sidebar no longer needs to be synchronized.**
  Real-time reactive sync between the browser UI and a DAW's own sample
  library tree was dropped as a requirement; `src/ui/SidebarBrowser` is a
  plain, non-reactive folder/pack list.
- **The pop-out window is reframed as a generic multi-DAW browser.** The
  original "Live device rack vs. floating window" framing (Live-specific
  docking) is replaced by `src/ui/PopoutWindow`, a plugin-owned detached
  `juce::DocumentWindow` that behaves identically in any VST3 host.
- **Single-sample audition and export are unchanged.** Monophonic
  instant-choke preview (front-cover Play button, `src/audio/PreviewEngine`)
  and native OS single-file drag of a waveform badge both carry over from
  the original Ableton Extension design without modification, since
  neither ever depended on Live's API.

## Status

Accepted. Recorded alongside a matching entry in this project's knowledge
wiki. Implemented as compiling skeletons in this same change — actual
export/audio-engine logic (drag payload polish, waveform rendering,
Ableton group-track wiring) is follow-up work.
