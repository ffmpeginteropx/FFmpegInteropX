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
    [version] $VcVersion = '14.1',

    <#
        Example values:
        8.1
        10.0.15063.0
        10.0.17763.0
        10.0.18362.0
    #>
    [version] $WindowsTargetPlatformVersion = '10.0.18362.0',

    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Release',

    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    # Set the search criteria for VSWHERE.EXE.
    [string[]] $VsWhereCriteria = '-latest',

    [System.IO.FileInfo] $Msys2Bin = 'C:\msys64\usr\bin\bash.exe'
)

function Build-Platform {
    param (
        [System.IO.DirectoryInfo] $SolutionDir,
        [string] $Platform,
        [string] $Configuration,
        [version] $WindowsTargetPlatformVersion,
        [version] $VcVersion,
        [string] $VsLatestPath,
        [string] $Msys2Bin = 'C:\msys64\usr\bin\bash.exe'
    )

    $PSBoundParameters | Out-String

    $vcvarsArchs = @{
        'x86' = @{
            'x86'   = 'x86'
            'x64'   = 'x86_amd64'
            'ARM'   = 'x86_arm'
            'ARM64' = 'x86_arm64'
        }

        'AMD64' = @{
            'x86'   = 'amd64_x86'
            'x64'   = 'amd64'
            'ARM'   = 'amd64_arm'
            'ARM64' = 'amd64_arm64'
        }
    }

    Write-Host "Building FFmpeg for Windows 10 apps ${Platform}..."

    # Load environment from VCVARS.
    $vcvarsArch = $vcvarsArchs[$env:PROCESSOR_ARCHITECTURE][$Platform]

    CMD /c "`"$VsLatestPath\VC\Auxiliary\Build\vcvarsall.bat`" $vcvarsArch uwp $WindowsTargetPlatformVersion -vcvars_ver=$VcVersion && SET" | . {
        PROCESS {
            if ($_ -match '^([^=]+)=(.*)') {
                if ($Matches[1] -notin 'HOME') {
                    Set-Item -Path "Env:\$($Matches[1])" -Value $Matches[2]
                }
            }
        }
    }

    # 14.16.27023 => v141
    $platformToolSet = "v$($VcVersion.Major)$("$VcVersion.Minor"[0])"

    $baseIntDir = "${SolutionDir}Build\${Platform}\${Configuration}"
    $baseOutDir = "${SolutionDir}Target\${Platform}\${Configuration}"

    try {
        MSBuild.exe $SolutionDir\Libs\zlib\SMP\libzlib.vcxproj `
            /p:TargetName='zlib' `
            /p:IntDir="$baseIntDir\libzlib\" `
            /p:OutDir="$baseOutDir\" `
            /p:Platform=$Platform `
            /p:Configuration=$Configuration `
            /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
            /p:PlatformToolset=$platformToolSet
    }
    catch {
        Exit
    }

    try {
        MSBuild.exe $SolutionDir\Libs\bzip2\SMP\libbz2.vcxproj `
            /p:TargetName='bz2' `
            /p:IntDir="$baseIntDir\libbz2\" `
            /p:OutDir="$baseOutDir\" `
            /p:Platform=$Platform `
            /p:Configuration=$Configuration `
            /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
            /p:PlatformToolset=$platformToolSet
    }
    catch {
        Exit
    }

    try {
        MSBuild.exe $SolutionDir\Libs\libiconv\SMP\libiconv.vcxproj `
            /p:TargetName='iconv' `
            /p:IntDir="$baseIntDir\libiconv\" `
            /p:OutDir="$baseOutDir\" `
            /p:Platform=$Platform `
            /p:Configuration=$Configuration `
            /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
            /p:PlatformToolset=$platformToolSet
    }
    catch {
        Exit
    }

    $env:LIB += ";$baseOutDir\lib\${Platform}"
    $env:INCLUDE += ";$baseOutDir\include"

    # Export full current PATH from environment into MSYS2
    $env:MSYS2_PATH_TYPE = 'inherit'

    # Build ffmpeg
    & $Msys2Bin --login -x $SolutionDir\FFmpegConfig.sh Win10 $Platform

    # Copy PDBs to built binaries dir
    Get-ChildItem -Recurse -Include '*.pdb' $baseIntDir\ffmpeg-Win10 | `
        Copy-Item -Destination $baseOutDir\ffmpeg-Win10\bin\
}

if (! (Test-Path $PSScriptRoot\ffmpeg\configure)) {
    Write-Error 'configure is not found in ffmpeg folder. Ensure this folder is populated with ffmpeg snapshot'
    Exit
}

if (!(Test-Path $Msys2Bin)) {

    $msysFound = $false
    @( 'C:\msys64', 'C:\msys' ) | ForEach-Object {
        if (Test-Path $_) {
            $Msys2Bin = "${_}\usr\bin\bash.exe"
            $msysFound = $true

            break
        }
    }

    # Search for MSYS locations
    if (! $msysFound) {
        Write-Error "MSYS2 not found."
        Exit;
    }
}

[System.IO.DirectoryInfo] $vsLatestPath = `
    & "$VSInstallerFolder\vswhere.exe" `
    $VsWhereCriteria `
    -property installationPath `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64

Write-Host "Visual Studio Installation folder: [$vsLatestPath]"

$start = Get-Date

foreach ($platform in $Platforms) {
    Build-Platform `
        -SolutionDir "${PSScriptRoot}\" `
        -Platform $platform `
        -Configuration 'Release' `
        -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion `
        -VcVersion $VcVersion `
        -VsLatestPath $vsLatestPath `
        -Msys2Bin $Msys2Bin
}

Write-Host 'Time elapsed'
Write-Host (' {0}' -f ((Get-Date) - $start))
