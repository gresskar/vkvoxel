# VKVoxel

Voxel-engine written in C++ and Vulkan

## 1. Dependencies

### 1.1. Windows

1. Install [Visual Studio 2026](https://visualstudio.microsoft.com/downloads/)
    - Enable `Desktop development with C++` under Workloads
    - Enable `Git for Windows` under Individual Components

2. Install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
    - Enable all components *except* `ARM64 binaries for cross compiling`
    - Make sure `VULKAN_SDK` is in your Environment Table after installation

3. Download [Premake](https://premake.github.io/download)
    - It doesn't matter where you extract it, just make sure it's in `$PATH`

### 1.2. Linux

| Dependency | Ubuntu pkg | Fedora pkg | Arch pkg | Gentoo pkg | Purpose |
|------------|------------|------------|----------|------------|---------|
| Git | `git` | `git` | `git` | `dev-vcs/git` | Version control system |
| Premake5 | N/A | `premake` | `premake` | `dev-util/premake`| Build system generator |
| Ninja | `ninja` | `ninja` | `ninja` | `dev-build/ninja` | Build system |
| GCC | `gcc` | `gcc` | `gcc` | `sys-devel/gcc` | C compiler |
| G++ | `g++` | `g++` | `gcc` | `sys-devel/gcc` | C++ compiler |
| SDL3 | `libsdl3-dev` | `SDL3-devel` | `sdl3` | `media-libs/libsdl3` | Window & input system |
| Vulkan headers | `libvulkan-dev` | `vulkan-headers` | `vulkan-headers` | `dev-util/vulkan-headers` | Vulkan header files |
| Vulkan layers | `vulkan-validationlayers` | `vulkan-validation-layers` | `vulkan-validation-layers` | `media-libs/vulkan-layers` | Vulkan validation layers |
| Volk dev | `libvulkan-volk-dev` | `vulkan-volk-devel` | `volk` | N/A | Meta loader for Vulkan |
| GLM | `libglm-dev` | `glm-devel` | `glm` | `media-libs/glm` | Mathematics library
| Slangc | N/A | N/A | N/A | N/A | Slang shader compiler (*download **[here](https://github.com/shader-slang/slang/releases)***)

## 2. Building

### 2.1. Windows

Clone the project through Visual Studio

Then generate a Visual Studio project:

```PowerShell
premake5.exe vs2026
```

### 2.2. Linux

```Bash
git clone https://github.com/gresskar/vkvoxel ~/.build/vkvoxel

cd ~/.build/vkvoxel

premake5 ninja

ninja Debug # or `ninja Release`
```

## 3. Graphics debugging

*Assuming project is cloned into `~/.build/vkvoxel`:*

```Bash
SDL_VIDEO_DRIVER=x11 renderdoccmd capture -d ~/.build/vkvoxel -c vkvoxel ~/.build/vkvoxel/bin/Debug/vkvoxel
```

This will launch the program - press `F12` or `PrtSc` to capture a frame.

Finally to debug the frame:

```
qrenderdoc ~/.build/vkvoxel/vkvoxel_frame*.rdc
```
