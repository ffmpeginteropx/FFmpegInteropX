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

    [Boolean] $ClearBuildFolders = $true
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

    Write-Host "Building FFmpeg for Windows 10 apps ${Platform}..."
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
        /p:OutputPath="$SolutionDir\Libs\Build\" `
        /p:Configuration="Release" `
        /p:Platform=${Env:\PreferredToolArchitecture}

    if ($lastexitcode -ne 0) { throw "Failed to build PkgConfigFake." }


    New-Item -ItemType Directory -Force $SolutionDir\Libs\Build\$Platform -OutVariable libs

    ('lib', 'licenses', 'include', 'build') | ForEach-Object {
        New-Item -ItemType Directory -Force $libs\$_
    }
    
    $env:LIB += ";${libs}\lib"
    $env:INCLUDE += ";${libs}\include"

    if ($ClearBuildFolders) {
        # Clean platform-specific library build dirs.
        Remove-Item -Force -Recurse $libs\build\*
        Remove-Item -Force -Recurse $libs\lib\*
        Remove-Item -Force -Recurse $libs\include\*
        Remove-Item -Force -Recurse $libs\licenses\*
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

        # Decide vcvars target string based on target platform
        if ($WindowsTarget -eq "UWP") { 
            $configurationName = "${Configuration}WinRT"
            $targetName = "${project}_winrt"
            $outDir = "$libs\build\"
        }
        else {
            $configurationName = ${Configuration}
            $targetName = ${project}
            $outDir = "$libs\build\$project\"
        }
        $intDir = "$libs\build\int\$project\"

        MSBuild.exe $SolutionDir\Libs\$folder\SMP\$project.vcxproj `
            /p:OutDir=$outDir `
            /p:IntDir=$intDir `
            /p:Configuration=$configurationName `
            /p:Platform=$Platform `
            /p:WindowsTargetPlatformVersion=$WindowsTargetPlatformVersion `
            /p:PlatformToolset=$PlatformToolset `
            /p:useenv=true

        if ($lastexitcode -ne 0) { throw "Failed to build library $project." }

        Copy-Item $libs\build\$project\include\* $libs\include\ -Recurse -Force
        Copy-Item $libs\build\$project\licenses\* $libs\licenses\ -Recurse -Force
        Copy-Item $libs\build\$project\lib\$Platform\$targetName.lib $libs\lib\ -Force
        Copy-Item $libs\build\$project\lib\$Platform\$targetName.pdb $libs\lib\ -Force
    }

    # Rename all libraries to ffmpeg target names
    $libdefs | ForEach-Object {

        $project = $_[1];
        $target = $_[2];
        $targetName = if ($WindowsTarget -eq "UWP") { "${project}_winrt" } else { $project }

        Move-Item $libs\lib\$targetName.lib $libs\lib\$target.lib -Force
        Move-Item $libs\lib\$targetName.pdb $libs\lib\$target.pdb -Force
    }

    # Fixup libxml2 includes for ffmpeg build
    Copy-Item $libs\include\libxml2\libxml $libs\include\ -Force -Recurse


    if ($WindowsTarget -eq "Win32") { 

        # Build x264
        
        $x264Archs = @{
            'x86'   = 'x86'
            'x64'   = 'x86_64'
            'ARM'   = 'arm'
            'ARM64' = 'aarch64'
        }
        $x264Arch = $x264Archs[$Platform]

        New-Item -ItemType Directory -Force $libs\build\x264

        $ErrorActionPreference = "Continue"
        & $Msys2Bin --login -c "cd \$libs\build\x264 && CC=cl ..\..\..\..\x264\configure --host=${x264Arch}-mingw64 --prefix=\$libs --disable-cli --enable-static && make -j8 && make install".Replace("\", "/").Replace(":", "")
        $ErrorActionPreference = "Stop"
        if ($lastexitcode -ne 0) { throw "Failed to build library x264." }


        #Build libvpx

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

        New-Item -ItemType Directory -Force $libs\build\libvpx
        
        $ErrorActionPreference = "Continue"
        & $Msys2Bin --login -c "cd \$libs\build\libvpx && ..\..\..\..\libvpx\configure --target=${vpxArch}-${vpxPlatform}-vs15 --prefix=\$libs --enable-static --disable-thumb --disable-debug --disable-examples --disable-tools --disable-docs --disable-unit_tests && make -j8 && make install".Replace("\", "/").Replace(":", "")
        $ErrorActionPreference = "Stop"
        if ($lastexitcode -ne 0) { throw "Failed to build library libvpx." }

        $vpxOutDirs = @{
            'x86'   = 'Win32'
            'x64'   = 'x64'
            'ARM'   = 'ARM'
            'ARM64' = 'ARM64'
        }
        $vpxOutDir = $vpxOutDirs[$Platform]

        Move-Item $libs\lib\$vpxOutDir\vpxmd.lib $libs\lib\vpx.lib -Force
        Remove-Item $libs\lib\$vpxOutDir -Force -Recurse
    } 


    # Build ffmpeg - disable strict error handling since ffmpeg writes to error out
    $ErrorActionPreference = "Continue"
    & $Msys2Bin --login -x $SolutionDir\FFmpegConfig.sh $WindowsTarget $Platform $SharedOrStatic
    $ErrorActionPreference = "Stop"
    if ($lastexitcode -ne 0) { throw "Failed to build FFmpeg." }

    # Copy PDBs to built binaries dir
    Copy-Item $libs\build\ffmpeg\*.pdb $libs\bin\ -Force
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

