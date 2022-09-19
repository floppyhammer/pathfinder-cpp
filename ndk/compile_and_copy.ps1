# OpenGL ES.
# ------------------------
# Enter the jni directory.
Set-Location "./jni"

# Compile as static lib using 4 threads.
ndk-build -j4

# Go back to current directory.
Set-Location ".."

# Copy headers and lib.
python copy_to_android_demo.py android
# ------------------------

# Vulkan.
# ------------------------
Set-Location "./jni"
ndk-build -j4 TARGET_GPU_API=vulkan
Set-Location ".."
python copy_to_android_demo.py android-vulkan
# ------------------------

# Wait for input.
Write-Host "All jobs finished."
$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
