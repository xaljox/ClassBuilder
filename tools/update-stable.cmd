@echo off
REM update-stable.cmd -- right-click this file and pick "Run as administrator".
REM
REM It just runs update-stable.ps1 (same folder), which copies the freshly
REM built Release ClassBuilder.exe to C:\Program Files (x86)\ClassBuilder,
REM keeps the previous one as a numbered fallback, and strips leftover Qt
REM deployment files. The window stays open so you can read the result.

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0update-stable.ps1"
