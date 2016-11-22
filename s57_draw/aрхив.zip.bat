@for %%* in (.) do set project=%%~nx*
@set datetemp=%date:~-10% 
@Set yyyy=%DATE:~-4%& 
@set mm=%DATE:~3,2%%
@set dd=%DATE:~,2%
@set yyyy_mm_dd=%yyyy%_%mm%_%dd%
@del /f /q .\..\%project%_#%yyyy_mm_dd%.zip
@"c:\Program Files\WinRAR\rar.exe" a -r .\..\%project%_#%yyyy_mm_dd%.zip *.*
