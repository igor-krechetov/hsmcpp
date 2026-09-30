param (
    # Path to build output directory (e.g. CMake build folder)
    [Parameter(Mandatory = $true)]
    [string]$BuildPath,

    # Path where packaging output should be placed
    [Parameter(Mandatory = $true)]
    [string]$InstallPath
)

# Fail fast on any error (important for CI pipelines)
$ErrorActionPreference = "Stop"

# -----------------------------
# Read version from file
# -----------------------------

# Version file is expected at:
#   <build>/version
#
# It must contain a single line like:
#   0.11.2
$VersionFile = Join-Path $BuildPath "version"

if (-not (Test-Path $VersionFile)) {
    throw "Version file not found: $VersionFile"
}

# Read version string and remove whitespace/newlines
$Version = (Get-Content $VersionFile -Raw).Trim()

if ([string]::IsNullOrWhiteSpace($Version)) {
    throw "Version file is empty: $VersionFile"
}
