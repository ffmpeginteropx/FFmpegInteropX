param(
    
    # If a version string is specified, a NuGet package will be created.
    [string] $LibPackageVersion,

    # If a version string is specified, a NuGet package will be created.
    [string] $FFmpegPackageVersion,

    # If a version string is specified, a NuGet package will be created.
    [string] $OverallPackageVersion = $null,
    
    # The version number to set in the dll file
    [version] $LibraryVersionNumber = $null,

    <#
        Example values:
        14.1
        14.2
        14.16
        14.16.27023
        14.23.27820

        Note. The PlatformToolset will be inferred from this value ('v141', 'v142'...)
    #>
    [version] $VcVersion = '14.5',

    <#
        Example values:
        8.1
        10.0.15063.0
        10.0.17763.0
        10.0.18362.0
    #>
    [version] $WindowsTargetPlatformVersion = '10.0.26100.0',

    <#
        Example values:
        8.1
        10.0.15063.0
        10.0.17763.0
        10.0.18362.0
    #>
    [version] $WindowsTargetPlatformMinVersion = '10.0.17763.0',

    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    # Set the search criteria for VSWHERE.EXE.
    [string[]] $VsWhereCriteria = '-latest',

    [System.IO.FileInfo] $BashExe = 'C:\msys64\usr\bin\bash.exe',

    [string] $NuGetPackageSource = 'https://api.nuget.org/v3/index.json',

    [switch] $ClearBuildFolders,

    [switch] $AllowParallelBuilds,
    
    [switch] $SkipBuildLibs,
    
    [switch] $SkipConfigureFFmpeg

)

if (!$LibraryVersionNumber)
{
    # Get LibraryVersionNumber from LibraryPackageNumber (remove prerelease suffix if present, add .0 if missing)
    $LibraryVersionNumber = [version]($LibPackageVersion -replace '-.*$', '') # Remove prerelease suffix
    if ($LibraryVersionNumber.Revision -eq -1) {
        $LibraryVersionNumber = [version]::new($LibraryVersionNumber.Major, $LibraryVersionNumber.Minor, $LibraryVersionNumber.Build, 0)
    }
}

if (!$OverallPackageVersion)
{
    # Get version from LibPackageVersion (remove prerelease suffix if present)
    $libPart = $LibPackageVersion -replace '-.*$', '' # Remove prerelease suffix

    # Get Revision by concatenating first three parts of FFmpeg version
    $ffmpegPart = $FFmpegPackageVersion -replace '-.*$', '' # Remove prerelease suffix
    $revisionPart = [string]::Join("", ($ffmpegPart -split '\.' | Select-Object -First 3))
    $revisionEnd = "00"

    # Get prerelease suffix from LibPackageVersion (if present)
    $prereleasePart = $LibPackageVersion -replace '^[0-9]+\.[0-9]+\.[0-9]+', '' # Remove first three parts
    $OverallPackageVersion = "$libPart.$revisionPart$revisionEnd$prereleasePart"
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

Write-Host
Write-Host "Releasing FFmpegInteropX..."
Write-Host
Write-Host "Overall Package Version:   $OverallPackageVersion"
Write-Host "Lib Package Version:       $LibPackageVersion"
Write-Host "Lib DLL Version:           $LibraryVersionNumber"
Write-Host "FFmpeg Package Version:    $FFmpegPackageVersion"
Write-Host
Write-Host "Timestamp: $timestamp"
Write-Host

# Stop on all PowerShell command errors
$ErrorActionPreference = "Stop"


$LibPackages = @(
    ".\Output\NuGet\FFmpegInteropX.Desktop.Lib.$LibPackageVersion.nupkg"
    ".\Output\NuGet\FFmpegInteropX.UWP.Lib.$LibPackageVersion.nupkg"
)

$FFmpegPackages = @(
    ".\Output\NuGet\FFmpegInteropX.Desktop.FFmpeg.$FFmpegPackageVersion.nupkg"
    ".\Output\NuGet\FFmpegInteropX.UWP.FFmpeg.$FFmpegPackageVersion.nupkg"
)

$OverallPackages = @(
    ".\Output\NuGet\FFmpegInteropX.$OverallPackageVersion.nupkg"
    ".\Output\NuGet\FFmpegInteropX.UWP.$OverallPackageVersion.nupkg"
)

if ((!(Test-Path $LibPackages[0])) -or (!(Test-Path $LibPackages[1])))
{
    Write-Host
    Write-Host "Building FFmpegInteropX Lib..."
    Write-Host

    .\Build-FFmpegInteropX.ps1 `
        -VcVersion $VcVersion `
        -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion `
        -WindowsTargetPlatformMinVersion $WindowsTargetPlatformMinVersion `
        -VcVersion $VcVersion `
        -LibraryVersionNumber $LibraryVersionNumber `
        -NugetPackageVersion $LibPackageVersion `
        -ClearBuildFolders:$ClearBuildFolders `
        -AllowParallelBuilds:$AllowParallelBuilds
    
    if ((!(Test-Path $LibPackages[0])) -or (!(Test-Path $LibPackages[1])))
    {
        Write-Warning "Failed to build FFmpegInteropX Lib..."
        Exit 1
    }
}

if ((!(Test-Path $FFmpegPackages[0])) -or (!(Test-Path $FFmpegPackages[1])))
{
    Write-Host
    Write-Host "Building FFmpegInteropX FFmpeg..."
    Write-Host

    .\Build-FFmpeg.ps1 `
        -VcVersion $VcVersion `
        -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion `
        -WindowsTargetPlatformMinVersion $WindowsTargetPlatformMinVersion `
        -NugetPackageVersion $FFmpegPackageVersion `
        -ClearBuildFolders:$ClearBuildFolders `
        -AllowParallelBuilds:$AllowParallelBuilds

    if ((!(Test-Path $FFmpegPackages[0])) -or (!(Test-Path $FFmpegPackages[1])))
    {
        Write-Warning "Failed to build FFmpegInteropX FFmpeg."
        Exit 1
    }
}

if ((!(Test-Path $OverallPackages[0])) -or (!(Test-Path $OverallPackages[1])))
{
    Write-Host
    Write-Host "Building FFmpegInteropX Overall Packages..."
    Write-Host

    .\Build-OverallPackage.ps1 `
        -OverallPackageVersion $OverallPackageVersion `
        -FFmpegPackageVersion $FFmpegPackageVersion `
        -LibPackageVersion $LibPackageVersion `
        -WindowsTargetPlatformMinVersion $WindowsTargetPlatformMinVersion

    if ((!(Test-Path $OverallPackages[0])) -or (!(Test-Path $OverallPackages[1])))
    {
        Write-Warning "Failed to build overall FFmpegInteropX NuGet packages."
        Exit 1
    }
}

Write-Host
Read-Host -Prompt "Press 'Return' to publish packages to NuGet"

Write-Host
Write-Host "Pushing to NuGet..."
Write-Host

nuget push $LibPackages[0] -Source $NuGetPackageSource -SkipDuplicate
if ($lastexitcode -ne 0) { throw "Failed to publish package!" }
nuget push $LibPackages[1] -Source $NuGetPackageSource -SkipDuplicate
if ($lastexitcode -ne 0) { throw "Failed to publish package!" }
nuget push $FFmpegPackages[0] -Source $NuGetPackageSource -SkipDuplicate
if ($lastexitcode -ne 0) { throw "Failed to publish package!" }
nuget push $FFmpegPackages[1] -Source $NuGetPackageSource -SkipDuplicate
if ($lastexitcode -ne 0) { throw "Failed to publish package!" }
nuget push $OverallPackages[0] -Source $NuGetPackageSource -SkipDuplicate
if ($lastexitcode -ne 0) { throw "Failed to publish package!" }
nuget push $OverallPackages[1] -Source $NuGetPackageSource -SkipDuplicate
if ($lastexitcode -ne 0) { throw "Failed to publish package!" }

Write-Host
Write-Host 'Time elapsed'
Write-Host ('{0}' -f ((Get-Date) - $start))
Write-Host

if ($success)
{
    Write-Host 'Release succeeded!'

}
else
{
    Write-Warning 'Release failed!'
    Exit 1
}
