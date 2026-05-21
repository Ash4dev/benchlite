# https://learn.microsoft.com/en-us/windows/win32/power/power-policy-settings

# Set to High Performance
powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c

# Run the executable (.\main.exe)
cmake --build .\build
.\build\main.exe
$exitCode = $LASTEXITCODE

# Reset to Balanced
powercfg /setactive 381b4222-f694-41f0-9685-ff5bb260df2e

exit $exitCode

