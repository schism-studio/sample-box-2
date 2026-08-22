# Architecture

The standalone application is the first shippable target. It owns desktop lifecycle and UI composition while `core`, `indexing`, and `audio` remain reusable by a future VST3 wrapper.

## Threading boundary

- Indexing performs filesystem traversal and metadata reads off the UI thread.
- The audio preview path must not block on filesystem, database, or UI work.
- UI receives immutable snapshots or messages from background work.

## VST3 later

A future VST3 target should adapt these libraries rather than duplicate their implementation.
