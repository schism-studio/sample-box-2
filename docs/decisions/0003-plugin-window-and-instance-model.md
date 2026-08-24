# 0003: Plug-in Window and Instance Model

## Context

Two questions came out of the Phase 1 audit that both amount to "how much
should the plug-in try to be its own application?"

**The pop-out window.** ADR 0001 decided that `src/ui/PopoutWindow` — a
plug-in-owned detached `juce::DocumentWindow` — would let the browser pop out
of the VST3 editor into its own native window, reframing the original
Ableton-only "device rack vs. floating window" idea as something
host-agnostic. The class was written and compiles, but has never been
instantiated by anything.

Reviewing it against the rest of the audit turned up costs that were not
visible when ADR 0001 was written. A second top-level window owned by a
plug-in is not equivalent to a window owned by an application:

- It can outlive the editor. This is exactly the class of bug found as
  defect D2, where a callback held a raw pointer to an editor a host had
  already destroyed.
- Focus and parenting behaviour is host-specific. Whether a floating window
  from a plug-in stays above the host, receives keyboard events, or survives
  the host going full-screen is not something the plug-in controls.
- It doubles the window-state surface: which window owns the browser at any
  moment, what happens when the editor is reopened while the pop-out is
  visible, and what gets persisted.

**Multiple plug-in instances.** Every instance currently runs its own
`LibraryScanner` and its own `ArtworkCache`. Three instances in one project
means three independent scans of the same library and three copies of the
decoded artwork in memory — significant for the target case of hundreds of
sample packs. The obvious fix is a process-wide index shared between
instances, which the shared-snapshot change (defect D1/D7) would make
straightforward.

But the feature it would support has no purpose. There is no workflow that
two simultaneous Sample Box browsers in one project serves better than one.

## Decisions

- **The VST3 editor uses standard plug-in window behaviour. There is no
  pop-out.** The editor is resizable, and that is the whole of it. This
  supersedes the pop-out decision in ADR 0001.

- **`src/ui/PopoutWindow` is removed** rather than left in place as dead
  code. Keeping a compiling, documented class for a feature that has been
  decided against is worse than not having it: it reads as an unfinished
  intention to anyone opening the directory, and it is carried by both build
  targets for no reason. The Git history is the record, and ADR 0001 plus this
  document explain why it existed and why it does not any more.

- **A single instance per project is the intended use.** Opening several is
  treated as using the product in a way it is not meant for, not as a case to
  engineer around.

- **No shared cross-instance library index or artwork cache will be built.**
  The duplicated scanning and memory cost of multiple instances is accepted
  and left unaddressed.

## Consequences

Two pieces of planned work are cancelled outright: the pop-out window, and
the process-wide shared index. Neither has any dependent work, so nothing
else in the Phase 1 plan changes.

Dropping the pop-out puts real weight on the editor itself being pleasant at
a range of sizes, since there is no escape hatch for someone who wants the
browser larger than the host's plug-in window comfortably allows. Resize
limits are currently 700×450 to 2400×1600, which should be checked against
how the target hosts actually treat a resizable editor.

The single-instance assumption is a stated assumption, not an enforced
constraint. Nothing prevents a user from loading several, and the plug-in
will not warn or refuse — it will simply scan more than it needs to and use
more memory than it needs to. If that turns out to be a common accident
rather than a rare one, the cheap mitigation is a warning rather than a
shared cache, and this decision should be revisited before any shared-state
work is started.

Note that the shared-snapshot work from defect D1/D7 stays exactly as it is.
Its purpose is lifetime safety within a single instance, and it removed two
full deep copies of the snapshot per scan. It happens to also make
cross-instance sharing easy, but that was never why it was done.

## Status

Accepted. Supersedes the pop-out window decision in ADR 0001.
