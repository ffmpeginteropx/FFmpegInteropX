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

    Write-Host "Checking online for $PackageName $PackageVersion..."

    Write-Host "dotnet package search $PackageName --exact-match --prerelease --format json --source $NuGetPackageSource | ConvertFrom-Json"
    $res = dotnet package search $PackageName --exact-match --prerelease --format json --source $NuGetPackageSource | ConvertFrom-Json
    $package = $res.searchResult[0].packages | Where-Object { $_.version -eq $PackageVersion }

    if ($package) {
        Write-Host "Package found!"
        return $true
    } else {
        Write-Host "Package not found!"
        return $false
    }
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

$PushPackages = @()

if ((!(Test-Package-Local -PackageName "FFmpegInteropX.Desktop.Lib" -PackageVersion $LibPackageVersion)) -or (!(Test-Package-Local -PackageName "FFmpegInteropX.UWP.Lib" -PackageVersion $LibPackageVersion)))
{
    if ((!(Test-Package-Online -PackageName "FFmpegInteropX.Desktop.Lib" -PackageVersion $LibPackageVersion)) -or (!(Test-Package-Online -PackageName "FFmpegInteropX.UWP.Lib" -PackageVersion $LibPackageVersion)))
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
            -AllowParallelBuilds:$AllowParallelBuilds

        $PushPackages += $LibPackages[0]
        $PushPackages += $LibPackages[1]
    }
}
else
{
    $PushPackages += $LibPackages[0]
    $PushPackages += $LibPackages[1]
}

if ((!(Test-Package-Local -PackageName "FFmpegInteropX.Desktop.FFmpeg" -PackageVersion $FFmpegPackageVersion)) -or (!(Test-Package-Local -PackageName "FFmpegInteropX.UWP.FFmpeg" -PackageVersion $FFmpegPackageVersion)))
{
    if ((!(Test-Package-Online -PackageName "FFmpegInteropX.Desktop.FFmpeg" -PackageVersion $FFmpegPackageVersion)) -or (!(Test-Package-Online -PackageName "FFmpegInteropX.UWP.FFmpeg" -PackageVersion $FFmpegPackageVersion)))
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
            -AllowParallelBuilds:$AllowParallelBuilds `
            -SkipConfigureFFmpeg:$SkipConfigureFFmpeg `
            -SkipBuildLibs:$SkipBuildLibs

        $PushPackages += $FFmpegPackages[0]
        $PushPackages += $FFmpegPackages[1]
   }
}
else
{
    $PushPackages += $FFmpegPackages[0]
    $PushPackages += $FFmpegPackages[1]
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
}

$PushPackages += $OverallPackages[0]
$PushPackages += $OverallPackages[1]

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
