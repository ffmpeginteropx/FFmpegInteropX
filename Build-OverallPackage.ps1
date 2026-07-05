param(

    # The referenced FFmpeg NuGet package version to use in the overall NuGet package.
    [string] $FFmpegPackageVersion,

    # The referenced FFmpegInteropX library version to use in the overall NuGet package.
    [string] $LibPackageVersion,

    # The version number of the overall NuGet package to create.
    [string] $OverallPackageVersion = $null,

    [ValidateSet('UWP', 'Desktop')]
    [string[]] $WindowsTargets = ('Desktop', 'UWP'),

    [version] $WindowsTargetPlatformMinVersion = '10.0.17763.0',

    # FFmpegInteropX NuGet settings
    [string] $FFmpegInteropXUrl = 'https://github.com/ffmpeginteropx/FFmpegInteropX.git',

    [string] $FFmpegInteropXBranch = $(git branch --show-current),
    
    [string] $FFmpegInteropXCommit = $(git rev-parse HEAD)
)

if (!$OverallPackageVersion)
{
    # Get version from LibPackageVersion (remove prerelease suffix if present)
    $libPart = $LibPackageVersion -replace '-.*$', '' # Remove prerelease suffix

    # Get Revision by concatenating first three parts of FFmpeg version
    $ffmpegPart = $FFmpegPackageVersion -replace '-.*$', '' # Remove prerelease suffix
    $revisionPart = [string]::Join("", ($ffmpegPart -split '\.' | Select-Object -First 3))
    $revisionEnd = "00"

    # Get prerelease suffix from LibPackageVersion (if present)
    $prereleasePart = $LibPackageVersion -replace '^[0-9]+\.[0-9]+\.[0-9]+', '' # Remove first three parts
    $OverallPackageVersion = "$libPart.$revisionPart$revisionEnd$prereleasePart"
}

if ($WindowsTargets -contains 'Desktop') {
    nuget pack .\Build\FFmpegInteropX.nuspec `
        -Properties "id=FFmpegInteropX;repositoryUrl=$FFmpegInteropXUrl;repositoryCommit=$FFmpegInteropXCommit;winsdk=$WindowsTargetPlatformMinVersion;libversion=$LibPackageVersion;ffmpegversion=$FFmpegPackageVersion;NoWarn=NU5128" `
        -Version $OverallPackageVersion `
        -OutputDirectory "${PSScriptRoot}\Output\NuGet" `
}

if ($WindowsTargets -contains 'UWP') {
    nuget pack .\Build\FFmpegInteropX.UWP.nuspec `
        -Properties "id=FFmpegInteropX.UWP;repositoryUrl=$FFmpegInteropXUrl;repositoryCommit=$FFmpegInteropXCommit;winsdk=$WindowsTargetPlatformMinVersion;libversion=$LibPackageVersion;ffmpegversion=$FFmpegPackageVersion;NoWarn=NU5128" `
        -Version $OverallPackageVersion `
        -OutputDirectory "${PSScriptRoot}\Output\NuGet" `
}
