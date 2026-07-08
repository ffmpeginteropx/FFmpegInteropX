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

    [switch] $DisableParallelBuilds,
    
    [switch] $SkipBuildLibs,
    
    [switch] $SkipConfigureFFmpeg

)

$PushPackages = [System.Collections.Generic.List[string]]::new()

function Test-Package-Local {
    param (
        [Parameter(Mandatory=$true, Position=0)]
        [string] $PackageName,
        [Parameter(Mandatory=$true, Position=1)]
        [string] $PackageVersion
    )

    return Test-Path(".\Output\NuGet\$PackageName.$PackageVersion.nupkg")
}

function Test-Package-Online {
    param (
        [Parameter(Mandatory=$true, Position=0)]
        [string] $PackageName,
        [Parameter(Mandatory=$true, Position=1)]
        [string] $PackageVersion
    )

    $res = dotnet package search $PackageName --exact-match --prerelease --format json --source $NuGetPackageSource | ConvertFrom-Json
    $package = $res.searchResult[0].packages | Where-Object { $_.version -eq $PackageVersion }

    if ($package) {
        return $true
    } else {
        return $false
    }
}

function Test-Package {
    param (
        [Parameter(Mandatory=$true, Position=0)]
        [string] $PackagePostfix,
        [Parameter(Mandatory=$true, Position=1)]
        [string] $PackageVersion,
        [Parameter(Position=2)]
        [string] $PackagePrefix = "FFmpegInteropX",
        [Parameter(Position=3)]
        [array] $WindowsTargets = @("Desktop", "UWP")
    )

    $NeedPush = @()
    $NeedBuild = $false

    foreach ($target in $WindowsTargets) {
        $packageName = "$PackagePrefix.$target.$PackagePostfix"

        Write-Host
        Write-Host "Checking for package: $packageName $PackageVersion..."

        if (Test-Package-Online -PackageName $packageName -PackageVersion $PackageVersion) {
            Write-Host "Online package found: $packageName $PackageVersion"
        } elseif (Test-Package-Local -PackageName $packageName -PackageVersion $PackageVersion) {
            Write-Host "Local package found: $packageName $PackageVersion"
            $NeedPush += $packageName
        } else {
            Write-Host "Package needs to be built: $packageName $PackageVersion"
            $NeedPush += $packageName
            $NeedBuild = $true
        }
    }

    foreach ($package in $NeedPush) {
        Write-Host "Package to push: $package.$PackageVersion"
        $PushPackages.Add(".\Output\NuGet\$package.$PackageVersion.nupkg")
    }

    return $NeedBuild
}

function Invoke() {
    # A handy way to run a command, and automatically throw an error if the
    # exit code is non-zero.

    if ($args.Count -eq 0) {
        throw "Must supply some arguments."
    }

    $command = $args[0]
    $commandArgs = @()
    if ($args.Count -gt 1) {
        $commandArgs = $args[1..($args.Count - 1)]
    }

    $old = $ErrorActionPreference
    $ErrorActionPreference = "Continue"

    & $command $commandArgs | Out-Default
    $result = $LASTEXITCODE

    $ErrorActionPreference = $old

    if ($result -ne 0) {
        throw "$command $commandArgs exited with code $result.`r`nOriginal command line:`r`n$command $commandArgs"
    }
}

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

$start = Get-Date
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

if (Test-Package "Lib" $LibPackageVersion)
{
    Write-Host
    Write-Host "Building FFmpegInteropX Lib..."
    Write-Host

    .\Build-FFmpegInteropX.ps1 `
        -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion `
        -WindowsTargetPlatformMinVersion $WindowsTargetPlatformMinVersion `
        -VcVersion $VcVersion `
        -LibraryVersionNumber $LibraryVersionNumber `
        -NugetPackageVersion $LibPackageVersion `
        -ClearBuildFolders:$ClearBuildFolders `
        -DisableParallelBuilds:$DisableParallelBuilds
}

if (Test-Package "FFmpeg" $FFmpegPackageVersion)
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
        -DisableParallelBuilds:$DisableParallelBuilds `
        -SkipConfigureFFmpeg:$SkipConfigureFFmpeg `
        -SkipBuildLibs:$SkipBuildLibs
}

$OverallPackages = @(
    ".\Output\NuGet\FFmpegInteropX.$OverallPackageVersion.nupkg",
    ".\Output\NuGet\FFmpegInteropX.UWP.$OverallPackageVersion.nupkg"
)

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
}

$PushPackages.Add($OverallPackages[0])
$PushPackages.Add($OverallPackages[1])

Write-Host
Write-Host "Packages to push:"

foreach ($package in $PushPackages)
{
    Write-Host " - $package"
}

Write-Host
Read-Host -Prompt "Press 'Return' to publish packages to NuGet"

Write-Host
Write-Host "Pushing to NuGet..."
Write-Host

foreach ($package in $PushPackages)
{
    Write-Host "Pushing $package..."
    Invoke nuget push $package -Source $NuGetPackageSource -SkipDuplicate
}

Write-Host
Write-Host 'Time elapsed'
Write-Host ('{0}' -f ((Get-Date) - $start))
Write-Host
Write-Host 'Release succeeded!'
