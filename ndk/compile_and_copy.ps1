# Compile as static lib using NDK.
# ------------------------
# Enter the jni directory.
Set-Location "./jni"

# Compile using 4 threads.
ndk-build -j4
# ------------------------

# Go back to current directory.
Set-Location ".."

# Copy headers and lib.
python copy_to_android_demo.py

# Wait for input.
Write-Host "All jobs finished."
$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
