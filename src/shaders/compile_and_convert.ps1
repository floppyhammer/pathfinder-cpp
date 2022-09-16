New-Item -Path "generated" -ItemType Directory

New-Variable -Name "GLSLC" -Visibility Public -Value $env:VULKAN_SDK"/Bin/glslc.exe"

& $GLSLC blit.vert -o generated/blit_vert.spv
& $GLSLC blit.frag -o generated/blit_frag.spv

& $GLSLC d3d9/fill.vert -o generated/fill_vert.spv
& $GLSLC d3d9/fill.frag -o generated/fill_frag.spv
& $GLSLC d3d9/tile.vert -o generated/tile_vert.spv
& $GLSLC d3d9/tile.frag -o generated/tile_frag.spv

& $GLSLC d3d11/bin.comp -o generated/bin_comp.spv
& $GLSLC d3d11/bound.comp -o generated/bound_comp.spv
& $GLSLC d3d11/dice.comp -o generated/dice_comp.spv
& $GLSLC d3d11/fill.comp -o generated/fill_comp.spv
& $GLSLC d3d11/propagate.comp -o generated/propagate_comp.spv
& $GLSLC d3d11/sort.comp -o generated/sort_comp.spv
& $GLSLC d3d11/tile.comp -o generated/tile_comp.spv

Copy-Item "area_lut.png" "generated"
Copy-Item "blit.frag" "generated"
Copy-Item "blit.vert" "generated"
Copy-Item "d3d9/*.*" "generated"
Copy-Item "d3d11/*.*" "generated"

Set-Location "generated"

python ../convert_files_to_header.py vert
python ../convert_files_to_header.py frag
python ../convert_files_to_header.py comp
python ../convert_files_to_header.py spv
python ../convert_files_to_header.py png

# Wait for input.
Write-Host "All jobs finished."
$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
