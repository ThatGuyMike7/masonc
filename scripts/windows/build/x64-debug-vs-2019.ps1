# Move from "masonc/scripts/windows/build" to "masonc".
Set-Location -Path "../../.."

# Create directory "masonc/build".
if (!(Test-Path "build"))
{
    New-Item -ItemType Directory -Force -Path "build"
}

# Platform- and build configuration specific directories are created by CMake.

# Create directory "masonc/intermediate".
if (!(Test-Path "intermediate"))
{
    New-Item -ItemType Directory -Force -Path "intermediate"
}

# Create directory "masonc/intermediate/cmake".
if (!(Test-Path "intermediate/cmake"))
{
    New-Item -ItemType Directory -Force -Path "intermediate/cmake"
}

# Move from "masonc" to "masonc/intermediate/cmake"
Set-Location -Path "intermediate/cmake"

$cmake_configure = 'cmake ../.. -G"Visual Studio 16" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Debug'
Invoke-Expression "& $cmake_configure"

$cmake_build = 'cmake --build . --config Debug'
Invoke-Expression "& $cmake_build"

if (Test-Path "../../build/win-64/debug/masonc.exe")
{
    # Run the built executable with working directory being the project root directory.
    Start-Process -FilePath "../../build/win-64/debug/masonc.exe" -WorkingDirectory "../.."
}

# If running in the console, wait for input before closing.
if ($Host.Name -eq "ConsoleHost")
{
    Write-Host "Press any key to continue..."
    $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyUp") > $null
}