
cl /std:c++20 /EHsc ^
/DIMATH_HALF_NO_LOOKUP_TABLE ^
/I E:\Packages\vcpkg\installed\x64-windows\include ^
main.cpp ^
/link ^
/LIBPATH:E:\Packages\vcpkg\installed\x64-windows\lib ^
openvdb.lib ^
Imath-3_2.lib ^
DirectXTex.lib ^
/OUT:VDBToDDS.exe