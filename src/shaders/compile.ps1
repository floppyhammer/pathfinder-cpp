New-Variable -Name "GLSLC" -Visibility Public -Value $env:VULKAN_SDK"/Bin/glslc.exe"

& $GLSLC blit.vert -o blit.vert.spv
& $GLSLC blit.frag -o blit.frag.spv

& $GLSLC d3d9/fill.vert -o d3d9/fill.vert.spv
& $GLSLC d3d9/fill.frag -o d3d9/fill.frag.spv
& $GLSLC d3d9/tile.vert -o d3d9/tile.vert.spv
& $GLSLC d3d9/tile.frag -o d3d9/tile.frag.spv

& $GLSLC d3d11/bin.comp -o d3d11/bin.comp.spv
& $GLSLC d3d11/bound.comp -o d3d11/bound.comp.spv
& $GLSLC d3d11/dice.comp -o d3d11/dice.comp.spv
& $GLSLC d3d11/fill.comp -o d3d11/fill.comp.spv
& $GLSLC d3d11/propagate.comp -o d3d11/propagate.comp.spv
& $GLSLC d3d11/sort.comp -o d3d11/sort.comp.spv
& $GLSLC d3d11/tile.comp -o d3d11/tile.comp.spv

# Wait for input.
Write-Host "All jobs finished."
$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
