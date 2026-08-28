del %1.comp.spv
set computeShader=%~dp0%1.comp
set "shaderIncludePath=%~dp0Shaders\Code"
set "shaderCompiler=C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe"

%shaderCompiler% -V --target-env vulkan1.4 -I"%shaderIncludePath%" -S comp -o "%computeShader%.spv" "%computeShader%" > "%~dp0csCompileLog.txt"