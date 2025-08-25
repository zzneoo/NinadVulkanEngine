REM cd C:\Users\Ninad\source\repos\BlueScreen\
del %1.vert.spv 
del %1.frag.spv
set vertexShader=%~dp0%1.vert
set fragmentShader=%~dp0%1.frag
set shaderCompiler=C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe

%shaderCompiler% -V -o %vertexShader%.spv %vertexShader% > vsCompileLog.txt
%shaderCompiler% -V -o %fragmentShader%.spv %fragmentShader% > fsCompileLog.txt