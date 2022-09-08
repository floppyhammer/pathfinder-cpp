New-Variable -Name "GLSLC" -Visibility Public -Value "C:/VulkanSDK/1.3.224.1/Bin/glslc.exe"

& $GLSLC blit.vert -o blit_vert.spv
& $GLSLC blit.frag -o blit_frag.spv

& $GLSLC d3d9/fill.vert -o d3d9/fill_vert.spv
& $GLSLC d3d9/fill.frag -o d3d9/fill_frag.spv
& $GLSLC d3d9/tile.vert -o d3d9/tile_vert.spv
& $GLSLC d3d9/tile.frag -o d3d9/tile_frag.spv

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
