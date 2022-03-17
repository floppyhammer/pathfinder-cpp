import os
import shutil

def delete_cpp_files(path):
    for root, dirs, files in os.walk(path):
        for f in files:
            exts = ('.cpp')
            if f.lower().endswith(exts):
                os.remove(os.path.join(root, f))


# Set Android src path.
android_src_path = "../demo/android/app/src/main/cpp"

# Delete existing src, third_party and lib folders in the Android demo.
if os.path.exists(android_src_path + "/demo/common"):
    shutil.rmtree(android_src_path + "/demo/common")
if os.path.exists(android_src_path + "/src"):
    shutil.rmtree(android_src_path + "/src")
if os.path.exists(android_src_path + "/lib"):
    shutil.rmtree(android_src_path + "/lib")
if os.path.exists(android_src_path + "/third_party"):
    shutil.rmtree(android_src_path + "/third_party")

# Copy new headers and sources.
shutil.copytree("../demo/common", android_src_path + "/demo/common")
shutil.copytree("../src", android_src_path + "/src")
shutil.copytree("../third_party", android_src_path + "/third_party")

# Except for the demo directory, delete sources in other directories.
delete_cpp_files(android_src_path + "/src")
delete_cpp_files(android_src_path + "/third_party")

# Copy libraries.
shutil.copytree("./obj/local/arm64-v8a", android_src_path + "/lib/arm64-v8a")
shutil.copytree("./obj/local/armeabi-v7a", android_src_path + "/lib/armeabi-v7a")
