# 1. Confirm you are in the intended repository and branch.
git status -sb

# 2. Syntax-check the patch BEFORE executing it.
$errors = $null
$tokens = $null
[System.Management.Automation.Language.Parser]::ParseFile(
    ".\apply-feature.ps1",
    [ref] $tokens,
    [ref] $errors
) | Out-Null
$errors | Format-List

# 3. Only if the prior command printed nothing:
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\apply-feature.ps1 -DryRun

# 4. Review dry-run output, then apply.
.\apply-feature.ps1

# 5. Confirm real source changes before building.
git diff --check
git diff --stat
git diff