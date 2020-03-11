@echo off
echo .
echo FFmpegInteropX has a new, much more flexible PowerShell build script.
echo Please check the documentation on Github for details
echo .
echo The new build script will now be executed with default settings.

pause

PowerShell -NoProfile -ExecutionPolicy Bypass -Command ".\Build-FFmpeg.ps1"

pause