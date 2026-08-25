# Sample Box VST3 — Ableton Status and Engineering Handoff

**Date:** 2026-08-25  
**Repository:** `schism-studio/sample-box-2`  
**Baseline commit:** *(to be filled after this commit)*  
**JUCE revision:** `7aae7d8e8deb8413bb01633d2795ef9974a181c5` (local at `external/JUCE`)

---

## 1. Executive summary

- The **standalone application** builds and runs correctly on Windows.
- The **VST3 bundle** builds successfully and is structurally valid.
- Ableton Live 11 and 12 **do not register** the VST3 in the browser, despite:
  - Correct installation to `C:\Program Files\Common Files\VST3`.
  - Valid `.vst3` bundle layout.
  - Release runtime dependencies (no Debug CRT).
  - Exports: `GetPluginFactory`, `InitDll`, `ExitDll`.
  - Successful Windows DLL load test.
- **pluginval** (Tracktion) can **discover** the plug-in but fails to **open** it:
  - `Unable to load VST-3 plug-in file`
  - `Unable to create juce::AudioPluginInstance`
- The failure is therefore a **VST3 component instantiation contract** issue, not a path, bundle, or Ableton-version issue.

The next agent should treat this as a **VST3 host-integration defect** to be diagnosed at the factory/processor level, not as a UI or cache issue.

---

## 2. What was accomplished this session

### 2.1 Artwork cache and Cover Flow stability

- Implemented an **asynchronous, bounded artwork cache** (`ArtworkCache`) with:
  - One background worker thread.
  - Per-key pending-load tracking.
  - Completion callbacks on the JUCE message thread.
  - LRU eviction.
- Fixed **Cover Flow flicker** by:
  - Using a **stable thumbnail size** (320×³20) for Cover Flow instead of animated card size.
  - Ensuring each pack has a single cache key regardless of scroll animation.
- Updated `CoverArtCard` to:
  - Use `juce::Component::SafePointer` in the completion callback.
  - Repaint only when the specific thumbnail becomes available.

**Result:** Standalone artwork behavior is stable under scroll; no placeholder/image cycling.

Files changed:
- `src/UI/ArtworkCache.h`
- `src/UI/ArtworkCache.cpp`
- `src/UI/CoverArtCard.cpp`

### 2.2 Build and deployment tooling

- Added **Windows Release installer script**:
  - `scripts/install-release-vst3.ps1`
  - Configures, builds, tests, and installs the Release VST3 bundle.
  - Verifies installed binary hash.
  - Requires Administrator Developer PowerShell.
- Added **Debug installer script**:
  - `scripts/install-debug-vst3.ps1`
  - Installs Debug VST3 bundle (used for local iteration).
- Fixed **Release build** by enabling C++ exceptions:
  - Added to `CMakeLists.txt`:

    ```cmake
    if (MSVC)
      add_compile_options(/EHsc)
    endif()
    ```

**Result:** Both Debug and Release configurations build and test cleanly in Developer PowerShell.

---

## 3. Current VST3 status in Ableton

### 3.1 Verified correct properties

- Installation path:

  ```text
  C:\Program Files\Common Files\VST3\Sample Box.vst3
  ```

- Bundle structure:

  ```text
  Sample Box.vst3
  └── Contents
      ├── Resources
      │   └── moduleinfo.json
      └── x86-win
          └── Sample Box.vst3
  ```

- `moduleinfo.json` contains:
  - Name: `Sample Box`
  - Vendor: `Schism Studio`
  - SDKVersion: `VST 3.8.0`
  - Two classes: Audio Module and Component Controller, with CIDs.
- Exports (via `dumpbin /exports`):

  ```text
  GetPluginFactory
  InitDll
  ExitDll
  _GetPluginFactory@0
  ```

- Windows DLL load test: **successful** (no missing dependencies).
- Runtime dependencies: **Release CRT only** (`MSVCP140.dll`, `VCRUNTIME140.dll`).

### 3.2 Host behavior

- Ableton Live 11.3.43 and Live 12.4.3:
  - Scan `C:\Program Files\Common Files\VST3`.
  - Log:

    ```text
    warning: VST3: couldn't determine fingerprint for plugin candidate:
    "C:\Program Files\Common Files\VST3\Sample Box.vst3"
    ```

  - Do **not** list `Sample Box` in the Plug-ins browser.
- pluginval:
  - Discovers the plug-in:

    ```text
    Testing plugin: VST3-Sample Box-8facefc9-bad426ef
    Schism Studio: Sample Box v0.1.0
    ```

  - Fails at open tests:

    ```text
    Unable to load VST-3 plug-in file
    Unable to create juce::AudioPluginInstance
    ```

### 3.3 Interpretation

- The VST3 is **discoverable** but **not instantiable**.
- The failure occurs before audio, editor, or parameter tests.
- This is consistent with:
  - A VST3 factory/processor contract mismatch.
  - A JUCE/VST3 generation or configuration issue.
  - A bus-layout or lifecycle incompatibility that prevents host creation.

---

## 4. Known code-level facts

### 4.1 VST3 target configuration (`src/plugin/CMakeLists.txt`)

```cmake
juce_add_plugin(SampleBox_VST3
  COMPANY_NAME "Schism Studio"
  PRODUCT_NAME "Sample Box"
  BUNDLE_ID "com.schismstudio.samplebox"
  PLUGIN_MANUFACTURER_CODE Schs
  PLUGIN_CODE Sbox
  FORMATS VST3
  VST3_CATEGORIES Tools
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)
```

- `PLUGIN_MANUFACTURER_CODE` / `PLUGIN_CODE` are placeholders.
- `IS_SYNTH FALSE` configures it as an audio effect.
- `VST3_CATEGORIES Tools` is valid but narrow.

### 4.2 Processor buses (`src/plugin/PluginProcessor.cpp`)

```cpp
PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}
```

- Stereo input and stereo output.
- `isBusesLayoutSupported` enforces mono→mono or stereo→stereo with matching I/O.

### 4.3 Preview engine integration

- `PluginProcessor` owns `PreviewEngine` and mixes preview output into the process block.
- `processBlock` is a pass-through with preview mixed on top.
- `loadSettings()` is called in the constructor, using a `juce::PropertiesFile`.

---

## 5. Outstanding VST3 defects

### D-VST3-1 — Host cannot instantiate the VST3

**Symptom:** Ableton and pluginval cannot create an instance.  
**Evidence:** pluginval logs:

```text
Unable to load VST-3 plug-in file
Unable to create juce::AudioPluginInstance
```

**Likely causes (ranked):**

1. JUCE/VST3 generation or factory mismatch (e.g., component/controller identity, CIDs, or SDK version).
2. Bus layout or `IS_SYNTH` configuration incompatible with Ableton’s expectations for a “Tools” effect.
3. Constructor-side initialization (e.g., `PropertiesFile`, `PreviewEngine`) failing in a headless scanner.

**Next diagnostics:**

- Run pluginval with lower strictness (1–3) and capture full logs.
- Add a minimal VST3 factory probe executable that:
  - Loads the VST3 DLL.
  - Calls `InitDll`.
  - Calls `GetPluginFactory`.
  - Enumerates classes and prints CIDs/categories.
  - Reports any exceptions or failures.
- Compare factory-reported CIDs with `moduleinfo.json`.
- Test a minimal JUCE VST3 example (e.g., `JUCE/examples/Plugins/`) built with the same JUCE revision to confirm host behavior.

**Likely repairs:**

- Adjust `IS_SYNTH` / bus configuration to match Ableton’s expectations for a browser/instrument.
- Ensure factory/component/controller identity is coherent and unique.
- Defer `PropertiesFile` / heavy initialization out of the processor constructor.

---

## 6. Completed feature work (standalone)

### 6.1 Artwork cache

- `ArtworkCache`:
  - Async decode on a single background thread.
  - Per-key pending-load set.
  - Completion callbacks on the message thread.
  - LRU eviction with configurable max entries.
- `CoverArtCard`:
  - Uses `SafePointer` in completion callback.
  - Requests a stable 320×³20 thumbnail for Cover Flow.

### 6.2 Build/deploy scripts

- `scripts/install-debug-vst3.ps1`
- `scripts/install-release-vst3.ps1`
- Both:
  - Require Administrator Developer PowerShell.
  - Install into `C:\Program Files\Common Files\VST3`.
  - Verify installed binary hash.

---

## 7. Recommended next engineering steps

### 7.1 VST3 host-integration repair (highest priority)

1. **Factory probe**
   - Implement a small CLI tool that calls `GetPluginFactory` and prints class metadata.
   - Confirm that CIDs and categories match `moduleinfo.json`.

2. **Bus / synth configuration experiment**
   - Temporarily set:

     ```cmake
     IS_SYNTH TRUE
     NEEDS_MIDI_INPUT FALSE
     NEEDS_MIDI_OUTPUT FALSE
     IS_MIDI_EFFECT FALSE
     ```

   - Rebuild Release, reinstall, retest in pluginval and Ableton.
   - If this resolves instantiation, document the minimal valid configuration for Ableton.

3. **Constructor-side initialization audit**
   - Move `loadSettings()` out of the processor constructor.
   - Ensure `PreviewEngine` construction cannot throw in a headless host.
   - Re-test.

4. **JUCE/VST3 alignment**
   - Confirm that the pinned JUCE revision is compatible with VST 3.8 metadata.
   - If needed, adjust JUCE VST3 wrapper or SDK alignment.

### 7.2 Phase 1 feature completion (per `PHASE1_CODE_AUDIT_AND_PLAN.md`)

Once the VST3 instantiates reliably:

1. **Hardening commits**
   - D1: Hold library snapshot as shared immutable state.
   - D2: Guard scan completion against destroyed editors.
   - D3: Idle carousel animation clock when settled.
   - D5–D8: Scan performance and edge-case improvements.

2. **Feature commits**
   - Shared browse state and pack drill-down navigation.
   - Sample shelf view for the focused pack.
   - Search and file-type filtering over the snapshot.
   - Perspective renderer refinements for depth, shadows, and reflections.

### 7.3 Phase 2 — Audition/preview

- Move `PreviewEngine` ownership clearly to the processor.
- Wire preview to a UI control (e.g., random audition button on the focused pack).
- Add playback progress ring and now-playing label.
- Ensure click-free choke and sample-rate correction.

### 7.4 Phase 3 — Export/drag

- Implement native cross-DAW file drag/export.
- Keep Ableton-specific hierarchy export optional and isolated.

### 7.5 Phase 4+ — Visual and metadata enhancements

- Complete cover-art browser skeleton.
- Waveform generation/cache.
- Duration, sample rate, channels, and optional tempo/key metadata.

---

## 8. Build and test commands (Windows)

From **Developer PowerShell for VS 2022**:

```powershell
# Configure/build/test Debug
cmake --preset windows-debug
cmake --build --preset windows-debug --parallel
ctest --preset windows-debug --output-on-failure

# Configure/build/test Release
cmake --preset windows-release
cmake --build --preset windows-release --parallel
ctest --preset windows-release --output-on-failure
```

Install VST3 (Administrator shell):

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\install-release-vst3.ps1
```

Validate with pluginval:

```text
pluginval.exe ^
  --strictness 5 ^
  --validate "C:\Program Files\Common Files\VST3\Sample Box.vst3"
```

---

## 9. Git hygiene

- Do not commit `build/`, `external/JUCE` contents, binaries, or machine paths.
- Preserve `external/JUCE/.gitkeep`.
- Keep `main` as a stable, buildable baseline.
- Feature branch suggestion:

  ```text
  feat/library-browser-workflow
  ```

- Suggested next commit message after this handoff:

  ```text
  build: enable MSVC exceptions and add VST3 installer scripts
  ```

---

## 10. References

- `SAMPLE_BOX_AGENT_HANDOFF_2026-08-24.md`
- `PHASE1_CODE_AUDIT_AND_PLAN.md`
- `FEATURE_AUDITION_AND_FLIP.md`
- `VISUAL_TARGET_COVERFLOW.md`
- Ableton help: “Using VST plug-ins on Windows”
- pluginval: https://github.com/Tracktion/pluginval

---

**End of handoff.**