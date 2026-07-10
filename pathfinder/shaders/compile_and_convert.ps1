Remove-Item -Path "generated" -Recurse

New-Item -Path "generated" -ItemType Directory

New-Variable -Name "GENERATOR" -Visibility Public -Value "../../cmake-build-debug/bin/pathfinder_shader_generator.exe"

# Compile shaders.
& $GENERATOR -i blit.vert -o generated/blit_vert.shdbin -t vert
& $GENERATOR -i blit.frag -o generated/blit_frag.shdbin -t frag

& $GENERATOR -i d3d9/fill.vert -o generated/fill_vert.shdbin -t vert
& $GENERATOR -i d3d9/fill.frag -o generated/fill_frag.shdbin -t frag
& $GENERATOR -i d3d9/tile.vert -o generated/tile_vert.shdbin -t vert
& $GENERATOR -i d3d9/tile.frag -o generated/tile_frag.shdbin -t frag
& $GENERATOR -i d3d9/tile_clip_copy.vert -o generated/tile_clip_copy_vert.shdbin -t vert
& $GENERATOR -i d3d9/tile_clip_copy.frag -o generated/tile_clip_copy_frag.shdbin -t frag
& $GENERATOR -i d3d9/tile_clip_combine.vert -o generated/tile_clip_combine_vert.shdbin -t vert
& $GENERATOR -i d3d9/tile_clip_combine.frag -o generated/tile_clip_combine_frag.shdbin -t frag

& $GENERATOR -i d3d11/bin.comp -o generated/bin_comp.shdbin -t comp
& $GENERATOR -i d3d11/bound.comp -o generated/bound_comp.shdbin -t comp
& $GENERATOR -i d3d11/dice.comp -o generated/dice_comp.shdbin -t comp
& $GENERATOR -i d3d11/fill.comp -o generated/fill_comp.shdbin -t comp
& $GENERATOR -i d3d11/propagate.comp -o generated/propagate_comp.shdbin -t comp
& $GENERATOR -i d3d11/sort.comp -o generated/sort_comp.shdbin -t comp
& $GENERATOR -i d3d11/tile.comp -o generated/tile_comp.shdbin -t comp

Copy-Item "area_lut.png" "generated"

Set-Location "generated"

# Generate headers.
python ../convert_files_to_header.py shdbin
python ../convert_files_to_header.py png

# Remove intermediate files.
Get-ChildItem -Recurse -File | Where { ($_.Extension -ne ".h") } | Remove-Item

# Wait for input.
Write-Host "All jobs finished."
Set-Location ".."
# $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
