import os
import shutil
from enum import Enum


# Remove source files.
def delete_cpp_files(path):
    for root, dirs, files in os.walk(path):
        for f in files:
            if f.lower().endswith(".cpp"):
                os.remove(os.path.join(root, f))


class GpuApi(Enum):
    GLES = "gles"
    VULKAN = "vulkan"


# Change API here.
target_gpu_api = GpuApi.GLES

print("Target GPU API:", target_gpu_api.name)

# Go into the jni directory.
os.system("cd jni")

# Compile.
os.system("ndk-build -j4 TARGET_GPU_API=" + str(target_gpu_api.value))

# Go back.
os.system("cd ..")

print("Finished compiling")

# Set Android src path.
android_src_path = "../demo/android-" + str(target_gpu_api.value) + "/app/src/main/cpp"

src_dirs = ["/demo/common", "/src", "/lib", "/third_party", "/assets"]
dst_dirs = ["/demo/common", "/src", "/lib", "/third_party", "/../assets"]

for i in range(len(src_dirs)):
    src_dir = src_dirs[i]
    dst_dir = android_src_path + dst_dirs[i]

    # Delete existing src, third_party and lib folders in the Android demo.
    if os.path.exists(dst_dir):
        shutil.rmtree(dst_dir)

    # Copy new headers and sources.
    if src_dir != "/lib":
        shutil.copytree(".." + src_dir, dst_dir)

    # Except for the demo directory, delete source files in other directories.
    if src_dir != "/demo/common":
        delete_cpp_files(dst_dir)

# Copy libraries.
shutil.copytree("./obj/local/arm64-v8a", android_src_path + "/lib/arm64-v8a")
shutil.copytree("./obj/local/armeabi-v7a", android_src_path + "/lib/armeabi-v7a")

print("Finished copying")
