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
| GLM | `libglm-dev` | `glm-devel` | `glm` | `media-libs/glm` | Mathematics library

You must also install the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#linux):

```Bash
sudo curl https://sdk.lunarg.com/sdk/download/1.4.335.0/linux/vulkansdk-linux-x86_64-1.4.335.0.tar.xz -o /tmp/vulkansdk-linux-x86_64-1.4.335.0.tar.xz

sudo mkdir -v /opt/VulkanSDK/

sudo tar -xvf /tmp/vulkansdk-linux-x86_64-1.4.335.0.tar.xz -C /opt/VulkanSDK/
```

Then point the `VULKAN_SDK` environment variable to its location:

```Bash
echo 'VULKAN_SDK="/opt/VulkanSDK/1.4.335.0/x86_64"' | sudo tee -a /etc/environment

source /etc/environment # or `. /etc/environment`
```

Finally add the Vulkan SDK's libraries as well:

```Bash
echo "${VULKAN_SDK}/lib" | sudo tee -a /etc/ld.so.conf.d/vulkan-sdk.conf

sudo ldconfig
```

## 2. Building

### 2.1. Windows

Clone the project through Visual Studio

Then generate a Visual Studio project:

```PowerShell
cd <PATH_TO_PROJECT>

premake5.exe vs2026
```

Now double-click on the `.slnx` file to load the project

### 2.2. Linux

```Bash
git clone https://github.com/gresskar/vkvoxel ~/.build/vkvoxel

cd ~/.build/vkvoxel

premake5 ninja

ninja # or `ninja Debug`
```

## 3. Graphics debugging

*Assuming project is cloned into `~/.build/vkvoxel`:*

```Bash
renderdoccmd capture -d ~/.build/vkvoxel -c vkvoxel ~/.build/vkvoxel/bin/Debug/vkvoxel
```

This will launch the program - press `F12` or `PrtSc` to capture a frame.

Finally to debug the frame:

```
qrenderdoc ~/.build/vkvoxel/vkvoxel_frame*.rdc
```

## 4. FAQ

> *How do I use Clang instead of GCC on Linux?*

Modify the `premake5.lua` file:

Edit `toolset "gcc"` -> `toolset "clang"` under `filter { "platforms:Linux" }`

> *Why is my IDE  giving me `#include` warnings on Linux?*

Your IDE probably isn't aware of the `$VULKAN_SDK` include path

First of all install `clangd` through your package manager

Then install the [clangd](https://open-vsx.org/vscode/item?itemName=llvm-vs-code-extensions.vscode-clangd) extension through VS Code

Finally generate a `compile_commands.json` file with the command: `premake5 ninja && bear -- ninja Debug`

> *How do I debug through VS Code on Linux?*

First of all install `lldb` through your package manager

Then install the [LLDB DAP](https://open-vsx.org/vscode/item?itemName=llvm-vs-code-extensions.lldb-dap) extension through VS Code