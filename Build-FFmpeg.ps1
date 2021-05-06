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
    [version] $WindowsTargetPlatformVersion = '10.0.19041.0',

    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Release',
    
    # shared: create shared dll - static: create lib for static linking
    [ValidateSet('shared', 'static')]
    [string] $SharedOrStatic = 'shared',

    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    # Set the search criteria for VSWHERE.EXE.
    [string[]] $VsWhereCriteria = '-latest',

    [System.IO.FileInfo] $BashExe = 'C:\msys64\usr\bin\bash.exe',

    [switch] $ClearBuildFolders,

    # If a version string is specified, a NuGet package will be created.
    [string] $NugetPackageVersion = $null,

    # FFmpeg NuGet settings
    [string] $FFmpegUrl = 'https://git.ffmpeg.org/ffmpeg.git',

    [string] $FFmpegCommit = $(git --git-dir Libs/ffmpeg/.git rev-parse HEAD)
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
        [string] $BashExe = 'C:\msys64\usr\bin\bash.exe'
    )

    $PSBoundParameters | Out-String

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

    # Build pkg-config fake
    MSBuild.exe $SolutionDir\Libs\PkgConfigFake\PkgConfigFake.csproj `
        /p:OutputPath="$SolutionDir\Intermediate\" `
        /p:Configuration=$Configuration `
        /p:Platform=${Env:\PreferredToolArchitecture}

    if ($lastexitcode -ne 0) { throw "Failed to build PkgConfigFake." }

    New-Item -ItemType Directory -Force $SolutionDir\Intermediate\FFmpeg$WindowsTarget\$Platform -OutVariable build

    New-Item -ItemType Directory -Force $SolutionDir\Output\FFmpeg$WindowsTarget\$Platform -OutVariable target

    if ($ClearBuildFolders) {
        # Clean platform-specific build and output dirs.
        Remove-Item -Force -Recurse $build\*
        Remove-Item -Force -Recurse $target\*
    }

    ('lib', 'licenses', 'include') | ForEach-Object {
        New-Item -ItemType Directory -Force $build\$_
        New-Item -ItemType Directory -Force $target\$_
    }
    
    $env:LIB += ";$build\lib"
    $env:INCLUDE += ";$build\include"
    $env:Path += ";$SolutionDir\Libs\gas-preprocessor"

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

        MSBuild.exe $SolutionDir\Libs\$folder\SMP\$project.vcxproj `
            /p:OutDir=$outDir `
            /p:IntDir=$intDir `
            /p:Configuration=$configurationName `
            /p:Platform=$Platform `
            /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
            /p:PlatformToolset=$PlatformToolset `
            /p:ForceImportBeforeCppTargets=$SolutionDir\Libs\LibOverrides.props `
            /p:useenv=true

        if ($lastexitcode -ne 0) { throw "Failed to build library $project." }

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


    if ($WindowsTarget -eq "Desktop") { 
        
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

        $ErrorActionPreference = "Continue"
        cmd.exe /C $SolutionDir\Libs\build-x265.bat $SolutionDir\Libs\x265\source $build\int\x265 $cmakePlatform $PlatformToolset
        $ErrorActionPreference = "Stop"
        if ($lastexitcode -ne 0) { throw "Failed to build library x264." }

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

        $ErrorActionPreference = "Continue"
        & $BashExe --login -c "cd \$build\x264 && CC=cl ..\..\..\..\Libs\x264\configure --host=${x264Arch}-mingw64 --prefix=\$build --disable-cli --enable-static && make -j8 && make install".Replace("\", "/").Replace(":", "")
        $ErrorActionPreference = "Stop"
        if ($lastexitcode -ne 0) { throw "Failed to build library x264." }


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
        
        $ErrorActionPreference = "Continue"
        & $BashExe --login -c "cd \$build\libvpx && ..\..\..\..\Libs\libvpx\configure --target=${vpxArch}-${vpxPlatform}-vs15 --prefix=\$build --enable-static --disable-thumb --disable-debug --disable-examples --disable-tools --disable-docs --disable-unit_tests && make -j8 && make install".Replace("\", "/").Replace(":", "")
        $ErrorActionPreference = "Stop"
        if ($lastexitcode -ne 0) { throw "Failed to build library libvpx." }

        Move-Item $build\lib\$cmakePlatform\vpxmd.lib $build\lib\vpx.lib -Force
        Remove-Item $build\lib\$cmakePlatform -Force -Recurse
    } 

    # Build ffmpeg
    Write-Host
    Write-Host "Building FFmpeg..."
    Write-Host

    $ErrorActionPreference = "Continue"
    & $BashExe --login -x $SolutionDir\FFmpegConfig.sh $WindowsTarget $Platform $SharedOrStatic
    $ErrorActionPreference = "Stop"
    if ($lastexitcode -ne 0) { throw "Failed to build FFmpeg." }

    # Copy PDBs to built binaries dir
    Get-ChildItem -Recurse -Include '*.pdb' $build\int\ffmpeg\ | Copy-Item -Destination $target\bin\ -Force

    # Copy license files
    Copy-Item $SolutionDir\Libs\FFmpeg\COPYING.LGPLv2.1 $target\licenses\ffmpeg.txt -Force
    Copy-Item $build\licenses\* $target\licenses\ -Force
}

Write-Host
Write-Host "Building FFmpeg$WindowsTarget"
Write-Host

# Stop on all PowerShell command errors
$ErrorActionPreference = "Stop"

if (! (Test-Path $PSScriptRoot\Libs\ffmpeg\configure)) {
    Write-Error 'configure is not found in ffmpeg folder. Ensure this folder is populated with ffmpeg snapshot'
    Exit 1
}

# Search for MSYS locations
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
        Write-Error "MSYS2 not found."
        Exit 1;
    }
}

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

# Check for nuget.exe if package shall be created
if ($NugetPackageVersion)
{
    try
    {
        nuget > 0
    }
    catch
    {
        Write-Error "nuget.exe not found."
    }
}

$start = Get-Date
$success = 1

foreach ($platform in $Platforms) {

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
            -BashExe $BashExe
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
    nuget pack .\FFmpegInteropX.FFmpegUWP.nuspec `
        -Properties "id=FFmpegInteropX.FFmpegUWP;repositoryUrl=$FFmpegUrl;repositoryCommit=$FFmpegCommit;NoWarn=NU5128" `
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
