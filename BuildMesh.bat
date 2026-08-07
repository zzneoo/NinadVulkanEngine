REM cd C:\Users\Ninad\source\repos\BlueScreen\

del %1.task.spv
del %1.mesh.spv
del %1.frag.spv

set taskShader=%~dp0%1.task
set meshShader=%~dp0%1.mesh
set fragmentShader=%~dp0%1.frag

set shaderCompiler=C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe

%shaderCompiler% --target-env vulkan1.3 -S task -o %taskShader%.spv %taskShader% > taskCompileLog.txt
%shaderCompiler% --target-env vulkan1.3 -S mesh -o %meshShader%.spv %meshShader% > meshCompileLog.txt
%shaderCompiler% --target-env vulkan1.3 -S frag -o %fragmentShader%.spv %fragmentShader% > fragCompileLog.txt
