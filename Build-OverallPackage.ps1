param(

    # If a version string is specified, a NuGet package will be created.
    [string] $NugetPackageVersion,

    # If a version string is specified, a NuGet package will be created.
    [string] $FFmpegPackageVersion,

    # If a version string is specified, a NuGet package will be created.
    [string] $LibPackageVersion,

    [version] $WindowsTargetPlatformVersion = '10.0.22621.0',

    # FFmpegInteropX NuGet settings
    [string] $FFmpegInteropXUrl = 'https://github.com/ffmpeginteropx/FFmpegInteropX.git',

    [string] $FFmpegInteropXBranch = $(git branch --show-current),

    [string] $FFmpegInteropXCommit = $(git --git-dir Libs/ffmpeg/.git rev-parse HEAD)
)


nuget pack .\Build\FFmpegInteropX.nuspec `
    -Properties "id=FFmpegInteropX;repositoryUrl=$FFmpegUrl;repositoryCommit=$FFmpegCommit;winsdk=$WindowsTargetPlatformVersion;libversion=$LibPackageVersion;ffmpegversion=$FFmpegPackageVersion;NoWarn=NU5128" `
    -Version $NugetPackageVersion `
    -OutputDirectory "${PSScriptRoot}\Output\NuGet"
