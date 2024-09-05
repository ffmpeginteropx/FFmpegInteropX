param(

    [ValidateSet('x86', 'x64', 'ARM', 'ARM64')]
    [string[]] $Platforms = ('x86', 'x64', 'ARM64'),

    <#
        Example values:
        14.1
        14.2
        14.16
        14.16.27023
        14.23.27820

        Note. The PlatformToolset will be inferred from this value ('v141', 'v142'...)
    #>
    [version] $VcVersion = '14.3',

    <#
        Example values:
        8.1
        10.0.15063.0
        10.0.17763.0
        10.0.18362.0
    #>
    [version] $WindowsTargetPlatformVersion = '10.0.22000.0',

    [ValidateSet('UWP', 'Desktop')]
    [string] $WindowsTarget = 'UWP',

    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Release',
    
    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    # Set the search criteria for VSWHERE.EXE.
    [string[]] $VsWhereCriteria = '-latest',

    [switch] $ClearBuildFolders,

    [switch] $AllowParallelBuilds,

    # If a version string is specified, a NuGet package will be created.
    [string] $NugetPackageVersion = $null,

    # The version number to set in the dll file
    [version] $LibraryVersionNumber = $null,

    # FFmpegInteropX NuGet settings
    [string] $FFmpegInteropXUrl = 'https://github.com/ffmpeginteropx/FFmpegInteropX.git',

    [string] $FFmpegInteropXBranch = $(git branch --show-current),
    
    [string] $FFmpegInteropXCommit = $(git --git-dir Libs/ffmpeg/.git rev-parse HEAD)
)

function Build-Platform {
    param (
        [System.IO.DirectoryInfo] $SolutionDir,
        [string] $Platform,
        [string] $PlatformToolset,
        [string] $VsLatestPath,
        [version] $LibraryVersionNumber
    )

    $PSBoundParameters | Out-String

    Write-Host
    Write-Host "Building FFmpegInteropX for Windows 10 ${Platform}..."
    Write-Host

    # Load environment from VCVARS.
    Import-Module "$VsLatestPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

    Enter-VsDevShell `
        -VsInstallPath $VsLatestPath `
        -StartInPath "$PWD"

    if ($ClearBuildFolders) {
        # Clean platform-specific build and output dirs.
        Remove-Item -Force -Recurse -ErrorAction Ignore $SolutionDir\Intermediate\FFmpegInteropX\$Platform\${Configuration}_${WindowsTarget}
        Remove-Item -Force -Recurse -ErrorAction Ignore $SolutionDir\Output\FFmpegInteropX\$Platform\${Configuration}_${WindowsTarget}
    }

    MSBuild.exe $SolutionDir\Source\FFmpegInteropX.vcxproj `
        /restore `
        /p:Configuration=${Configuration}_${WindowsTarget} `
        /p:Platform=$Platform `
        /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
        /p:PlatformToolset=$PlatformToolset,
        /p:LibraryVersionNumber=$LibraryVersionNumber

    if ($lastexitcode -ne 0) { throw "Failed to build library FFmpegInteropX.vcxproj." }

    if ($Platform -eq "x64" -and $WindowsTarget -ne "UWP")
    {
        MSBuild.exe $SolutionDir\FFmpegInteropX.sln `
            /restore `
            /t:FFmpegInteropX_DotNet `
            /p:Configuration=${Configuration}_${WindowsTarget} `
            /p:Platform=$Platform `
            /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
            /p:TargetFramework="net6.0-windows$WindowsTargetPlatformVersion" `
            /p:AssemblyVersion=$LibraryVersionNumber `
            /p:FileVersion=$LibraryVersionNumber

        if ($lastexitcode -ne 0) { throw "Failed to build library FFmpegInteropX.DotNet.csproj." }
    }

}

Write-Host
Write-Host "Building FFmpegInteropX..."
Write-Host

# Stop on all PowerShell command errors
$ErrorActionPreference = "Stop"

[System.IO.DirectoryInfo] $vsLatestPath = `
    & "$VSInstallerFolder\vswhere.exe" `
    $VsWhereCriteria `
    -property installationPath `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64

Write-Host "Visual Studio Installation folder: [$vsLatestPath]"

# 14.16.27023 => v141
$platformToolSet = "v$($VcVersion.Major)$("$($VcVersion.Minor)"[0])"
Write-Host "Platform Toolset: [$platformToolSet]"

# Export full current PATH from environment into MSYS2
$env:MSYS2_PATH_TYPE = 'inherit'

# Save orignal environment variables
$oldEnv = @{};
foreach ($item in Get-ChildItem env:)
{
    $oldEnv.Add($item.Name, $item.Value);
}

$start = Get-Date
$success = 1

# Restore nuget packets for solution
nuget.exe restore ${PSScriptRoot}\FFmpegInteropX.sln

if ($lastexitcode -ne 0) { throw "Failed to restore NuGet packages." }

if ($NugetPackageVersion -and !$LibraryVersionNumber) {
    $versionPart = ($NugetPackageVersion -Split '-')[0];
    $LibraryVersionNumber = [Version]($versionPart + ".0");
}

if (!$LibraryVersionNumber)
{
    $LibraryVersionNumber = "1.0.0.0";
}

Write-Host "LibraryVersionNumber: $LibraryVersionNumber"

if ($AllowParallelBuilds -and $Platforms.Count -gt 1)
{
    $processes = @{}

    $addparams = ""
    if ($ClearBuildFolders)
    {
        $addparams += " -ClearBuildFolders"
    }

    foreach ($platform in $Platforms) {
        # WinUI does not support ARM
        if ($WindowsTarget -eq "Desktop" -and $platform -eq "ARM")
        {
            continue;
        }

        $proc = Start-Process -PassThru powershell "-File .\Build-FFmpegInteropX.ps1 -Platforms $platform -VcVersion $VcVersion -WindowsTarget $WindowsTarget -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion -Configuration $Configuration -VSInstallerFolder ""$VSInstallerFolder"" -VsWhereCriteria ""$VsWhereCriteria"" -FFmpegInteropXUrl ""$FFmpegInteropXUrl"" -FFmpegInteropXBranch ""FFmpegInteropXBranch"" -FFmpegInteropXCommit ""$FFmpegInteropXCommit"" -LibraryVersionNumber $LibraryVersionNumber $addparams"
        $processes[$platform] = $proc
    }

    foreach ($platform in $Platforms) {
        # WinUI does not support ARM
        if ($WindowsTarget -eq "Desktop" -and $platform -eq "ARM")
        {
            continue;
        }

        $processes[$platform].WaitForExit();
        $result = $processes[$platform].ExitCode;
        if ($result -eq 0)
        {
            Write-Host "Build for $platform succeeded!"
        }
        else
        {
            Write-Host "Build for $platform failed with ErrorCode: $result"
            $success = 0
        }
    }
}
else
{
    foreach ($platform in $Platforms) {

        # WinUI does not support ARM
        if ($WindowsTarget -eq "Desktop" -and $platform -eq "ARM")
        {
            continue;
        }

        try
        {
            Build-Platform `
                -SolutionDir "${PSScriptRoot}\" `
                -Platform $platform `
                -PlatformToolset $platformToolSet `
                -VsLatestPath $vsLatestPath `
                -LibraryVersionNumber $LibraryVersionNumber
        }
        catch
        {
            Write-Warning "Error occured: $PSItem"
            $success = 0
            Break
        }
        finally
        {
            # Restore orignal environment variables
            foreach ($item in $oldEnv.GetEnumerator())
            {
                Set-Item -Path env:"$($item.Name)" -Value $item.Value
            }
            foreach ($item in Get-ChildItem env:)
            {
                if (!$oldEnv.ContainsKey($item.Name))
                {
                     Remove-Item -Path env:"$($item.Name)"
                }
            }
        }
    }
}

if ($success -and $NugetPackageVersion)
{
    nuget pack .\Build\FFmpegInteropX.$WindowsTarget.Lib.nuspec `
        -Properties "id=FFmpegInteropX.$WindowsTarget.Lib;repositoryUrl=$FFmpegInteropXUrl;repositoryBranch=$FFmpegInteropXBranch;repositoryCommit=$FFmpegInteropXCommit;winsdk=$WindowsTargetPlatformVersion;NoWarn=NU5128" `
        -Version $NugetPackageVersion `
        -Symbols -SymbolPackageFormat symbols.nupkg `
        -OutputDirectory "Output\NuGet"
}

Write-Host
Write-Host 'Time elapsed'
Write-Host ('{0}' -f ((Get-Date) - $start))
Write-Host

if ($success)
{
    Write-Host 'Build succeeded!'

}
else
{
    Write-Error 'Build failed!'
    Exit 1
}
