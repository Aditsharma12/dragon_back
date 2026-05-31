$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

$ProjectDir = $PSScriptRoot -replace '\\','/'
$ProjectDir = $ProjectDir -replace '^([A-Za-z]):', '/\L$1'

# Convert Windows path to MSYS2 path  e.g. C:/foo -> /c/foo
$MsysProjectDir = ($PSScriptRoot -replace '\\','/') -replace '^([A-Za-z]):', { "/$(($args[0].Groups[1].Value).ToLower())" }

Write-Host "Building Drogon backend inside MSYS2 shell..." -ForegroundColor Cyan

$BuildScript = @"
set -e
export PATH="/mingw64/bin:/usr/bin:`$PATH"
PROJECT='/c/Users/adit1/OneDrive/Desktop/cpp back'
cd "`$PROJECT"
rm -rf build
cmake -G 'MinGW Makefiles' -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/mingw64
mingw32-make -C build -j4

# Copy required DLLs next to the exe
DLLDIR=/mingw64/bin
EXEDIR="`$PROJECT/build"
for dll in libdrogon.dll libtrantor.dll libjsoncpp-25.dll libssl-3-x64.dll libcrypto-3-x64.dll \
           libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll zlib1.dll \
           libbrotlidec.dll libbrotlicommon.dll libcares-2.dll; do
    [ -f "`$DLLDIR/`$dll" ] && cp -u "`$DLLDIR/`$dll" "`$EXEDIR/" && echo "  Copied `$dll"
done
echo "Build done!"
"@

$BuildScript | Out-File -Encoding ascii -FilePath "$env:TEMP\drogon_build.sh"

& C:\msys64\usr\bin\bash.exe -l "$env:TEMP\drogon_build.sh"

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    exit 1
}

Write-Host ""
Write-Host "Starting server at http://localhost:8080 ..." -ForegroundColor Yellow

# Add MSYS2 DLLs to PATH so the exe can find them at runtime
$env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH
& ".\build\drogon_backend.exe"
