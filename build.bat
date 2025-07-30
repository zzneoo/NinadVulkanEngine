cd C:\Users\Ninad\source\repos\BlueScreen\
del %1.vert.spv 
del %1.frag.spv
glslangValidator.exe -V -o %1.vert.spv "C:\Users\Ninad\source\repos\BlueScreen\%1.vert" > vsCompileLog.txt
glslangValidator.exe -V -o %1.frag.spv "C:\Users\Ninad\source\repos\BlueScreen\%1.frag" > fsCompileLog.txt