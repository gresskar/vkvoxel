require("os")

workspace "vkvoxel"
   configurations { "Debug", "Release" }

project "vkvoxel"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++23"

   --local vulkan_sdk = os.getenv("VULKAN_SDK")
   --if not vulkan_sdk or vulkan_sdk == "" then
      --error("environment variable 'VULKAN_SDK' is either not set or is empty!")
   --end

   prebuildcommands {
      "{ECHO} compiling shaders...",
      "slangc shaders/shader.slang -target spirv -profile spirv_1_6 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o shaders/shader.slang.spv"
      --{ vulkan_sdk .. "bin/slangc shader.slang -target spirv -profile spirv_1_6 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o shader.slang.spv" }
   }

   --buildoptions { "-pipe" }
   --includedirs { vulkan_sdk .. "/include" }
   files { "src/*.hpp", "src/*.cpp" }
   --libdirs { vulkan_sdk .. "/lib" }
   links { "SDL3" }

   filter { "configurations:Debug" }
      defines { "DEBUG" }
      warnings "Extra"
      symbols "On"
      optimize "Off"
      linktimeoptimization "Off"
      sanitize { "Address" }

   filter { "configurations:Release" }
      defines { "NDEBUG" }
      symbols "Off"
      optimize "On"
      linktimeoptimization "On"
      