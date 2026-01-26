all:
#	bcc32c.exe  objs\%.obj /nobjs
	bcc32c.exe *.cpp /I. /DUNICODE /DGLUT_DISABLE_ATEXIT_HACK /nobjs
	copy objs\*.exe .
#%.obj : %.cpp
#	bcc32c.exe  $< /c /I. /DUNICODE /DGLUT_DISABLE_ATEXIT_HACK /nobjs
