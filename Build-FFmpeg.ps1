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

    [ValidateSet('UWP', 'Win32')]
    [string] $WindowsTarget = 'UWP',

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
    
    # shared: create shared dll - static: create lib for static linking
    [ValidateSet('shared', 'static')]
    [string] $SharedOrStatic = 'shared',

    [System.IO.DirectoryInfo] $VSInstallerFolder = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer",

    # Set the search criteria for VSWHERE.EXE.
    [string[]] $VsWhereCriteria = '-latest',

    [System.IO.FileInfo] $Msys2Bin = 'C:\msys64\usr\bin\bash.exe',

    [Boolean] $ClearBuildFolders = $false
)

function Build-Platform {
    param (
        [System.IO.DirectoryInfo] $SolutionDir,
        [string] $Platform,
        [string] $Configuration,
        [string] $PlatformToolset,
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

    Write-Host ""
    Write-Host "Building FFmpeg for Windows 10 ($WindowsTarget) ${Platform}..."
    Write-Host ""
    
    # Load environment from VCVARS.
    $vcvarsArch = $vcvarsArchs[$env:PROCESSOR_ARCHITECTURE][$Platform]

    # Decide vcvars target string based on target platform
    $windowsTargetString = if ($WindowsTarget -eq "UWP") { "uwp $WindowsTargetPlatformVersion" } else { $windowsTargetString = "" }

    CMD /c "`"$VsLatestPath\VC\Auxiliary\Build\vcvarsall.bat`" $vcvarsArch $windowsTargetString -vcvars_ver=$VcVersion && SET" | . {
        PROCESS {
            Write-Host $_
            if ($_ -match '^([^=]+)=(.*)') {
                if ($Matches[1] -notin 'HOME') {
                    Set-Item -Path "Env:\$($Matches[1])" -Value $Matches[2]
                }
            }
        }
    }

    if ($lastexitcode -ne 0) { throw "Failed to configure vcvarsall environment." }


    # Build pkg-config fake
    MSBuild.exe $SolutionDir\Libs\PkgConfigFake\PkgConfigFake.csproj `
        /p:OutputPath="$SolutionDir\Output\" `
        /p:Configuration="Release" `
        /p:Platform=${Env:\PreferredToolArchitecture}

    if ($lastexitcode -ne 0) { throw "Failed to build PkgConfigFake." }
    
    New-Item -ItemType Directory -Force $SolutionDir\Output\FFmpeg$WindowsTarget\$Platform -OutVariable build

    New-Item -ItemType Directory -Force $SolutionDir\FFmpeg$WindowsTarget\$Platform -OutVariable target
    
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

        Write-Host ""
        Write-Host "Building Library ${folder}..."
        Write-Host ""

        # Decide vcvars target string based on target platform
        if ($WindowsTarget -eq "UWP") { 
            $configurationName = "${Configuration}WinRT"
            $targetName = "${project}_winrt"
            $outDir = "$build\"
        }
        else {
            $configurationName = ${Configuration}
            $targetName = ${project}
            $outDir = "$build\$project\"
        }
        $intDir = "$build\int\$project\"

        MSBuild.exe $SolutionDir\Libs\$folder\SMP\$project.vcxproj `
            /p:OutDir=$outDir `
            /p:IntDir=$intDir `
            /p:Configuration=$configurationName `
            /p:Platform=$Platform `
            /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
            /p:PlatformToolset=$PlatformToolset `
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


    if ($WindowsTarget -eq "Win32") { 
        
        $env:Path += ";$(Split-Path $Msys2Bin)"

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
        & $Msys2Bin --login -c "cd \$build\x264 && CC=cl ..\..\..\..\Libs\x264\configure --host=${x264Arch}-mingw64 --prefix=\$build --disable-cli --enable-static && make -j8 && make install".Replace("\", "/").Replace(":", "")
        $ErrorActionPreference = "Stop"
        if ($lastexitcode -ne 0) { throw "Failed to build library x264." }


        #Build libvpx
        Write-Host ""
        Write-Host "Building Library libvpx..."
        Write-Host ""

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
        & $Msys2Bin --login -c "cd \$build\libvpx && ..\..\..\..\Libs\libvpx\configure --target=${vpxArch}-${vpxPlatform}-vs15 --prefix=\$build --enable-static --disable-thumb --disable-debug --disable-examples --disable-tools --disable-docs --disable-unit_tests && make -j8 && make install".Replace("\", "/").Replace(":", "")
        $ErrorActionPreference = "Stop"
        if ($lastexitcode -ne 0) { throw "Failed to build library libvpx." }

        Move-Item $build\lib\$cmakePlatform\vpxmd.lib $build\lib\vpx.lib -Force
        Remove-Item $build\lib\$cmakePlatform -Force -Recurse
    } 

    # Build ffmpeg
    Write-Host ""
    Write-Host "Building FFmpeg..."
    Write-Host ""

    $ErrorActionPreference = "Continue"
    & $Msys2Bin --login -x $SolutionDir\FFmpegConfig.sh $WindowsTarget $Platform $SharedOrStatic
    $ErrorActionPreference = "Stop"
    if ($lastexitcode -ne 0) { throw "Failed to build FFmpeg." }

    # Copy PDBs to built binaries dir
    Copy-Item $build\int\ffmpeg\*.pdb $target\bin\ -Force

    # Copy license files
    Copy-Item $SolutionDir\FFmpeg\COPYING.LGPLv2.1 $target\licenses\ffmpeg.txt -Force
    Copy-Item $build\licenses\* $target\licenses\ -Force
}

Write-Host
Write-Host "Building FFmpegInteropX..."
Write-Host

# Stop on all PowerShell command errors
$ErrorActionPreference = "Stop"

if (! (Test-Path $PSScriptRoot\ffmpeg\configure)) {
    Write-Error 'configure is not found in ffmpeg folder. Ensure this folder is populated with ffmpeg snapshot'
    Exit 1
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

$start = Get-Date
$success = 1

foreach ($platform in $Platforms) {

    try
    {
        Build-Platform `
            -SolutionDir "${PSScriptRoot}\" `
            -Platform $platform `
            -Configuration 'Release' `
            -PlatformToolset $platformToolSet `
            -VsLatestPath $vsLatestPath `
            -Msys2Bin $Msys2Bin
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

Write-Host ''
Write-Host 'Time elapsed'
Write-Host ('{0}' -f ((Get-Date) - $start))
Write-Host ''

if ($success)
{
    Write-Host 'Build succeeded!'

}
else
{
    Write-Host 'Build failed!'
    Exit 1
}

