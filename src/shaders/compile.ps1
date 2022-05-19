New-Variable -Name "GLSLC" -Visibility Public -Value "C:/VulkanSDK/1.3.204.0/Bin/glslc.exe"

& $GLSLC blit.vert -o blit_vert.spv
& $GLSLC blit.frag -o blit_frag.spv

& $GLSLC d3d9/fill.vert -o d3d9/fill_vert.spv
& $GLSLC d3d9/fill.frag -o d3d9/fill_frag.spv
& $GLSLC d3d9/tile.vert -o d3d9/tile_vert.spv
& $GLSLC d3d9/tile.frag -o d3d9/tile_frag.spv

# Wait for input.
Write-Host "All jobs finished."
$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
