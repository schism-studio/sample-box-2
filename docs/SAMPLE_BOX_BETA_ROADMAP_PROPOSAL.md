# Sample Box - Beta Roadmap Proposal

**Date:** 2026-09-30 | **Repo:** `schism-studio/sample-box-2` | **Baseline:** `beta` @ `c2b27b0`
**Deliverable:** standalone app + VST3 sample-pack browser (Windows x64)
**Status:** PROPOSAL for review. No code has been changed.

Sources merged: Beta Handoff (2026-09-30), Feature Scope list (user), Phase 1 Audit, Audition & Flip spec, Cover Flow visual target, Safe Git Workflow.

---

## 1. Guiding principles

1. **Stability before spectacle.** Every phase ends on a green `scripts\build.bat clean release`, passing tests, and an Ableton load check.
2. **One shared UI.** Standalone and VST3 share `MainPanel`. Nothing forks.
3. **Immutable snapshot, derived views.** Search, filters, history and views read the in-memory `LibrarySnapshot`; no filesystem hits from the UI.
4. **Message thread stays clean.** No file I/O, image decode or scanning there. No I/O, allocation or locks on the audio thread.
5. **Cosmetic and complex features go last**, behind a renderer abstraction, so they can't destabilize the plugin.
6. **Local-only agent work.** One branch per task off `beta`, commit locally, no pushing by the agent. You review, then push and PR.

## 2. Feature inventory

Your scope list plus carry-over items from the audits. Status "verify" = unconfirmed in current code; check before building.

| ID | Feature | Source | Complexity | Status |
|---|---|---|---|---|
| S1 | Plugin state persistence (per-instance path, view, settings) | Audit | Low | verify (stubs at audit) |
| S2 | Selection/BrowseState model (one selection concept) | Audit | Low | planned |
| S3 | Scan status UI (counts, unreadable dirs, cancelled) | Audit | Low | planned |
| S4 | Global stop button | Scope | Low | planned |
| S5 | Options tab (user defaults) | Scope | Low-Med | planned |
| S6 | View tabs: Grid / Carousel / Coverflow | Scope | Med | partial (`multi-view` branch) |
| S7 | Search bar with fuzzy filter | Scope | Med | planned |
| S8 | Alphabet index strip (scrollable, popped-out letter) | Scope | Med | planned |
| S9 | Cover tile titles option (default off) | Scope | Low | planned |
| S10 | Audition polish (shuffle bag, fades, SR correction, bounce suppression) | Spec | Med | verify |
| S11 | Preview audio path in processor output bus (ADR 0002) | Spec | High | verify (handoff says in place) |
| S12 | Drag cover into DAW: N random samples, count set in Options | Scope | Med-High | partial (`configurable-random-pack-drag`) |
| S13 | Drag sample rows out (native cross-DAW) | Audit | Med | spike passed |
| S14 | History side panel (retractable; played titles; draggable) | Scope | Med | planned |
| S15 | Global "last triggered" waveform clip-art button, draggable during playback | Scope | Med | planned |
| S16 | "Sample Box" - build your own pack by dragging into a local box; save externally | Scope | High | planned |
| S17 | Artwork scraper for packs | Scope | High | planned (needs network policy) |
| S18 | Async artwork pipeline, bounded LRU, placeholders | Audit | Med | partly shipped (D4) |
| S19 | 3D Cover Flow renderer (Y-rotation, reflections, snap, momentum) | Visual | High | planned |
| S20 | Coverflow-only flip to per-pack file browser (audio/folders first, images/extras last) | Scope | High | planned |
| S21 | Lightweight DRM | Scope | Very high | needs design doc |
| S22 | Waveform cache + metadata (duration/rate/channels) | Audit | Med-High | post-beta |
| S23 | Release tooling: CI, tags, packaging | Audit | Med | post-beta |

Notes:
- S20 is now **coverflow-only** per your scope. This changes the earlier spec, which flipped in any view. Grid and Carousel need a different drill-down (Phase 3).
- S21's design doc was referenced ("expanded upon in other doc") but is not in the project files. It must be supplied before scheduling.

## 3. Phased plan

### Phase 0 - Stabilize the baseline (gate)
**Goal:** know exactly what we have.
- Rebuild merged `beta` clean; run all tests.
- Confirm PR #3 in Ableton (no blank cards for empty/artwork-only folders).
- Delete stray `criptsbuild.bat clean release`.
- Resolve which branch is the "architecture restructure" (candidates `73117ba`, `36cc37a`); record in handoff. Do not merge or delete either.
- Verify S1, S10, S11 status by reading current source.
- Decide `IS_SYNTH TRUE/FALSE` (recommend TRUE for MIDI-track loading).
- Tag `beta-phase0-green`.
**Exit:** clean build, tests green, VST3 loads in Ableton, open questions recorded.

### Phase 1 - Foundation and state
**Features:** S1, S2, S3, S4, S5.
**Why first:** every later feature reads or writes selection, settings, or playback state. S5 (Options) is the home for the random-drag count, title toggle and default view.
**Exit:** settings and state survive a DAW project reload; global stop works; scan status visible.

### Phase 2 - Audio and interaction core
**Features:** S11 (if not already done), S10, S13, S12, S14, S15.
**Order matters:** audio path and audition polish first; load VST3 in Ableton before any UI is layered on. Then drag-out of rows (S13), then cover drag with N random samples (S12), then the history panel (S14) and last-triggered clip-art button (S15), which both consume the same "triggered sample" event.
**Risk:** highest of all phases (audio thread, plugin bus, drag behavior inside hosts).
**Exit:** audition click-free and pitch-correct at 44.1/48 kHz; drag works into Ableton while audio plays; preview absent from offline bounce.

### Phase 3 - Browse experience
**Features:** S6 (view tabs), S7 (fuzzy search), S8 (alphabet index), S9 (title option), S18 (artwork pipeline completion), plus a simple pack drill-down for Grid/Carousel views.
**Rule:** Coverflow tab exists in Phase 3 as the current pseudo-3D carousel; the real renderer arrives in Phase 5.
**Exit:** all three tabs switch without losing selection; search and alphabet jump respond instantly on a large library.

### Phase 4 - Creation tools
**Features:** S16 (Sample Box), then S17 (artwork scraper).
- S16: local "box" staging area with drag in, reorder/remove, and explicit export to a chosen folder (copy, never mutate originals). Session-scoped with save prompt.
- S17: opt-in only, rate-limited, cached on disk, never blocks UI; needs a decision on sources, licensing and offline behavior. Keep it isolated so the plugin works fully without network.
**Exit:** a pack built in the box exports correctly and opens in a DAW.

### Phase 5 - Visual showpiece
**Features:** S19 (Cover Flow renderer), S20 (coverflow-only flip to file browser).
- Introduce a renderer abstraction first; ship pure `juce::Graphics` strip-sliced perspective. OpenGL stays out of scope for the VST3.
- S20 reverse face: virtualized `ListBox`; audio files and folders first, images and extras last; click row to audition; Escape flips back and restores scroll.
**Exit:** idle CPU unchanged between 40-pack and 1,000-pack libraries; identical look in standalone and VST3.

### Phase 6 - Protection and release
**Features:** S21 (DRM), S22 (waveform/metadata), S23 (CI, tags, packaging).
- DRM: write the design doc first (threat model, offline activation, what is protected, user friction), then a review gate before any code. It must not introduce network dependency at load time or risk host stability.
- Package the standalone and VST3 together; tag a release from `main` after promotion.

## 4. Dependencies

```text
Phase 0 -> Phase 1 (state, settings, selection)
Phase 1 -> Phase 2 (audio/drag need selection + settings)
Phase 2 -> Phase 3 (history/clip-art events feed UI; search works on snapshot)
Phase 3 -> Phase 5 (renderer + flip need view tabs and artwork pipeline)
Phase 2 -> Phase 4 (Sample Box reuses drag plumbing)
Phase 6 is independent of 4/5 except DRM design, which should start early as a document.
```

## 5. Risk register

| Risk | Phase | Mitigation |
|---|---|---|
| Audio-thread violations in preview | 2 | No I/O/alloc on audio thread; reader prepared off-thread; Ableton test after each change |
| Drag behavior differs per host | 2, 4 | Native file drag only; define fallback messaging; test Ableton first |
| Scan or artwork work hitches UI on large libraries | 3 | Filter the snapshot; async decode; bounded cache; 10k-sample synthetic test |
| Renderer cost or OpenGL issues in VST3 | 5 | Pure-Graphics renderer, cached bitmaps, OpenGL off by default |
| Scraper legal/network issues | 4 | Opt-in, cached, isolated, source decision before build |
| DRM breaks loading or hosts | 6 | Design doc + review gate; no load-time network |
| Scope creep mid-phase | all | Finish exit criteria before starting the next phase |

## 6. Working method for every task

1. `git switch beta; git pull --ff-only origin beta; git switch -c feat/<name>`.
2. Implement one item; commit locally with detailed notes (what, why, risks, test results).
3. Verify: `git diff --check`, `git diff --staged --stat`, `scripts\build.bat clean release`.
4. You push, open a PR into `beta`, run the Ableton check, merge as `Merge PR #N: ...`.
5. Tag each phase exit (`beta-phaseN-green`) for instant rollback.
6. Never commit `build/`, JUCE contents, binaries, or commercial samples. Keep `external/JUCE/.gitkeep`.

## 7. Open decisions

1. Which branch is the "architecture restructure"?
2. `IS_SYNTH TRUE` (instrument, MIDI track) or `FALSE` (effect)?
3. DRM design doc: where is it, or should one be drafted?
4. Artwork scraper: which sources, and opt-in or automatic?
5. Drill-down for Grid/Carousel views: same file browser as coverflow, or simpler list?
6. Sample Box export: copy files only, or also generate metadata (e.g. manifest)?
7. Is the history panel session-only or persisted between sessions?
8. Beta scope cut: does beta include Phase 4, or is beta = Phases 0-3 + 5, with 4 and 6 post-beta?

## 8. Suggested beta definition

**Beta = Phases 0-3 (stable, browsable, auditionable, draggable) plus Phase 5 if time allows.** Phases 4 and 6 (Sample Box, scraper, DRM, release tooling) follow as 1.0 work. This keeps the first shippable build small, testable and stable.
