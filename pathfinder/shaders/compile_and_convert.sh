#!/bin/bash

# 1. Clean up and recreate the generated directory
rm -rf "generated"
mkdir -p "generated"

# 2. Set the generator path (Note: No .exe suffix on macOS)
GENERATOR="../../cmake-build-debug/bin/pathfinder_shader_generator"

# 3. Verify the generator exists
if [ ! -f "$GENERATOR" ]; then
    echo "Error: Generator not found at $GENERATOR"
    exit 1
fi

# 4. Compile shaders
$GENERATOR -i blit.vert -o generated/blit_vert.shdbin -t vert
$GENERATOR -i blit.frag -o generated/blit_frag.shdbin -t frag

$GENERATOR -i d3d9/fill.vert -o generated/fill_vert.shdbin -t vert
$GENERATOR -i d3d9/fill.frag -o generated/fill_frag.shdbin -t frag
$GENERATOR -i d3d9/tile.vert -o generated/tile_vert.shdbin -t vert
$GENERATOR -i d3d9/tile.frag -o generated/tile_frag.shdbin -t frag
$GENERATOR -i d3d9/tile_clip_copy.vert -o generated/tile_clip_copy_vert.shdbin -t vert
$GENERATOR -i d3d9/tile_clip_copy.frag -o generated/tile_clip_copy_frag.shdbin -t frag
$GENERATOR -i d3d9/tile_clip_combine.vert -o generated/tile_clip_combine_vert.shdbin -t vert
$GENERATOR -i d3d9/tile_clip_combine.frag -o generated/tile_clip_combine_frag.shdbin -t frag

$GENERATOR -i d3d11/bin.comp -o generated/bin_comp.shdbin -t comp
$GENERATOR -i d3d11/bound.comp -o generated/bound_comp.shdbin -t comp
$GENERATOR -i d3d11/dice.comp -o generated/dice_comp.shdbin -t comp
$GENERATOR -i d3d11/fill.comp -o generated/fill_comp.shdbin -t comp
$GENERATOR -i d3d11/propagate.comp -o generated/propagate_comp.shdbin -t comp
$GENERATOR -i d3d11/sort.comp -o generated/sort_comp.shdbin -t comp
$GENERATOR -i d3d11/tile.comp -o generated/tile_comp.shdbin -t comp

# 5. Copy supporting files
cp "area_lut.png" "generated/"

# 6. Generate headers in the target directory
cd "generated" || exit

python3 ../convert_files_to_header.py shdbin
python3 ../convert_files_to_header.py png

# 7. Remove intermediate files (keeping only .h files)
find . -maxdepth 1 -type f ! -name "*.h" -delete

echo "All jobs finished."
cd ".."