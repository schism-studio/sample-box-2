# Sample Box

A standalone-first desktop sample browser. The first release proves local library scanning, fast search, and click-free sample auditioning before adding a VST3 adapter.

## Development

Prerequisites: CMake 3.24+, Ninja, a C++20 compiler, and JUCE available at `external/JUCE`.

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug --parallel
ctest --preset windows-debug
```

## Architecture

- `src/core`: domain models and application state
- `src/indexing`: asynchronous library discovery and metadata storage
- `src/audio`: preview transport and waveform work
- `src/ui`: JUCE views and visual theme
- `src/app`: desktop application entry point

See `docs/architecture.md` and `docs/testing.md`.
