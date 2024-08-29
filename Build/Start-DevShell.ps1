param(

    [ValidateSet('x86', 'x64', 'ARM', 'ARM64')]
    [string] $Platform = 'x86',

    <#
        Example values:
        14.1
        14.2
        14.16
        14.16.27023
        14.23.27820

        Note. The PlatformToolset will be inferred from this value ('v141', 'v142'...)
    #>
    [version] $VcVersion = '14.2',

    [ValidateSet('UWP', 'Desktop')]
    [string] $WindowsTarget = 'UWP',

    <#
        Example values:
        8.1
        10.0.15063.0
        10.0.17763.0
        10.0.18362.0
    #>
    [version] $WindowsTargetPlatformVersion = '10.0.22621.0',

    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    # Set the search criteria for VSWHERE.EXE.
    [string[]] $VsWhereCriteria = '-latest',

    [switch] $InProcess
)

if ($InProcess)
{
    Write-Host
    Write-Host "Starting DevShell $Platform $WindowsTarget $WindowsTargetPlatformVersion $VcVersion"
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

    $hostArch = ( 'x86', 'x64' )[ [System.Environment]::Is64BitOperatingSystem ]
    $targetArch = $Platform.ToLower()

    Write-Host
    Write-Host "Building FFmpeg for Windows 10 ($WindowsTarget) ${Platform}..."
    Write-Host

    # Load environment from VCVARS.
    Import-Module "$VsLatestPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

    Enter-VsDevShell `
        -VsInstallPath $VsLatestPath `
        -StartInPath "$PWD" `
        -DevCmdArguments "-arch=$targetArch -host_arch=$hostArch -winsdk=$WindowsTargetPlatformVersion -vcvars_ver=$VcVersion -app_platform=$WindowsTarget"
}
else
{
    powershell -NoExit -Command "$PSCommandPath -Platform $Platform -VcVersion $VcVersion -WindowsTarget $WindowsTarget -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion -VSInstallerFolder '$VSInstallerFolder' -VsWhereCriteria $VsWhereCriteria -InProcess"
}
