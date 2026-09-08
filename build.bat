@echo off

REM ============================================================
REM Shader Directory Paths
REM ============================================================

REM Source directory containing .vert and .frag shader files
set "shaderCodePath=%~dp0Shaders\Code"

REM Output directory for compiled SPIR-V (.spv) files
set "shaderBinaryPath=%~dp0Shaders\Binary"

REM Directory for shader compilation log files
set "shaderLogPath=%~dp0Shaders\Log"

REM ============================================================
REM Input Shader Files
REM ============================================================

REM %1 is the shader name passed to this batch file.
REM Example: build.bat PBR
REM Vertex shader:   Shaders\Code\PBR.vert
REM Fragment shader: Shaders\Code\PBR.frag

set "vertexShader=%shaderCodePath%\%1.vert"
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

REM Delete old vertex SPIR-V file
del /Q "%shaderBinaryPath%\%1.vert.spv" 2>nul

REM Delete old fragment SPIR-V file
del /Q "%shaderBinaryPath%\%1.frag.spv" 2>nul

REM ============================================================
REM Compile Vertex Shader
REM ============================================================

"%shaderCompiler%" -V --target-env vulkan1.4 -I"%shaderCodePath%" -o "%shaderBinaryPath%\%1.vert.spv" "%vertexShader%" > "%shaderLogPath%\%1.vsCompileLog.txt" 2>&1

REM ============================================================
REM Compile Fragment Shader
REM ============================================================

"%shaderCompiler%" -V --target-env vulkan1.4 -I"%shaderCodePath%" -o "%shaderBinaryPath%\%1.frag.spv" "%fragmentShader%" > "%shaderLogPath%\%1.fsCompileLog.txt" 2>&1

REM ============================================================
REM Exit Batch Script
REM ============================================================

exit /b 0
