param
(
  [Parameter(mandatory=$true)][string] $File,
  [Parameter(mandatory=$false)][string] $OutputDirectory = $null,
  [Parameter(mandatory=$true)][string] $TargetVersion
)

$File = [System.IO.Path]::GetFullPath("$PSScriptRoot\$File")

if (-not($OutputDirectory))
{
    $OutputDirectory = [System.IO.Path]::GetDirectoryName($File)
}
else
{
    $OutputDirectory = "$PSScriptRoot\$OutputDirectory"
}

Write-Host
Write-Host ==================== Repack NuGet package =====================
Write-Host
Write-Host "File:            $File"
Write-Host "OutputDirectory: $OutputDirectory"
Write-Host "TargetVersion:   $TargetVersion"
Write-Host

# Stop on all PowerShell command errors
$ErrorActionPreference = "Stop"

$temp = "$PSScriptRoot\Temp"
$folder = "$PSScriptRoot\Temp\Package"

if (Test-Path $temp) { Remove-Item -Force -Recurse $temp }

New-Item -ItemType Directory -Force $temp
New-Item -ItemType Directory -Force $folder

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory($File, $folder)

Remove-Item -Force -Recurse $folder\package
Remove-Item -Force -Recurse $folder\_rels
Remove-Item -Force -LiteralPath $folder\[Content_Types].xml

$nuspec = Get-Item $folder\*.nuspec

$xml = [xml](Get-content($nuspec))
$files = $xml.DocumentElement.AppendChild($xml.CreateElement("files"))
$all = $files.AppendChild($xml.CreateElement("file"))
$all.Attributes.Append($xml.CreateAttribute("src"))
$all.src = "Package\**\*.*"
$xml.Save($nuspec)

Move-Item $nuspec $temp\
$nuspec = Get-Item $temp\*.nuspec

nuget pack $nuspec -Version $TargetVersion -PackagesDirectory $folder -OutputDirectory $OutputDirectory -Symbols -SymbolPackageFormat symbols.nupkg -Properties NoWarn=NU5128

Remove-Item -Force -Recurse $temp