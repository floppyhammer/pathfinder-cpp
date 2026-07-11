# Pathfinder C++

[![License](https://img.shields.io/badge/license-MIT%2FApache--2.0-blue.svg)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-14-green.svg)](https://en.wikipedia.org/wiki/C%2B%2B14)

A high-performance, GPU-accelerated 2D vector graphics library, ported from the original Rust implementation of [Pathfinder 3](https://github.com/servo/pathfinder). This version brings the power of massivly parallel vector rasterization to the C++ ecosystem, with native **Vulkan**, **OpenGL**, and **Metal** backends.

## 🚀 Key Features

*   **Massively Parallel Rasterization**: Leverages the GPU to rasterize complex vector scenes at high speeds.
*   **Dual Rendering Strategies**:
    *   **Hybrid Mode**: CPU-assisted binning with hardware rasterization. Optimized for mobile devices and static path caching.
    *   **GPU-Driven Mode**: Full compute-shader-based pipeline. Minimizes CPU overhead by moving binning and tiling entirely to the GPU.
*   **Advanced Effects**: Native support for shadows, blurs, gradients, and complex clipping paths.
*   **Cross-Platform**: Designed for Windows, Linux, macOS, Android, and Web (Wasm).

## 🛠 Supported Backends

| Backend | Hybrid Mode | GPU-Driven Mode | Notes |
| :--- | :---: | :---: | :--- |
| **Vulkan** | ✅ | ✅ | Best performance |
| **OpenGL 4.3+** | ✅ | ✅ | Desktop standard |
| **OpenGL ES 3.1+** | ✅ | ✅* | *Limited by Storage Image access on some ES devices |
| **OpenGL ES 3.0** | ✅ | ❌ | Compatibility mode |
| **WebGL2** | ✅ | ❌ | Web target |

## 📦 Getting Started

### Prerequisites
*   C++14 compatible compiler.
*   CMake 3.15+.
*   Vulkan SDK (if building with Vulkan support or regenerating shaders).

### Building the Library
```bash
git clone --recursive https://github.com/your-repo/pathfinder-cpp.git
cd pathfinder-cpp
mkdir build && cd build
cmake ..
make
```

## 🎮 Demos

### Windows / Linux / macOS
The native demo uses a common application framework.
1.  Initialize submodules: `git submodule update --init --recursive`.
2.  Open the project in your favorite IDE (CLion, VS) and run the `demo` target.

### Android
1.  Copy `assets` into `demo/android/app/src/main`.
2.  Open `demo/android` in Android Studio.
3.  Deploy to a device supporting GLES 3.0+.

### Web (Emscripten)
1.  Set up the Emscripten environment.
2.  Run `./build_wasm.sh`.
3.  Run `./demo/web/serve.sh` and visit `http://127.0.0.1:8000`.

## 📜 Acknowledgments
This project is a port of the [Pathfinder](https://github.com/servo/pathfinder) engine developed by the Servo project.
