param(

    [ValidateSet('x86', 'x64', 'ARM', 'ARM64')]
    [string[]] $Platforms = ('x86', 'x64', 'ARM', 'ARM64'),

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
    [version] $WindowsTargetPlatformVersion = '10.0.22621.0',

    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Release',

    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    # Set the search criteria for VSWHERE.EXE.
    [string[]] $VsWhereCriteria = '-latest',

    [switch] $ClearBuildFolders,

    # If a version string is specified, a NuGet package will be created.
    [string] $NugetPackageVersion = $null,

    # FFmpegInteropX NuGet settings
    [string] $FFmpegInteropXUrl = 'https://github.com/ffmpeginteropx/FFmpegInteropX.git',

    [string] $FFmpegInteropXBranch = $(git branch --show-current),

    [string] $FFmpegInteropXCommit = $(git --git-dir Libs/ffmpeg/.git rev-parse HEAD)
)

function Build-Platform {
    param (
        [System.IO.DirectoryInfo] $SolutionDir,
        [string] $Platform,
        [version] $WindowsTargetPlatformVersion,
        [version] $VcVersion,
        [string] $PlatformToolset,
        [string] $VsLatestPath
    )

    $PSBoundParameters | Out-String

    $hostArch = ( 'x86', 'x64' )[ [System.Environment]::Is64BitOperatingSystem ]
    $targetArch = $Platform.ToLower()

    Write-Host
    Write-Host "Building FFmpegInteropX for Windows 10 ${Platform}..."
    Write-Host

    # Load environment from VCVARS.
    Import-Module "$VsLatestPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

    Enter-VsDevShell `
        -VsInstallPath $VsLatestPath `
        -StartInPath "$PWD" `
        -DevCmdArguments "-arch=$targetArch -host_arch=$hostArch -winsdk=$WindowsTargetPlatformVersion -vcvars_ver=$VcVersion -app_platform=UWP"

    if ($ClearBuildFolders) {
        # Clean platform-specific build and output dirs.
        Remove-Item -Force -Recurse -ErrorAction Ignore $SolutionDir\Intermediate\FFmpegInteropX\$Platform\*
        Remove-Item -Force -Recurse -ErrorAction Ignore $SolutionDir\Output\FFmpegInteropX\$Platform\*
    }

	if ($targetArch -eq "x86")
	{
	    MSBuild.exe $SolutionDir\Source\FFmpegInteropX.DotNet.csproj `
			/restore `
			/p:Configuration=$Configuration `
			/p:WINUI=1 `
			/p:Platform=AnyCPU `
			/p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
			/p:useenv=true

		if ($lastexitcode -ne 0) { throw "Failed to build library FFmpegInteropX.DotNet.csproj." }
	}

    MSBuild.exe $SolutionDir\Source\FFmpegInteropX.vcxproj `
        /restore `
        /p:Configuration=$Configuration `
        /p:Platform=$Platform `
        /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
        /p:PlatformToolset=$PlatformToolset `
        /p:useenv=true

    if ($lastexitcode -ne 0) { throw "Failed to build library FFmpegInteropX.vcxproj." }
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

foreach ($platform in $Platforms) {

    try
    {
        Build-Platform `
            -SolutionDir "${PSScriptRoot}\" `
            -Platform $platform `
            -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion `
            -VcVersion $VcVersion `
            -PlatformToolset $platformToolSet `
            -VsLatestPath $vsLatestPath
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

if ($success -and $NugetPackageVersion)
{
    nuget pack .\Build\FFmpegInteropX.nuspec `
        -Properties "id=FFmpegInteropX;repositoryUrl=$FFmpegInteropXUrl;repositoryBranch=$FFmpegInteropXBranch;repositoryCommit=$FFmpegInteropXCommit;NoWarn=NU5128" `
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
