require("os")

workspace "vkvoxel"
   configurations { "Release", "Debug" }

   project "vkvoxel"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++23"
   architecture "x86_64"

   local vulkan_sdk = os.getenv("VULKAN_SDK")
   if not vulkan_sdk or vulkan_sdk == "" then
      error("environment variable 'VULKAN_SDK' is either not set or is empty!")
   end

   prebuildcommands {
      { vulkan_sdk .. "/bin/slangc ./shaders/shader.slang -target spirv -profile spirv_1_6 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o ./shaders/shader.slang.spv" }
   }

   includedirs { vulkan_sdk .. "/include" }
   files { "src/*.hpp", "src/*.cpp" }
   libdirs { vulkan_sdk .. "/lib" }
   links { "SDL3", "volk" }

   filter { "configurations:Release" }
      defines { "NDEBUG" }
      symbols "Off"
      optimize "On"
      linktimeoptimization "On"

   filter { "configurations:Debug" }
      defines { "DEBUG" }
      warnings "Extra"
      optimize "Off"
      linktimeoptimization "Off"
      sanitize { "Address" }
      if os.host() == "windows" then
      symbols "Default"
      else
      symbols "On"
      end

