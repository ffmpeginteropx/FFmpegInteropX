param(
    [ValidateSet('MSYS2', 'nasm', 'perl')]
    [string[]] $Tools = ('MSYS2', 'nasm', 'perl'),
    [string] $NasmVersion = "2.15.05",
    [string] $PerlVersion = "5.32.1.1"
)

# Stop on all PowerShell command errors
$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Path $PSScriptRoot\..\Tools -Force -OutVariable root

if ($Tools.Contains("MSYS2"))
{
    if (!(Test-Path "C:\msys64"))
    {
		Write-Host
        Write-Host "Installing MSYS2..."
        Write-Host
		
        # Download and install MSYS2
        curl.exe -SL --output msys2-x86_64-latest.sfx.exe https://repo.msys2.org/distrib/msys2-x86_64-latest.sfx.exe
        .\msys2-x86_64-latest.sfx.exe -oC:\

        Remove-Item .\msys2-x86_64-latest.sfx.exe

        # Install MSYS2 dependencies
        C:\msys64\usr\bin\bash.exe --login -c "pacman --noconfirm -Syu"
        C:\msys64\usr\bin\bash.exe --login -c "pacman --noconfirm -Su"
        C:\msys64\usr\bin\bash.exe --login -c "pacman --noconfirm -S make perl diffutils yasm nasm mingw-w64-x86_64-meson mingw-w64-x86_64-ninja"
		C:\msys64\usr\bin\bash.exe --login -c "pacman --noconfirm -Scc"
		C:\msys64\usr\bin\bash.exe --login -c "pacman --noconfirm -S gcc"
    }
	else
	{
		Write-Host
        Write-Host "Updating MSYS2 packages..."
        Write-Host
		
		C:\msys64\usr\bin\bash.exe --login -c "pacman --noconfirm -Syu"
		C:\msys64\usr\bin\bash.exe --login -c "pacman --noconfirm -Scc"
	}
}

if ($Tools.Contains("nasm"))
{
	Write-Host
	Write-Host "Installing NASM..."
	Write-Host
		
    $temp = "$root\nasm-$NasmVersion"
    $target = "$root\nasm"

    Remove-Item $temp -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item $target -Recurse -Force -ErrorAction SilentlyContinue

    curl.exe -SL --output nasm.zip https://www.nasm.us/pub/nasm/releasebuilds/$NasmVersion/win64/nasm-$NasmVersion-win64.zip
    Expand-Archive -Path nasm.zip -DestinationPath $root
    Rename-Item -Path $temp -NewName "nasm"

    Remove-Item .\nasm.zip
}

if ($Tools.Contains("perl"))
{
	Write-Host
	Write-Host "Installing Perl..."
	Write-Host
	
    $target = "$root\perl"

    Remove-Item $target -Recurse -Force -ErrorAction SilentlyContinue

    curl.exe -SL --output perl.zip https://strawberryperl.com/download/$PerlVersion/strawberry-perl-$PerlVersion-64bit-portable.zip
    Expand-Archive -Path perl.zip -DestinationPath $target

    Remove-Item .\perl.zip
}