param(

    [ValidateSet('x86', 'x64', 'ARM', 'ARM64')]
    [string[]] $Platforms = ('x86', 'x64', 'ARM', 'ARM64'),

    [version] $VcVersion = '14.1',

    <#
        Example values:
        8.1
        10.0.15063.0
        10.0.17763.0
        10.0.18362.0
    #>
    [version] $WindowsTargetPlatformVersion = '10.0.15063.0',

    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Release',

    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    [System.IO.FileInfo] $Msys2Bin = 'C:\msys64\usr\bin\bash.exe'
)

[System.IO.DirectoryInfo] $vsLatestPath = & "$VSInstallerFolder\vswhere.exe" -latest -property installationPath -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64

Write-Host "Visual Studio Installation folder: [$vsLatestPath]"

# Export full current PATH from environment into MSYS2
$env:MSYS2_PATH_TYPE = 'inherit'

if (! (Test-Path $PSScriptRoot\ffmpeg\configure)) {
    Write-Error 'configure is not found in ffmpeg folder. Ensure this folder is populated with ffmpeg snapshot'
    Exit
}

# 14.16.27023 => v141
$platformToolSet = "v$($VcVersion.Major)$("$VcVersion.Minor"[0])"

# [$env:PROCESSOR_ARCHITECTURE][$platform]
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

$start = Get-Date

foreach ($platform in $Platforms) {

    Write-Host "Building FFmpeg for Windows 10 apps ${platform}..."

    # Load environment from VCVARS.
    $vcvarsArch = $vcvarsArchs[$env:PROCESSOR_ARCHITECTURE][$platform]
    CMD /c "`"$vsLatestDir\VC\Auxiliary\Build\vcvarsall.bat`" $vcvarsArch uwp $WindowsTargetPlatformVersion && SET" | . {
        PROCESS {
            if ($_ -match '^([^=]+)=(.*)') {
                if ($matches[1] -notin 'HOME') {
                    Set-Item -Path "Env:\$($Matches[1])" -Value $Matches[2]
                }
            }
        }
    }

    New-Item -ItemType Directory -Force $PSScriptRoot\Libs\Build\$platform -OutVariable libs

    ('lib', 'licenses', 'include', 'build') | ForEach-Object {
        New-Item -ItemType Directory -Force $libs\$_
    }
    # Clean platform-specific build dir.
    Remove-Item -Force -Recurse $libs\build\*

    MSBuild.exe .\Libs\zlib\SMP\libzlib.vcxproj `
        /p:OutDir="$libs\build\" `
        /p:Configuration="${Configuration}WinRT" `
        /p:Platform=$platform `
        /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
        /p:PlatformToolset=$platformToolSet

    Get-ChildItem -Recurse -Include '*.h' $libs\build\libzlib\include | Copy-Item -Destination $libs\include\
    Copy-Item -Recurse $libs\build\libzlib\licenses\* -Destination $libs\licenses\
    Copy-Item $libs\build\libzlib\lib\$platform\libzlib_winrt.lib $libs\lib\zlib.lib
    Copy-Item $libs\build\libzlib\lib\$platform\libzlib_winrt.pdb $libs\lib\zlib.pdb

    MSBuild.exe .\Libs\bzip2\SMP\libbz2.vcxproj `
        /p:OutDir="$libs\build\" `
        /p:Configuration="${Configuration}WinRT" `
        /p:Platform=$platform `
        /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
        /p:PlatformToolset=$platformToolSet

    Get-ChildItem -Recurse -Include '*.h' $libs\build\libbz2\include | Copy-Item -Destination $libs\include\
    Copy-Item -Recurse $libs\build\libbz2\licenses\* -Destination $libs\licenses\
    Copy-Item $libs\build\libbz2\lib\$platform\libbz2_winrt.lib $libs\lib\bz2.lib
    Copy-Item $libs\build\libbz2\lib\$platform\libbz2_winrt.pdb $libs\lib\bz2.pdb

    MSBuild.exe .\Libs\bzip2\SMP\libiconv.vcxproj `
        /p:OutDir="$libs\build\" `
        /p:Configuration="${Configuration}WinRT" `
        /p:Platform=$platform `
        /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
        /p:PlatformToolset=$platformToolSet

    Get-ChildItem -Recurse -Include '*.h' $libs\build\libiconv\include | Copy-Item -Destination $libs\include\
    Copy-Item -Recurse $libs\build\libiconv\licenses\* -Destination $libs\licenses\
    Copy-Item $libs\build\libiconv\lib\$platform\libiconv_winrt.lib $libs\lib\iconv.lib
    Copy-Item $libs\build\libiconv\lib\$platform\libiconv_winrt.pdb $libs\lib\iconv.pdb

    $env:LIB += ";${libs}\lib"
    $env:INCLUDE += ";${libs}\include"

    # Build ffmpeg
    & $Msys2Bin --login -x $PSScriptRoot\FFmpegConfig.sh Win10 $platform

    # Copy PDBs to built binaries dir
    Get-ChildItem -Recurse -Include '*.pdb' .\ffmpeg\Output\Windows10\$platform | Copy-Item -Destination .\ffmpeg\Build\Windows10\$platform\bin\
}

Write-Host 'Time elapsed' ' {0}' -f ((Get-Date) - $start)