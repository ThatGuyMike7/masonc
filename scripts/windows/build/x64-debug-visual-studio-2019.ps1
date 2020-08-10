# Move from "masonc/scripts/windows/build" to "masonc".
Set-Location -Path "../../.."

# Create directory "masonc/build".
if (!(Test-Path "build"))
{
    New-Item -ItemType Directory -Force -Path "build"
}

# Create directory "masonc/intermediate".
If (!(Test-Path "intermediate"))
{
    New-Item -ItemType Directory -Force -Path "intermediate"
}

# Create directory "masonc/intermediate/cmake".
If (!(Test-Path "intermediate/cmake"))
{
    New-Item -ItemType Directory -Force -Path "intermediate/cmake"
}

# Move from "masonc" to "masonc/intermediate/cmake"
Set-Location -Path "intermediate/cmake"

# cmake -G "Visual Studio 15" -D CMAKE_GENERATOR_PLATFORM=x64 -D CMAKE_BUILD_TYPE=Debug ..
# cmake --build . --config Debug

$cmake_configure = 'cmake ../.. -G"Visual Studio 16" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Debug'
Invoke-Expression "& $cmake_configure"

# Move from "masonc/intermediate/cmake" to "masonc/build/win-64/debug".
#Set-Location -Path "../../build/win-64/debug"

$cmake_build = 'cmake --build . --config Debug'
Invoke-Expression "& $cmake_build"

#Start-Process -FilePath notepad.exe -WorkingDirectory c:\temp

# If running in the console, wait for input before closing.
if ($Host.Name -eq "ConsoleHost")
{
    Write-Host "Press any key to continue..."
    $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyUp") > $null
}