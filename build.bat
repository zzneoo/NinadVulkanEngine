REM cd C:\Users\Ninad\source\repos\BlueScreen\

del %1.vert.spv
del %1.frag.spv

set vertexShader=%~dp0%1.vert
set fragmentShader=%~dp0%1.frag

set "shaderIncludePath=%~dp0Shaders\Code"
set "shaderCompiler=C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe"

%shaderCompiler% -V --target-env vulkan1.4 -I"%shaderIncludePath%" -o "%vertexShader%.spv" "%vertexShader%" > "%~dp0vsCompileLog.txt"
%shaderCompiler% -V --target-env vulkan1.4 -I"%shaderIncludePath%" -o "%fragmentShader%.spv" "%fragmentShader%" > "%~dp0fsCompileLog.txt"