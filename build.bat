cmake -G "NMake Makefiles" -S . -B .
nmake
#cd ..
#"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.37.32822\bin\Hostx86\x86\link.exe" /nologo @CMakeFiles\craft.dir\objects1.rsp /out:craft.exe /implib:craft.lib /pdb:craft.pdb /version:0.0 /machine:X86 /debug /INCREMENTAL /subsystem:console deps/curl_windows/lib/libcurl.lib Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib opengl32.lib deps/glfw/src/glfw3.lib /MANIFEST /MANIFESTFILE:CMakeFiles\craft.dir/intermediate.manifest CMakeFiles\craft.dir/manifest.res