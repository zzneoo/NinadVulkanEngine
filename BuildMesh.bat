REM cd C:\Users\Ninad\source\repos\BlueScreen\

del %1.task.spv
del %1.mesh.spv
del %1.frag.spv

set taskShader=%~dp0%1.task
set meshShader=%~dp0%1.mesh
set fragmentShader=%~dp0%1.frag


set "shaderIncludePath=%~dp0Shaders\Code"
set "shaderCompiler=C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe"

%shaderCompiler% -V --target-env vulkan1.4 -I"%shaderIncludePath%" -S task -o "%taskShader%.spv" "%taskShader%" > "%~dp0taskCompileLog.txt"
%shaderCompiler% -V --target-env vulkan1.4 -I"%shaderIncludePath%" -S mesh -o "%meshShader%.spv" "%meshShader%" > "%~dp0meshCompileLog.txt"
%shaderCompiler% -V --target-env vulkan1.4 -I"%shaderIncludePath%" -S frag -o "%fragmentShader%.spv" "%fragmentShader%" > "%~dp0fragCompileLog.txt"
