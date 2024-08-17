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

    [ValidateSet('UWP', 'Desktop')]
    [string] $WindowsTarget = 'UWP',

    <#
        Example values:
        8.1
        10.0.15063.0
        10.0.17763.0
        10.0.18362.0
    #>
    [version] $WindowsTargetPlatformVersion = '10.0.22000.0',

    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Release',

    # shared: create shared dll - static: create lib for static linking
    [ValidateSet('shared', 'static')]
    [string] $SharedOrStatic = 'shared',

    [ValidateSet('enable', 'disable')]
    [string] $Gpl = 'disable',

    [ValidateSet('enable', 'disable')]
    [string] $Encoders = 'disable',

    [ValidateSet('enable', 'disable')]
    [string] $Devices = 'disable',

    [ValidateSet('enable', 'disable')]
    [string] $Programs = 'disable',

    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    # Set the search criteria for VSWHERE.EXE.
    [string[]] $VsWhereCriteria = '-latest',

    [System.IO.FileInfo] $BashExe = 'C:\msys64\usr\bin\bash.exe',

    [switch] $ClearBuildFolders,

    # If a version string is specified, a NuGet package will be created.
    [string] $NugetPackageVersion = $null,

    # FFmpeg NuGet settings
    [string] $FFmpegUrl = 'https://git.ffmpeg.org/ffmpeg.git',

    [string] $FFmpegCommit = $(git --git-dir $PSScriptRoot/Libs/ffmpeg/.git rev-parse HEAD),

    [switch] $AllowParallelBuilds,

    [switch] $SkipBuildPkgConfigFake,

    [switch] $SkipBuildLibs,

    [switch] $SkipConfigureFFmpeg

)

function Build-Platform {
    param (
        [System.IO.DirectoryInfo] $SolutionDir,
        [string] $Platform,
        [string] $Configuration,
        [version] $WindowsTargetPlatformVersion,
        [version] $VcVersion,
        [string] $PlatformToolset,
        [string] $VsLatestPath,
        [string] $BashExe = 'C:\msys64\usr\bin\bash.exe',
        [string] $LogFileName,
        [bool] $SkipBuildPkgConfigFake,
        [bool] $SkipBuildLibs,
        [bool] $SkipConfigureFFmpeg
    )

    New-Item -ItemType Directory -Force $SolutionDir\Intermediate\FFmpeg$WindowsTarget | Out-Null
    Start-Transcript -Path $LogFileName

    $PSBoundParameters | Out-String

    $hostArch = ( 'x86', 'x64' )[ [System.Environment]::Is64BitOperatingSystem ]
    $targetArch = $Platform.ToLower()

    # Build x86 and x64 with x86 toolchain
    if (($targetArch -eq 'x86') -or ($targetArch -eq 'x64'))
    {
        $hostArch = 'x86'
    }

    Write-Host
    Write-Host "Building FFmpeg for Windows 10 ($WindowsTarget) ${Platform} ${Gpl}-gpl ${Encoders}-encoders ${Devices}-devices ${Programs}-programs ..."
    Write-Host

    # Load environment from VCVARS.
    Import-Module "$VsLatestPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

    Enter-VsDevShell `
        -VsInstallPath $VsLatestPath `
        -StartInPath "$PWD" `
        -DevCmdArguments "-arch=$targetArch -host_arch=$hostArch -winsdk=$WindowsTargetPlatformVersion -vcvars_ver=$VcVersion -app_platform=$WindowsTarget"

    New-Item -ItemType Directory -Force $SolutionDir\Intermediate\FFmpeg$WindowsTarget\$Platform -OutVariable build | Out-Null
    New-Item -ItemType Directory -Force $SolutionDir\Output\FFmpeg$WindowsTarget\$Platform -OutVariable target | Out-Null

    $env:LIB += ";$build\lib"
    $env:INCLUDE += ";$build\include"
    $env:Path += ";$SolutionDir\Libs\gas-preprocessor"

    if (! $SkipBuildLibs)
    {
        # Build pkg-config fake
        if (! $SkipBuildPkgConfigFake) {
            invoke MSBuild.exe $SolutionDir\Libs\PkgConfigFake\PkgConfigFake.csproj `
                /p:OutputPath="$SolutionDir\Intermediate\" `
                /p:Configuration=$Configuration `
                /p:Platform=${Env:\PreferredToolArchitecture}

            if ($lastexitcode -ne 0) { throw "Failed to build PkgConfigFake." }
        }

        if ($ClearBuildFolders) {
            # Clean platform-specific build and output dirs.
            Remove-Item -Force -Recurse $build\*
            Remove-Item -Force -Recurse $target\*
        }

        ('lib', 'licenses', 'include') | ForEach-Object {
            New-Item -ItemType Directory -Force $build\$_ | Out-Null
            New-Item -ItemType Directory -Force $target\$_ | Out-Null
        }

        # library definitions: <FolderName>, <ProjectName>, <FFmpegTargetName>
        $libdefs = @(
            @('zlib', 'libzlib', 'zlib'),
            @('bzip2', 'libbz2', 'bz2'),
            @('libiconv', 'libiconv', 'iconv'),
            @('liblzma', 'liblzma', 'lzma'),
            @('libxml2', 'libxml2', 'libxml2')
            )

        # Build all libraries
        $libdefs | ForEach-Object {

            $folder = $_[0]
            $project = $_[1]

            Write-Host
            Write-Host "Building Library ${folder}..."
            Write-Host

            # Decide vcvars target string based on target platform
            if ($WindowsTarget -eq "UWP") {
                $configurationName = "${Configuration}WinRT"
                $targetName = "${project}_winrt"
            }
            else {
                $configurationName = ${Configuration}
                $targetName = ${project}
            }
            $intDir = "$build\int\$project\"
            $outDir = "$build\$project\"

            invoke MSBuild.exe $SolutionDir\Libs\$folder\SMP\$project.vcxproj `
                /p:OutDir=$outDir `
                /p:IntDir=$intDir `
                /p:Configuration=$configurationName `
                /p:Platform=$Platform `
                /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
                /p:PlatformToolset=$PlatformToolset `
                /p:ForceImportBeforeCppTargets=$SolutionDir\Libs\build-scripts\LibOverrides.props `
                /p:useenv=true

            Copy-Item $build\$project\include\* $build\include\ -Recurse -Force
            Copy-Item $build\$project\licenses\* $build\licenses\ -Recurse -Force
            Copy-Item $build\$project\lib\$Platform\$targetName.lib $build\lib\ -Force
            Copy-Item $build\$project\lib\$Platform\$targetName.pdb $build\lib\ -Force
        }

        # Rename all libraries to ffmpeg target names
        $libdefs | ForEach-Object {

            $project = $_[1];
            $ffmpegTarget = $_[2];
            $targetName = if ($WindowsTarget -eq "UWP") { "${project}_winrt" } else { $project }

            Move-Item $build\lib\$targetName.lib $build\lib\$ffmpegTarget.lib -Force
            Move-Item $build\lib\$targetName.pdb $build\lib\$ffmpegTarget.pdb -Force
        }

        # Fixup libxml2 includes for ffmpeg build
        Copy-Item $build\include\libxml2\libxml $build\include\ -Force -Recurse

        # Build openssl if not already exists
        if (!(Test-Path("$build\lib\ssl.lib")) -or !(Test-Path("$build\lib\crypto.lib"))) {

            Write-Host
            Write-Host "Building Library openssl..."
            Write-Host

            $opensslPlatforms = @{
                'x86'   = 'VC-WIN32'
                'x64'   = 'VC-WIN64A'
                'ARM'   = 'VC-WIN32-ARM'
                'ARM64' = 'VC-WIN64-ARM'
            }
            $opensslPlatform = $opensslPlatforms[$Platform]

            if ($WindowsTarget -eq "UWP") {
                $opensslPlatform = $opensslPlatform + "-UWP"
            }

            New-Item -ItemType Directory -Force $build\int\openssl -OutVariable ssldir | Out-Null

            $oldPath = $env:Path
            $env:Path += ";$SolutionDir\Tools\perl\perl\bin;C$SolutionDir\Tools\perl\c\bin;$SolutionDir\Tools\nasm"
            $oldDir = get-location
            set-location "$ssldir"

            try {
                invoke perl $SolutionDir\Libs\openssl\Configure $opensslPlatform --prefix=$build --openssldir=$build --with-zlib-include=$build\include --with-zlib-lib=$build\lib\zlib.lib no-tests no-secure-memory
                invoke nmake clean
                invoke nmake
                invoke nmake install_sw
            } finally {
                set-location $oldDir
                $env:Path = $oldPath
            }

            Copy-Item -Force $SolutionDir\Libs\openssl\license.txt $build\licenses\openssl.txt
            Copy-Item -Force $ssldir\libssl_static.lib $build\lib\ssl.lib
            Copy-Item -Force $ssldir\libcrypto_static.lib $build\lib\crypto.lib
            } else {
            Write-Host
            Write-Host "Openssl already exists in target build configuration. Skipping build."
            Write-Host
        }

        #Build dav1d
        Write-Host ""
        Write-Host "Building Library dav1d..."
        Write-Host ""
        invoke $BashExe --login -c "cd \$SolutionDir && Libs/build-scripts/build-dav1d.sh $WindowsTarget $Platform".Replace("\", "/").Replace(":", "")

        if ($Encoders -eq "enable") {

            $env:Path += ";$(Split-Path $BashExe)"

            #Build x265
            Write-Host ""
            Write-Host "Building Library x265..."
            Write-Host ""

            $cmakePlatforms = @{
                'x86'   = 'Win32'
                'x64'   = 'x64'
                'ARM'   = 'ARM'
                'ARM64' = 'ARM64'
            }
            $cmakePlatform = $cmakePlatforms[$Platform]

            New-Item -ItemType Directory -Force $build\int\x265

            invoke cmd.exe /C $SolutionDir\Libs\build-scripts\build-x265.bat $SolutionDir\Libs\x265\source $build\int\x265 $cmakePlatform $PlatformToolset

            Copy-Item $build\int\x265\x265-static.lib $build\lib\x265.lib -Force
            Copy-Item $build\int\x265\include\* $build\include\ -Force
            Copy-Item $SolutionDir\Libs\x265\COPYING $build\licenses\x265.txt -Force

            #Build x264
            Write-Host ""
            Write-Host "Building Library x264..."
            Write-Host ""

            $x264Archs = @{
                'x86'   = 'x86'
                'x64'   = 'x86_64'
                'ARM'   = 'arm'
                'ARM64' = 'aarch64'
            }
            $x264Arch = $x264Archs[$Platform]

            New-Item -ItemType Directory -Force $build\x264

            invoke $BashExe --login -c "cd \$build\x264 && CC=cl ..\..\..\..\Libs\x264\configure --host=${x264Arch}-mingw64 --prefix=\$build --disable-cli --enable-static && make -j8 -e CPPFLAGS=-Oy && make install".Replace("\", "/").Replace(":", "")

            #Build libvpx
            Write-Host
            Write-Host "Building Library libvpx..."
            Write-Host

            $vpxArchs = @{
                'x86'   = 'x86'
                'x64'   = 'x86_64'
                'ARM'   = 'armv7'
                'ARM64' = 'arm64'
            }
            $vpxPlatforms = @{
                'x86'   = 'win32'
                'x64'   = 'win64'
                'ARM'   = 'win32'
                'ARM64' = 'win64'
            }
            $vpxArch = $vpxArchs[$Platform]
            $vpxPlatform = $vpxPlatforms[$Platform]

            New-Item -ItemType Directory -Force $build\libvpx

            invoke $BashExe --login -c "unset tmp && unset temp && cd \$build\libvpx && ..\..\..\..\Libs\libvpx\configure --target=${vpxArch}-${vpxPlatform}-vs17 --prefix=\$build --enable-static --disable-thumb --disable-debug --disable-examples --disable-tools --disable-docs --disable-unit_tests && make -j8 -e CPPFLAGS=-Oy && make install".Replace("\", "/").Replace(":", "")

            Move-Item $build\lib\$cmakePlatform\vpxmd.lib $build\lib\vpx.lib -Force
            Remove-Item $build\lib\$cmakePlatform -Force -Recurse
        }
    }

    # Build ffmpeg
    Write-Host
    Write-Host "Building FFmpeg..."
    Write-Host

    if ($SkipConfigureFFmpeg)
    {
        $ffmpegparam = "-SkipConfigure"
    }
    else
    {
        $ffmpegparam = ""
    }

    $folderName = "FFmpeg$WindowsTarget"

    invoke $BashExe --login -x $SolutionDir\Build\FFmpegConfig.sh $WindowsTarget $Platform $SharedOrStatic $Gpl $Encoders $Devices $Programs $folderName $ffmpegparam

    # Copy PDBs to built binaries dir
    Get-ChildItem -Recurse -Include '*.pdb' $build\int\ffmpeg\ | Copy-Item -Destination $target\bin\ -Force

    # Copy license files
    if ($Gpl -eq "enable") {
        # Openssl3 requires GPLv3 when not using LGPL
        Copy-Item $SolutionDir\Libs\FFmpeg\COPYING.GPLv3 $target\licenses\ffmpeg.txt -Force
    }
    else {
        Copy-Item $SolutionDir\Libs\FFmpeg\COPYING.LGPLv2.1 $target\licenses\ffmpeg.txt -Force
    }
    Copy-Item $build\licenses\* $target\licenses\ -Force
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

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

Write-Host
Write-Host "Building FFmpeg$WindowsTarget"
Write-Host
Write-Host "Timestamp: $timestamp"
Write-Host

# Stop on all PowerShell command errors
$ErrorActionPreference = "Stop"

if (! (Test-Path $PSScriptRoot\Libs\ffmpeg\configure)) {
    Write-Error 'configure is not found in ffmpeg folder. Ensure this folder is populated with ffmpeg snapshot'
    Exit 1
}

# Check build tools

if (!(Test-Path $BashExe)) {
    $msysFound = $false
    @( 'C:\msys64', 'C:\msys' ) | ForEach-Object {
        if (Test-Path $_) {
            $BashExe = "${_}\usr\bin\bash.exe"
            $msysFound = $true

            break
        }
    }

    if (! $msysFound) {
        Write-Warning "MSYS2 not found."
        choice /c YN /m "Do you want to install MSYS2 now to C:\msys64?"
        if ($LASTEXITCODE -eq 1)
        {
            .\Build\InstallTools.ps1 MSYS2
            $BashExe = "C:\msys64\usr\bin\bash.exe"
            if (!(Test-Path $BashExe)) {
                Exit 1
            }
        }
        else
        {
            Exit 1
        }
    }
}

if (! (Test-Path "$PSScriptRoot\Tools\nasm")) {
    Write-Warning "NASM not found. Installing..."
    .\Build\InstallTools.ps1 nasm
}

if (! (Test-Path "$PSScriptRoot\Tools\perl")) {
    Write-Warning "Perl not found. Installing..."
    .\Build\InstallTools.ps1 perl
}

[System.IO.DirectoryInfo] $vsLatestPath = `
    & "$VSInstallerFolder\vswhere.exe" `
    $VsWhereCriteria `
    -property installationPath `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64

if (!$vsLatestPath){
    Write-Error "Visual Studio not found!"
}

Write-Host "Visual Studio Installation folder: [$vsLatestPath]"

# 14.16.27023 => v141
$platformToolSet = "v$($VcVersion.Major)$("$($VcVersion.Minor)"[0])"
Write-Host "Platform Toolset: [$platformToolSet]"

# Export full current PATH from environment into MSYS2
$env:MSYS2_PATH_TYPE = 'inherit'

# Check for nuget.exe if package shall be created
if ($NugetPackageVersion)
{
    try
    {
        nuget > $null
    }
    catch
    {
        Write-Error "nuget.exe not found."
    }
}

$start = Get-Date
$success = 1

if ($AllowParallelBuilds -and $Platforms.Count -gt 1)
{
    $processes = @{}
    $clear = ""
    if ($ClearBuildFolders)
    {
        $clear = "-ClearBuildFolders"
    }

    $addparams = ""
    if ($SkipBuildLibs)
    {
        $addparams += " -SkipBuildLibs"
    }

    if ($SkipConfigureFFmpeg)
    {
        $addparams += " -SkipConfigureFFmpeg"
    }

    $skipPkgConfig = ""
    foreach ($platform in $Platforms) {
        if ($SkipBuildPkgConfigFake)
        {
            $skip
        }
        $proc = Start-Process -PassThru powershell "-File .\Build-FFmpeg.ps1 -Platforms $platform -VcVersion $VcVersion -WindowsTarget $WindowsTarget -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion -Configuration $Configuration -SharedOrStatic $SharedOrStatic -Gpl $Gpl -Encoders $Encoders -Devices $Devices -Programs $Programs -VSInstallerFolder ""$VSInstallerFolder"" -VsWhereCriteria ""$VsWhereCriteria"" -BashExe ""$BashExe"" $clear -FFmpegUrl $FFmpegUrl -FFmpegCommit $FFmpegCommit $skipPkgConfig $addparams"
        $processes[$platform] = $proc

        # only build PkgConfigFake once
        $skipPkgConfig = "-SkipBuildPkgConfigFake"
    }

    foreach ($platform in $Platforms) {
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
    # Save orignal environment variables
    $oldEnv = @{};
    foreach ($item in Get-ChildItem env:)
    {
        $oldEnv.Add($item.Name, $item.Value);
    }

    foreach ($platform in $Platforms) {

        try { Stop-Transcript } catch { }

        $logFile = "${PSScriptRoot}\Intermediate\FFmpeg$WindowsTarget\Build_" + $timestamp + "_$platform.log"

        try
        {
            Build-Platform `
                -SolutionDir "${PSScriptRoot}\" `
                -Platform $platform `
                -Configuration 'Release' `
                -WindowsTargetPlatformVersion $WindowsTargetPlatformVersion `
                -VcVersion $VcVersion `
                -PlatformToolset $platformToolSet `
                -VsLatestPath $vsLatestPath `
                -BashExe $BashExe `
                -LogFileName $logFile `
                -SkipBuildPkgConfigFake $SkipBuildPkgConfigFake `
                -SkipBuildLibs $SkipBuildLibs `
                -SkipConfigureFFmpeg $SkipConfigureFFmpeg
        }
        catch
        {
            Write-Warning "Error occured: $PSItem"
            $success = 0
            Break
        }
        finally
        {
            try { Stop-Transcript } catch { }

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

        # only build PkgConfigFake once
        $BuildPkgConfigFake = $false;
    }
}

if ($success -and $NugetPackageVersion)
{
    nuget pack .\Build\FFmpegInteropX.$WindowsTarget.FFmpeg.nuspec `
        -Properties "id=FFmpegInteropX.$WindowsTarget.FFmpeg;repositoryUrl=$FFmpegUrl;repositoryCommit=$FFmpegCommit;winsdk=$WindowsTargetPlatformVersion;NoWarn=NU5128" `
        -Version $NugetPackageVersion `
        -Symbols -SymbolPackageFormat symbols.nupkg `
        -OutputDirectory "${PSScriptRoot}\Output\NuGet"
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
    Write-Warning 'Build failed!'
    Exit 1
}
