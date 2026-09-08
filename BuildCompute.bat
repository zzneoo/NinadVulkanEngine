@echo off

REM ============================================================
REM Shader Directory Paths
REM ============================================================

set "shaderCodePath=%~dp0Shaders\Code"
set "shaderBinaryPath=%~dp0Shaders\Binary"
set "shaderLogPath=%~dp0Shaders\Log"

REM ============================================================
REM Input Compute Shader
REM ============================================================

set "computeShader=%shaderCodePath%\%1.comp"

REM ============================================================
REM Vulkan Shader Compiler
REM ============================================================

set "shaderCompiler=C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe"

REM ============================================================
REM Create Required Directories
REM ============================================================

if not exist "%shaderBinaryPath%" (
    mkdir "%shaderBinaryPath%"
)

if not exist "%shaderLogPath%" (
    mkdir "%shaderLogPath%"
)

REM ============================================================
REM Delete Previously Compiled Shader
REM ============================================================

del /Q "%shaderBinaryPath%\%1.comp.spv" 2>nul

REM ============================================================
REM Debug Paths
REM ============================================================

echo Shader Code Path:
echo %shaderCodePath%

echo.
echo Compute Shader:
echo %computeShader%

echo.
echo Output Shader:
echo %shaderBinaryPath%\%1.comp.spv

echo.

REM ============================================================
REM Compile Compute Shader
REM ============================================================

"%shaderCompiler%" -V --target-env vulkan1.4 -I"%shaderCodePath%" -S comp -o "%shaderBinaryPath%\%1.comp.spv" "%computeShader%" > "%shaderLogPath%\%1.csCompileLog.txt" 2>&1

REM ============================================================
REM Exit Batch Script
REM ============================================================

exit /b 0