param(

    # The version number of the overall NuGet package to create.
    [string] $NugetPackageVersion,

    # The referenced FFmpeg NuGet package version to use in the overall NuGet package.
    [string] $FFmpegPackageVersion,

    # The referenced FFmpegInteropX library version to use in the overall NuGet package.
    [string] $LibPackageVersion,

    [ValidateSet('UWP', 'Desktop')]
    [string[]] $WindowsTargets = ('Desktop', 'UWP'),

    [version] $WindowsTargetPlatformMinVersion = '10.0.17763.0',

    # FFmpegInteropX NuGet settings
    [string] $FFmpegInteropXUrl = 'https://github.com/ffmpeginteropx/FFmpegInteropX.git',

    [string] $FFmpegInteropXBranch = $(git branch --show-current),
    
    [string] $FFmpegInteropXCommit = $(git --git-dir Libs/ffmpeg/.git rev-parse HEAD)
)

if ($WindowsTargets -contains 'Desktop') {
    nuget pack .\Build\FFmpegInteropX.nuspec `
        -Properties "id=FFmpegInteropX;repositoryUrl=$FFmpegInteropXUrl;repositoryCommit=$FFmpegInteropXCommit;winsdk=$WindowsTargetPlatformMinVersion;libversion=$LibPackageVersion;ffmpegversion=$FFmpegPackageVersion;NoWarn=NU5128" `
        -Version $NugetPackageVersion `
        -OutputDirectory "${PSScriptRoot}\Output\NuGet" `
}

if ($WindowsTargets -contains 'UWP') {
    nuget pack .\Build\FFmpegInteropX.UWP.nuspec `
        -Properties "id=FFmpegInteropX.UWP;repositoryUrl=$FFmpegInteropXUrl;repositoryCommit=$FFmpegInteropXCommit;winsdk=$WindowsTargetPlatformMinVersion;libversion=$LibPackageVersion;ffmpegversion=$FFmpegPackageVersion;NoWarn=NU5128" `
        -Version $NugetPackageVersion `
        -OutputDirectory "${PSScriptRoot}\Output\NuGet" `
}
