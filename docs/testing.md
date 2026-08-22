# Testing

## Local gate

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug --parallel
ctest --preset windows-debug --output-on-failure
git diff --check
```

## Required test layers

- Unit tests for metadata parsing, filtering, and persistence.
- Integration tests for asynchronous scanning and preview-engine lifecycle.
- Manual smoke tests for launch, resizes, folder selection, preview retriggering, and clean shutdown.

Do not commit commercial sample packs. Use tiny generated or redistributable fixtures only.
