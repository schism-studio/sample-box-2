<#
.SYNOPSIS
  Applies the "Options tab" patch to sample-box-2 (Browse | Options tabs, global options).

.DESCRIPTION
  - Creates: AppOptions.h, OptionsStore.h, OptionsComponent.h/.cpp
  - Edits:   CMake, BrowseState.h, CoverArtCard.h/.cpp, CoverArtCarousel.cpp,
             PreviewEngine.h/.cpp, MainPanel.h/.cpp, MainWindow.cpp,
             PluginProcessor.h/.cpp, PluginEditor.cpp
  - All-or-nothing: every anchor is located in memory first. If any anchor is missing
    or ambiguous, NOTHING is written and the problem is listed.
  - Idempotent: edits whose marker text is already present are skipped.
  - Originals are copied to %TEMP%\samplebox-options-patch-<timestamp>\ before writing.

.PARAMETER RepoRoot  Repo root (default: current directory).
.PARAMETER DryRun    Check anchors and report; write nothing.
.PARAMETER Force     Overwrite the 4 new files if they already exist.

.EXAMPLE
  .\apply-options-tab.ps1 -DryRun
  .\apply-options-tab.ps1
#>
[CmdletBinding()]
param(
    [string]$RepoRoot = (Get-Location).Path,
    [switch]$DryRun,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path -LiteralPath $RepoRoot).Path

if (-not (Test-Path -LiteralPath (Join-Path $RepoRoot 'src\ui\MainPanel.cpp'))) {
    Write-Host "ERROR: '$RepoRoot' does not look like the sample-box-2 root (src\ui\MainPanel.cpp not found)." -ForegroundColor Red
    exit 1
}

$errors   = New-Object System.Collections.Generic.List[string]
$texts    = @{}
$orig     = @{}
$nls      = @{}
$bom      = @{}
$newFiles = [ordered]@{}

# ----------------------------------------------------------------- helpers
function Fix-Nl([string]$s, [string]$nl) { [regex]::Replace($s, '\r?\n', $nl) }

function Add-Indent([string]$s, [string]$ind) {
    $lines = $s -split '\r?\n'
    $out = foreach ($l in $lines) { if ($l.Length -gt 0) { $ind + $l } else { $l } }
    return ($out -join "`n")
}

function Load-File([string]$rel) {
    if ($texts.ContainsKey($rel)) { return $true }
    $p = Join-Path $RepoRoot $rel
    if (-not (Test-Path -LiteralPath $p)) { $errors.Add("Missing file: $rel"); return $false }
    $bytes = [IO.File]::ReadAllBytes($p)
    $bom[$rel] = ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF)
    $t = [IO.File]::ReadAllText($p)
    $texts[$rel] = $t
    $orig[$rel]  = $t
    if ($t.Contains("`r`n")) { $nls[$rel] = "`r`n" } else { $nls[$rel] = "`n" }
    return $true
}

# Regex edit: anchor must match exactly once. $Build gets the Match, returns replacement text.
function Edit-Regex([string]$Rel, [string]$Desc, [string]$Marker, [string]$Pattern, [scriptblock]$Build) {
    if (-not (Load-File $Rel)) { return }
    $t = $texts[$Rel]
    if ($t.Contains($Marker)) { Write-Host "  [skip] $Rel :: $Desc (already applied)" -ForegroundColor DarkYellow; return }
    $ms = [regex]::Matches($t, $Pattern, [System.Text.RegularExpressions.RegexOptions]::Multiline)
    if ($ms.Count -ne 1) { $errors.Add("$Rel :: $Desc :: expected exactly 1 anchor match, found $($ms.Count)"); return }
    $m   = $ms[0]
    $rep = Fix-Nl ([string](& $Build $m)) $nls[$Rel]
    $texts[$Rel] = $t.Substring(0, $m.Index) + $rep + $t.Substring($m.Index + $m.Length)
    Write-Host "  [ok]   $Rel :: $Desc" -ForegroundColor Green
}

# Finds the span of a C++ function: header regex (without the '{'), then brace matching.
function Find-FunctionSpan([string]$t, [string]$headerPattern) {
    $ms = [regex]::Matches($t, $headerPattern)
    if ($ms.Count -ne 1) { return $null }
    $m = $ms[0]
    $i = $t.IndexOf('{', $m.Index + $m.Length)
    if ($i -lt 0) { return $null }
    $n = $t.Length; $depth = 0; $j = $i
    while ($j -lt $n) {
        $c = $t[$j]
        if ($c -eq '/' -and ($j + 1) -lt $n -and $t[$j + 1] -eq '/') {
            $e = $t.IndexOf("`n", $j); if ($e -lt 0) { $e = $n - 1 }; $j = $e + 1; continue
        }
        if ($c -eq '/' -and ($j + 1) -lt $n -and $t[$j + 1] -eq '*') {
            $e = $t.IndexOf('*/', $j + 2); if ($e -lt 0) { return $null }; $j = $e + 2; continue
        }
        if ($c -eq '"') {
            $j++
            while ($j -lt $n -and $t[$j] -ne '"') { if ($t[$j] -eq '\') { $j++ }; $j++ }
            $j++; continue
        }
        if ($c -eq "'") {
            $j++
            while ($j -lt $n -and $t[$j] -ne "'") { if ($t[$j] -eq '\') { $j++ }; $j++ }
            $j++; continue
        }
        if ($c -eq '{') { $depth++ }
        elseif ($c -eq '}') { $depth--; if ($depth -eq 0) { return @{ Start = $m.Index; End = $j + 1 } } }
        $j++
    }
    return $null
}

# Replace a whole function (header..closing brace) with new text. $MustContain guards against surprises.
function Replace-Function([string]$Rel, [string]$Desc, [string]$Marker, [string]$HeaderPattern, [string[]]$MustContain, [string]$NewText) {
    if (-not (Load-File $Rel)) { return }
    $t = $texts[$Rel]
    if ($t.Contains($Marker)) { Write-Host "  [skip] $Rel :: $Desc (already applied)" -ForegroundColor DarkYellow; return }
    $span = Find-FunctionSpan $t $HeaderPattern
    if ($null -eq $span) { $errors.Add("$Rel :: $Desc :: could not locate function"); return }
    $old = $t.Substring($span.Start, $span.End - $span.Start)
    foreach ($needle in $MustContain) {
        if (-not $old.Contains($needle)) { $errors.Add("$Rel :: $Desc :: existing function no longer contains '$needle' - refusing to overwrite"); return }
    }
    $texts[$Rel] = $t.Substring(0, $span.Start) + (Fix-Nl $NewText $nls[$Rel]) + $t.Substring($span.End)
    Write-Host "  [ok]   $Rel :: $Desc" -ForegroundColor Green
}

# Insert text right after a whole function.
function Insert-AfterFunction([string]$Rel, [string]$Desc, [string]$Marker, [string]$HeaderPattern, [string]$NewText) {
    if (-not (Load-File $Rel)) { return }
    $t = $texts[$Rel]
    if ($t.Contains($Marker)) { Write-Host "  [skip] $Rel :: $Desc (already applied)" -ForegroundColor DarkYellow; return }
    $span = Find-FunctionSpan $t $HeaderPattern
    if ($null -eq $span) { $errors.Add("$Rel :: $Desc :: could not locate function"); return }
    $nl = $nls[$Rel]
    $texts[$Rel] = $t.Substring(0, $span.End) + $nl + $nl + (Fix-Nl $NewText $nl) + $t.Substring($span.End)
    Write-Host "  [ok]   $Rel :: $Desc" -ForegroundColor Green
}

function Add-NewFile([string]$rel, [string]$content) {
    $p = Join-Path $RepoRoot $rel
    if ((Test-Path -LiteralPath $p) -and -not $Force) {
        Write-Host "  [skip] $rel (exists; use -Force to overwrite)" -ForegroundColor DarkYellow
        return
    }
    $newFiles[$rel] = $content
    Write-Host "  [new]  $rel" -ForegroundColor Green
}

# ----------------------------------------------------------------- new file contents
$appOptionsH = @'
#pragma once
#include "BrowseState.h"

namespace samplebox
{
// Global user options. Persisted by the host (standalone + plug-in share one PropertiesFile).
struct AppOptions
{
    PackViewMode defaultViewMode = PackViewMode::coverFlow;
    bool showCoverTitles = true;      // matches the previous always-on behaviour
    float previewVolume = 1.0f;       // linear gain, 0..1
};
}
'@

$optionsStoreH = @'
#pragma once
#include "../core/AppOptions.h"
#include <juce_data_structures/juce_data_structures.h>

namespace samplebox
{
inline AppOptions loadOptions(juce::PropertiesFile& file)
{
    AppOptions o;
    const int mode = file.getIntValue("defaultViewMode", static_cast<int>(o.defaultViewMode));
    if (mode >= static_cast<int>(PackViewMode::list) && mode <= static_cast<int>(PackViewMode::coverFlow))
        o.defaultViewMode = static_cast<PackViewMode>(mode);
    o.showCoverTitles = file.getBoolValue("showCoverTitles", o.showCoverTitles);
    o.previewVolume = juce::jlimit(0.0f, 1.0f,
        static_cast<float>(file.getDoubleValue("previewVolume", static_cast<double>(o.previewVolume))));
    return o;
}

inline void saveOptions(juce::PropertiesFile& file, const AppOptions& o)
{
    file.setValue("defaultViewMode", static_cast<int>(o.defaultViewMode));
    file.setValue("showCoverTitles", o.showCoverTitles);
    file.setValue("previewVolume", static_cast<double>(o.previewVolume));
    file.saveIfNeeded();
}
}
'@

$optionsComponentH = @'
#pragma once
#include "../core/AppOptions.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace samplebox
{
// Options tab content. Knows nothing about hosts: the owner supplies onChanged.
class OptionsComponent final : public juce::Component
{
public:
    OptionsComponent();

    // Updates the controls without firing onChanged.
    void setOptions(const AppOptions& newOptions);

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void(const AppOptions&)> onChanged;

private:
    void notify() { if (onChanged) onChanged(options); }

    AppOptions options;
    juce::Label viewLabel, volumeLabel;
    juce::ComboBox viewCombo;
    juce::ToggleButton titlesToggle { "Show pack titles on covers" };
    juce::Slider volumeSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsComponent)
};
}
'@

$optionsComponentCpp = @'
#include "OptionsComponent.h"
#include "Theme.h"

namespace samplebox
{
OptionsComponent::OptionsComponent()
{
    viewLabel.setText("Default view", juce::dontSendNotification);
    volumeLabel.setText("Preview volume", juce::dontSendNotification);

    viewCombo.addItem("List", 1);
    viewCombo.addItem("Grid", 2);
    viewCombo.addItem("Cover Flow", 3);
    viewCombo.onChange = [this]
    {
        options.defaultViewMode = static_cast<PackViewMode>(viewCombo.getSelectedId() - 1);
        notify();
    };

    titlesToggle.setColour(juce::ToggleButton::textColourId, theme::textPrimary);
    titlesToggle.onClick = [this]
    {
        options.showCoverTitles = titlesToggle.getToggleState();
        notify();
    };

    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 22);
    volumeSlider.setTextBoxIsEditable(false);
    volumeSlider.textFromValueFunction = [](double v)
    {
        return juce::String(juce::roundToInt(v * 100.0)) + "%";
    };
    volumeSlider.onValueChange = [this]
    {
        options.previewVolume = static_cast<float>(volumeSlider.getValue());
        notify();
    };

    addAndMakeVisible(viewLabel);
    addAndMakeVisible(volumeLabel);
    addAndMakeVisible(viewCombo);
    addAndMakeVisible(titlesToggle);
    addAndMakeVisible(volumeSlider);

    setOptions(options);
}

void OptionsComponent::setOptions(const AppOptions& newOptions)
{
    options = newOptions;
    viewCombo.setSelectedId(static_cast<int>(options.defaultViewMode) + 1, juce::dontSendNotification);
    titlesToggle.setToggleState(options.showCoverTitles, juce::dontSendNotification);
    volumeSlider.setValue(static_cast<double>(options.previewVolume), juce::dontSendNotification);
}

void OptionsComponent::paint(juce::Graphics& g)
{
    g.fillAll(theme::background);
}

void OptionsComponent::resized()
{
    auto area = getLocalBounds().reduced(24, 16);
    constexpr int rowH = 32, labelW = 160, gap = 12, controlW = 360;

    auto row = [&](juce::Label& label, juce::Component& control)
    {
        auto r = area.removeFromTop(rowH);
        label.setBounds(r.removeFromLeft(labelW));
        control.setBounds(r.removeFromLeft(juce::jmin(r.getWidth(), controlW)));
        area.removeFromTop(gap);
    };

    row(viewLabel, viewCombo);

    auto toggleRow = area.removeFromTop(rowH);
    toggleRow.removeFromLeft(labelW);
    titlesToggle.setBounds(toggleRow.removeFromLeft(controlW));
    area.removeFromTop(gap);

    row(volumeLabel, volumeSlider);
}
}
'@

# ----------------------------------------------------------------- snippets for edits
$mainPanelPublic = @'
using OptionsChanged = std::function<void(const AppOptions&)>;

// Call once from the host after construction. Applies the options immediately,
// then reports every later change through onChanged so the host can persist it.
void configureOptions(const AppOptions& initial, OptionsChanged onChanged);
'@

$mainPanelPrivate = @'
enum class Tab { browse, options };
void showTab(Tab tab);
void applyOptions(const AppOptions& o);

Tab currentTab = Tab::browse;
juce::TextButton browseTabButton { "Browse" };
juce::TextButton optionsTabButton { "Options" };
OptionsComponent optionsPanel;
OptionsChanged onOptionsChanged;

'@

$mainPanelCtor = @'

for (auto* b : { &browseTabButton, &optionsTabButton })
{
    b->setClickingTogglesState(true);
    b->setRadioGroupId(1001);
    addAndMakeVisible(*b);
}
browseTabButton.onClick  = [this] { showTab(Tab::browse); };
optionsTabButton.onClick = [this] { showTab(Tab::options); };

addChildComponent(optionsPanel);
optionsPanel.onChanged = [this](const AppOptions& o)
{
    applyOptions(o);
    if (onOptionsChanged)
        onOptionsChanged(o);
};
showTab(Tab::browse);
'@

$mainPanelFunctions = @'
void MainPanel::resized()
{
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop(48).reduced(14, 0);
    stopButton.setBounds(header.removeFromRight(76).reduced(0, 8));
    titleLabel.setBounds(header.removeFromLeft(150));
    browseTabButton.setBounds(header.removeFromLeft(90).reduced(2, 8));
    optionsTabButton.setBounds(header.removeFromLeft(90).reduced(2, 8));

    // Options tab: library-folder strip on top, option rows below.
    auto content = bounds;
    settingsStrip.setBounds(content.removeFromTop(66).reduced(12, 6));
    optionsPanel.setBounds(content);

#if SAMPLEBOX_DRAG_SPIKE
    dragSpike.setBounds(bounds.removeFromBottom(96).removeFromLeft(280).reduced(12, 6));
#endif

    browserView.setBounds(bounds);

    constexpr int toastWidth = 360;
    constexpr int toastHeight = 36;
    toastNotification.setBounds((getWidth() - toastWidth) / 2,
                                getHeight() - toastHeight - 20,
                                toastWidth, toastHeight);
}

void MainPanel::configureOptions(const AppOptions& initial, OptionsChanged onChanged)
{
    onOptionsChanged = std::move(onChanged);
    optionsPanel.setOptions(initial);
    applyOptions(initial);
}

void MainPanel::applyOptions(const AppOptions& o)
{
    browseState.showCoverTitles = o.showCoverTitles;
    browseState.setPackViewMode(o.defaultViewMode);
    browserView.refresh();
}

void MainPanel::showTab(Tab tab)
{
    currentTab = tab;
    const bool opts = (tab == Tab::options);
    browserView.setVisible(!opts);
    optionsPanel.setVisible(opts);
    settingsStrip.setVisible(opts);
    browseTabButton.setToggleState(!opts, juce::dontSendNotification);
    optionsTabButton.setToggleState(opts, juce::dontSendNotification);
    resized();
}
'@

$mainWindowSnippet = @'

// Global options: load, apply, and persist through the shared settings file.
const auto options = loadOptions(*settings);
previewEngine.setVolume(options.previewVolume);
mainPanel.configureOptions(options, [this](const AppOptions& o)
{
    previewEngine.setVolume(o.previewVolume);
    saveOptions(*settings, o);
});
'@

$pluginProcessorFunctions = @'
AppOptions PluginProcessor::getOptions() const
{
    const_cast<PluginProcessor*>(this)->ensureSettings();
    return loadOptions(*settings);
}

void PluginProcessor::setOptions(const AppOptions& o)
{
    ensureSettings();
    saveOptions(*settings, o);
    previewEngine.setVolume(o.previewVolume);
}
'@

$pluginEditorSnippet = @'
// Global options: load, apply, and persist through the processor's settings file.
const auto options = processor.getOptions();
processor.getPreviewEngine().setVolume(options.previewVolume);
mainPanel.configureOptions(options, [this](const AppOptions& o) { processor.setOptions(o); });

'@

# ----------------------------------------------------------------- run
Write-Host ""
Write-Host "Repo: $RepoRoot" -ForegroundColor Cyan
if ($DryRun) { Write-Host "DRY RUN - nothing will be written." -ForegroundColor Cyan }
try {
    $dirty = & git -C $RepoRoot status --short 2>$null
    if ($dirty) { Write-Host "NOTE: working tree has uncommitted changes. Consider committing or stashing first." -ForegroundColor DarkYellow }
} catch { }

Write-Host ""
Write-Host "New files:" -ForegroundColor Cyan
Add-NewFile 'src\core\AppOptions.h'        $appOptionsH
Add-NewFile 'src\ui\OptionsStore.h'        $optionsStoreH
Add-NewFile 'src\ui\OptionsComponent.h'    $optionsComponentH
Add-NewFile 'src\ui\OptionsComponent.cpp'  $optionsComponentCpp

Write-Host ""
Write-Host "Edits:" -ForegroundColor Cyan

# --- CMake
Edit-Regex 'src\ui\CMakeLists.txt' 'add OptionsComponent.cpp to SampleBoxUI' 'OptionsComponent.cpp' `
    '^([ \t]*)MainPanel\.cpp(?=[ \t]*\r?$)' {
    param($m) $i = $m.Groups[1].Value; "${i}MainPanel.cpp`n${i}OptionsComponent.cpp" }

# --- BrowseState.h
Edit-Regex 'src\core\BrowseState.h' 'add showCoverTitles field' 'bool showCoverTitles' `
    '^([ \t]*)double[ \t]+thumbnailZoomCoverFlow[^;\r\n]*;' {
    param($m) $i = $m.Groups[1].Value; $m.Value + "`n" + $i + "bool showCoverTitles = true;" }

# --- CoverArtCard
Edit-Regex 'src\ui\CoverArtCard.h' 'declare setShowTitle' 'void setShowTitle' `
    '^([ \t]*)void[ \t]+setVisualState[^;]*;' {
    param($m) $i = $m.Groups[1].Value; $m.Value + "`n" + $i + "void setShowTitle(bool show);" }

Edit-Regex 'src\ui\CoverArtCard.h' 'add showTitle member' 'bool showTitle' `
    '^([ \t]*)bool[ \t]+selected[^;\r\n()]*;' {
    param($m) $i = $m.Groups[1].Value; $m.Value + "`n" + $i + "bool showTitle = true;" }

Edit-Regex 'src\ui\CoverArtCard.cpp' 'define setShowTitle' 'CoverArtCard::setShowTitle' `
    '^([ \t]*)void[ \t]+CoverArtCard::paint[ \t]*\(' {
    param($m)
    $i = $m.Groups[1].Value
    $i + "void CoverArtCard::setShowTitle(bool show)`n" + $i + "{`n" +
    $i + "    if (showTitle == show)`n" + $i + "        return;`n" +
    $i + "    showTitle = show;`n" + $i + "    repaint();`n" + $i + "}`n`n" +
    $m.Value }

Edit-Regex 'src\ui\CoverArtCard.cpp' 'wrap title drawing in if (showTitle)' 'if (showTitle)' `
    '^([ \t]*)(\w+\.setColour\(theme::textPrimary\);\s*\w+\.setFont\([^;]*;\s*\w+\.drawFittedText\(pack->title[^;]*;)' {
    param($m)
    $i = $m.Groups[1].Value
    $body = $m.Groups[2].Value
    $i + "if (showTitle)`n" + $i + "{`n" + $i + "    " + $body + "`n" + $i + "}" }

# --- CoverArtCarousel.cpp
Edit-Regex 'src\ui\CoverArtCarousel.cpp' 'push showCoverTitles into pooled cards' 'setShowTitle(' `
    '^([ \t]*)(\w+)(\.|->)bindToPack\([^;]*;' {
    param($m)
    $i = $m.Groups[1].Value
    $m.Value + "`n" + $i + $m.Groups[2].Value + $m.Groups[3].Value + "setShowTitle(browseState.showCoverTitles);" }

# --- PreviewEngine
Edit-Regex 'src\audio\PreviewEngine.h' 'declare setVolume' 'void setVolume(float' `
    '^([ \t]*)void[ \t]+stop[ \t]*\([ \t]*\)[ \t]*;' {
    param($m)
    $i = $m.Groups[1].Value
    $m.Value + "`n`n" + $i + "// Linear preview gain (0..1). Thread-safe; applied on the audio thread.`n" +
    $i + "void setVolume(float linearGain) noexcept { volume.store(linearGain, std::memory_order_relaxed); }" }

Edit-Regex 'src\audio\PreviewEngine.h' 'add volume member' 'std::atomic<float> volume' `
    '^([ \t]*)std::atomic<bool>[ \t]+isFadingOut[^;]*;' {
    param($m) $i = $m.Groups[1].Value; $m.Value + "`n" + $i + "std::atomic<float> volume { 1.0f };" }

Edit-Regex 'src\audio\PreviewEngine.cpp' 'apply volume in getNextAudioBlock' 'volume.load(' `
    '^([ \t]*)if[ \t]*\([ \t]*bufferToFill\.buffer[ \t]*==[ \t]*nullptr[^)]*\)[ \t]*(?:\r?\n[ \t]*)?return;' {
    param($m)
    $i = $m.Groups[1].Value
    $m.Value + "`n`n" + $i + "bufferToFill.buffer->applyGain(bufferToFill.startSample, bufferToFill.numSamples,`n" +
    $i + "                               volume.load(std::memory_order_relaxed));" }

# --- MainPanel.h
Edit-Regex 'src\ui\MainPanel.h' 'include OptionsComponent.h' '#include "OptionsComponent.h"' `
    '^([ \t]*)#include[ \t]+"ToastNotification\.h"' {
    param($m) $m.Value + "`n" + $m.Groups[1].Value + '#include "OptionsComponent.h"' }

Edit-Regex 'src\ui\MainPanel.h' 'declare configureOptions' 'configureOptions(' `
    '^([ \t]*)void[ \t]+showToast[ \t]*\([^;]*;' {
    param($m) $i = $m.Groups[1].Value; $m.Value + "`n`n" + (Add-Indent $mainPanelPublic $i) }

Edit-Regex 'src\ui\MainPanel.h' 'add tab/options private members' 'OptionsComponent optionsPanel' `
    '^([ \t]*)JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR[ \t]*\([ \t]*MainPanel[ \t]*\)' {
    param($m) $i = $m.Groups[1].Value; (Add-Indent $mainPanelPrivate $i) + "`n" + $m.Value }

# --- MainPanel.cpp
Edit-Regex 'src\ui\MainPanel.cpp' 'constructor: tab buttons + options wiring' 'browseTabButton.onClick' `
    '^([ \t]*)addAndMakeVisible[ \t]*\([ \t]*browserView[ \t]*\)[ \t]*;' {
    param($m) $i = $m.Groups[1].Value; $m.Value + "`n" + (Add-Indent $mainPanelCtor $i) }

Replace-Function 'src\ui\MainPanel.cpp' 'replace resized() and add configureOptions/applyOptions/showTab' `
    'browseTabButton.setBounds' 'void[ \t]+MainPanel::resized[ \t]*\([ \t]*\)' `
    @('titleLabel.setBounds', 'stopButton.setBounds', 'settingsStrip.setBounds', 'browserView.setBounds') `
    $mainPanelFunctions

# --- MainWindow.cpp (standalone)
Edit-Regex 'src\app\MainWindow.cpp' 'include OptionsStore.h' 'OptionsStore.h' `
    '^[ \t]*#include[ \t]+"MainWindow\.h"' {
    param($m) $m.Value + "`n" + '#include "../ui/OptionsStore.h"' }

Edit-Regex 'src\app\MainWindow.cpp' 'load/apply/persist options in constructor' 'configureOptions(' `
    '^([ \t]*)setContentNonOwned[ \t]*\([^;]*;' {
    param($m) $i = $m.Groups[1].Value; $m.Value + "`n" + (Add-Indent $mainWindowSnippet $i) }

# --- PluginProcessor
Edit-Regex 'src\plugin\PluginProcessor.h' 'include AppOptions.h' '#include "../core/AppOptions.h"' `
    '^[ \t]*#include[ \t]+"\.\./audio/PreviewEngine\.h"' {
    param($m) $m.Value + "`n" + '#include "../core/AppOptions.h"' }

Edit-Regex 'src\plugin\PluginProcessor.h' 'declare getOptions/setOptions' 'AppOptions getOptions' `
    '^([ \t]*)juce::String[ \t]+getSampleLibraryPath[ \t]*\([ \t]*\)[ \t]*const[ \t]*;' {
    param($m)
    $i = $m.Groups[1].Value
    $m.Value + "`n" + $i + "AppOptions getOptions() const;`n" + $i + "void setOptions(const AppOptions& o);" }

Edit-Regex 'src\plugin\PluginProcessor.cpp' 'include OptionsStore.h' 'OptionsStore.h' `
    '^[ \t]*#include[ \t]+"\.\./core/StatePersistence\.h"' {
    param($m) $m.Value + "`n" + '#include "../ui/OptionsStore.h"' }

Insert-AfterFunction 'src\plugin\PluginProcessor.cpp' 'define getOptions/setOptions' 'PluginProcessor::getOptions' `
    'void[ \t]+PluginProcessor::setSampleLibraryPath[ \t]*\([^)]*\)' $pluginProcessorFunctions

# --- PluginEditor.cpp
Edit-Regex 'src\plugin\PluginEditor.cpp' 'load/apply/persist options in constructor' 'configureOptions(' `
    '^([ \t]*)const[ \t]+auto[ \t]+savedPath[ \t]*=[ \t]*processor\.getSampleLibraryPath[ \t]*\(' {
    param($m) $i = $m.Groups[1].Value; (Add-Indent $pluginEditorSnippet $i) + $i + $m.Value.TrimStart() }

# ----------------------------------------------------------------- verdict
Write-Host ""
if ($errors.Count -gt 0) {
    Write-Host "PATCH NOT APPLIED. No files were modified. Problems:" -ForegroundColor Red
    foreach ($e in $errors) { Write-Host "  - $e" -ForegroundColor Red }
    Write-Host ""
    Write-Host "Paste this list back and the anchors can be adjusted to your current source." -ForegroundColor Yellow
    exit 1
}

$changed = @($texts.Keys | Where-Object { $texts[$_] -ne $orig[$_] })
Write-Host ("Ready: {0} file(s) to edit, {1} new file(s)." -f $changed.Count, $newFiles.Count) -ForegroundColor Cyan

if ($DryRun) { Write-Host "Dry run complete. Re-run without -DryRun to apply." -ForegroundColor Cyan; exit 0 }
if ($changed.Count -eq 0 -and $newFiles.Count -eq 0) { Write-Host "Nothing to do (already applied)." -ForegroundColor Cyan; exit 0 }

$stamp     = Get-Date -Format 'yyyyMMdd-HHmmss'
$backupDir = Join-Path $env:TEMP "samplebox-options-patch-$stamp"
foreach ($rel in $changed) {
    $dst = Join-Path $backupDir $rel
    New-Item -ItemType Directory -Force -Path (Split-Path $dst) | Out-Null
    Copy-Item -LiteralPath (Join-Path $RepoRoot $rel) -Destination $dst
}

foreach ($rel in $changed) {
    $enc = New-Object System.Text.UTF8Encoding($bom[$rel])
    [IO.File]::WriteAllText((Join-Path $RepoRoot $rel), $texts[$rel], $enc)
}
foreach ($rel in $newFiles.Keys) {
    $p = Join-Path $RepoRoot $rel
    New-Item -ItemType Directory -Force -Path (Split-Path $p) | Out-Null
    $content = Fix-Nl $newFiles[$rel] "`r`n"
    [IO.File]::WriteAllText($p, $content + "`r`n", (New-Object System.Text.UTF8Encoding($false)))
}

Write-Host ""
Write-Host "Applied. Backups of edited files: $backupDir" -ForegroundColor Green
Write-Host ""
Write-Host "Next (Developer PowerShell, repo root):" -ForegroundColor Cyan
Write-Host "  git status -sb"
Write-Host "  git diff                      # review (q exits the pager)"
Write-Host "  cmake --preset windows-release   # re-configure: a new .cpp was added"
Write-Host "  cmake --build --preset windows-release --parallel"
Write-Host "  ctest --preset windows-release --output-on-failure"
Write-Host ""
Write-Host "To undo: git restore <files>   (and delete the 4 new files), or copy back from the backup folder." -ForegroundColor DarkYellow
