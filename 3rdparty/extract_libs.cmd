@del .\ChromeRoatta.zip /Q
@echo detect 7z.exe path
@set zip7zPath=".\7z.exe"
@if not exist %zip7zPath% set zip7zPath="%ProgramFiles%\7-Zip\7z.exe"
@if not exist %zip7zPath% set zip7zPath="%ProgramFiles(x86)%\7-Zip\7z.exe"
@if not exist %zip7zPath% set zip7zPath="7z.exe"
@if not exist %zip7zPath% goto end
@echo using following 7z %zip7zPath%
@call cleanup.cmd
@rem goto finish
@call %zip7zPath% x .\glfw-3.4.bin.WIN64.zip
@call %zip7zPath% x .\glew-2.1.0-win32.zip
@rem %zip7zPath% x .\glad.gl4.6.zip
@call %zip7zPath% x .\glad.zip -oglad
@call %zip7zPath% x .\glm-0.9.9.7.zip
@move glm glm-0.9.9.7
@call %zip7zPath% x .\stb.zip
@goto finish
:end
@echo unsuccessfull finish, probably 7-Zip is missing
:finish