@echo off

REM ============================================================
REM Shader Directory Paths
REM ============================================================

REM Source directory containing .task, .mesh and .frag files
set "shaderCodePath=%~dp0Shaders\Code"

REM Output directory for compiled SPIR-V (.spv) files
set "shaderBinaryPath=%~dp0Shaders\Binary"

REM Directory for shader compilation log files
set "shaderLogPath=%~dp0Shaders\Log"

REM ============================================================
REM Input Shader Files
REM ============================================================

REM %1 is the shader name passed to this batch file.
REM Example: BuildMesh.bat Meshlet
REM Task shader:     Shaders\Code\Meshlet.task
REM Mesh shader:     Shaders\Code\Meshlet.mesh
REM Fragment shader: Shaders\Code\Meshlet.frag

set "taskShader=%shaderCodePath%\%1.task"
set "meshShader=%shaderCodePath%\%1.mesh"
set "fragmentShader=%shaderCodePath%\%1.frag"

REM ============================================================
REM Vulkan Shader Compiler
REM ============================================================

set "shaderCompiler=C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe"

REM ============================================================
REM Create Required Directories
REM ============================================================

REM Create the Binary directory if it does not exist
if not exist "%shaderBinaryPath%" (
mkdir "%shaderBinaryPath%"
)

REM Create the Log directory if it does not exist
if not exist "%shaderLogPath%" (
mkdir "%shaderLogPath%"
)

REM ============================================================
REM Delete Previously Compiled Shaders
REM ============================================================

REM Delete old task shader SPIR-V file
del /Q "%shaderBinaryPath%\%1.task.spv" 2>nul

REM Delete old mesh shader SPIR-V file
del /Q "%shaderBinaryPath%\%1.mesh.spv" 2>nul

REM Delete old fragment shader SPIR-V file
del /Q "%shaderBinaryPath%\%1.frag.spv" 2>nul

REM ============================================================
REM Compile Task Shader
REM ============================================================

"%shaderCompiler%" -V --target-env vulkan1.4 -I"%shaderCodePath%" -S task -o "%shaderBinaryPath%\%1.task.spv" "%taskShader%" > "%shaderLogPath%\%1.taskCompileLog.txt" 2>&1

REM ============================================================
REM Compile Mesh Shader
REM ============================================================

"%shaderCompiler%" -V --target-env vulkan1.4 -I"%shaderCodePath%" -S mesh -o "%shaderBinaryPath%\%1.mesh.spv" "%meshShader%" > "%shaderLogPath%\%1.meshCompileLog.txt" 2>&1

REM ============================================================
REM Compile Fragment Shader
REM ============================================================

"%shaderCompiler%" -V --target-env vulkan1.4 -I"%shaderCodePath%" -S frag -o "%shaderBinaryPath%\%1.frag.spv" "%fragmentShader%" > "%shaderLogPath%\%1.fragCompileLog.txt" 2>&1

REM ============================================================
REM Exit Batch Script
REM ============================================================

exit /b 0
