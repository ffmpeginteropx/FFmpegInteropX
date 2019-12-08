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

    if ($lastexitcode -ne 0) { Exit $lastexitcode }

    # 14.16.27023 => v141
    $platformToolSet = "v$($VcVersion.Major)$("$($VcVersion.Minor)"[0])"
    Write-Host "Platform Toolset: [$platformToolSet]"

    New-Item -ItemType Directory -Force $SolutionDir\Libs\Build\$Platform -OutVariable libs

    ('lib', 'licenses', 'include', 'build') | ForEach-Object {
        New-Item -ItemType Directory -Force $libs\$_
    }
    # Clean platform-specific build dir.
    Remove-Item -Force -Recurse $libs\build\*

    MSBuild.exe $SolutionDir\Libs\zlib\SMP\libzlib.vcxproj `
        /p:OutDir="$libs\build\" `
        /p:Configuration="${Configuration}WinRT" `
        /p:Platform=$Platform `
        /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
        /p:PlatformToolset=$platformToolSet

    if ($lastexitcode -ne 0) { Exit $lastexitcode }

    Get-ChildItem -Recurse -Include '*.h' $libs\build\libzlib\include | Copy-Item -Destination $libs\include\
    Copy-Item -Recurse $libs\build\libzlib\licenses\* -Destination $libs\licenses\
    Copy-Item $libs\build\libzlib\lib\$Platform\libzlib_winrt.lib $libs\lib\zlib.lib
    Copy-Item $libs\build\libzlib\lib\$Platform\libzlib_winrt.pdb $libs\lib\zlib.pdb

    MSBuild.exe $SolutionDir\Libs\bzip2\SMP\libbz2.vcxproj `
        /p:OutDir="$libs\build\" `
        /p:Configuration="${Configuration}WinRT" `
        /p:Platform=$Platform `
        /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
        /p:PlatformToolset=$platformToolSet

    if ($lastexitcode -ne 0) { Exit $lastexitcode }

    Get-ChildItem -Recurse -Include '*.h' $libs\build\libbz2\include | Copy-Item -Destination $libs\include\
    Copy-Item -Recurse $libs\build\libbz2\licenses\* -Destination $libs\licenses\
    Copy-Item $libs\build\libbz2\lib\$Platform\libbz2_winrt.lib $libs\lib\bz2.lib
    Copy-Item $libs\build\libbz2\lib\$Platform\libbz2_winrt.pdb $libs\lib\bz2.pdb

    MSBuild.exe $SolutionDir\Libs\libiconv\SMP\libiconv.vcxproj `
        /p:OutDir="$libs\build\" `
        /p:Configuration="${Configuration}WinRT" `
        /p:Platform=$Platform `
        /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
        /p:PlatformToolset=$platformToolSet
    
    if ($lastexitcode -ne 0) { Exit $lastexitcode }

    Get-ChildItem -Recurse -Include '*.h' $libs\build\libiconv\include | Copy-Item -Destination $libs\include\
    Copy-Item -Recurse $libs\build\libiconv\licenses\* -Destination $libs\licenses\
    Copy-Item $libs\build\libiconv\lib\$Platform\libiconv_winrt.lib $libs\lib\iconv.lib
    Copy-Item $libs\build\libiconv\lib\$Platform\libiconv_winrt.pdb $libs\lib\iconv.pdb

    $env:LIB += ";${libs}\lib"
    $env:INCLUDE += ";${libs}\include"

    # Export full current PATH from environment into MSYS2
    $env:MSYS2_PATH_TYPE = 'inherit'

    # Build ffmpeg - disable strict error handling since ffmpeg writes to error out
    $ErrorActionPreference = "Continue"
    & $Msys2Bin --login -x $SolutionDir\FFmpegConfig.sh Win10 $Platform
    $ErrorActionPreference = "Stop"

    if ($lastexitcode -ne 0) { Exit $lastexitcode }

    # Copy PDBs to built binaries dir
    Get-ChildItem -Recurse -Include '*.pdb' $SolutionDir\ffmpeg\Output\Windows10\$Platform | `
        Copy-Item -Destination $SolutionDir\ffmpeg\Build\Windows10\$Platform\bin\
}

# Stop on all PowerShell command errors
$ErrorActionPreference = "Stop"

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