# C:/VulkanSDK/1.3.204.0/Bin/glslc.exe blit.vert -o blit_vert.spv
# C:/VulkanSDK/1.3.204.0/Bin/glslc.exe blit.frag -o blit_frag.spv

C:/VulkanSDK/1.3.204.0/Bin/glslc.exe d3d9/fill.vert -o d3d9/fill_vert.spv
C:/VulkanSDK/1.3.204.0/Bin/glslc.exe d3d9/fill.frag -o d3d9/fill_frag.spv
C:/VulkanSDK/1.3.204.0/Bin/glslc.exe d3d9/tile.vert -o d3d9/tile_vert.spv
C:/VulkanSDK/1.3.204.0/Bin/glslc.exe d3d9/tile.frag -o d3d9/tile_frag.spv

# Wait for input.
Write-Host "All jobs finished."
$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
