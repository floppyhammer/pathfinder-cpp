import os
import sys
import platform

deps = [
    ('https://github.com/KhronosGroup/glslang.git', 'glslang', 'vulkan-sdk-1.4.321'),
    ('https://github.com/KhronosGroup/SPIRV-Cross.git', 'spirv-cross', 'vulkan-sdk-1.4.321'),
    ('https://github.com/CLIUtils/CLI11.git', 'CLI11', 'v2.3.2'),  # For shader builder only.
]


def download_deps():
    for url, name, branch in dev_deps:
        if not os.path.exists('third_party/' + name):
            cmd = 'git clone -b ' + branch + ' ' + url + ' third_party/' + name

            print(cmd)
            os.system(cmd)

            # Get glslang deps
            if name == "glslang":
                os.chdir('./3rd/glslang')
                os.system(python_alias + ' update_glslang_sources.py')
                os.chdir('../..')


if __name__ == "__main__":
    python_alias = "python3"

    if platform.system() == 'Windows':
        python_alias = "python"

    download_deps()
