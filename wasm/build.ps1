emcmake cmake .. -G "MinGW Makefiles"

emmake make

Write-Host "Building finished."
$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
