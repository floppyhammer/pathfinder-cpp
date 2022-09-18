import os
import shutil


def delete_cpp_files(path):
    for root, dirs, files in os.walk(path):
        for f in files:
            # Remove source files.
            if f.lower().endswith(".cpp"):
                os.remove(os.path.join(root, f))


# Set Android src path.
android_src_path = "../demo/android-vulkan/app/src/main/cpp"

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
