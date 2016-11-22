@del /f /q zip.vbs

@echo Set objArgs = WScript.Arguments >> zip.vbs
@echo zipFile = objArgs(0) >> zip.vbs
@echo sFolder = objArgs(1) >> zip.vbs
@echo    With CreateObject("Scripting.FileSystemObject") >> zip.vbs
@echo        zipFile = .GetAbsolutePathName(zipFile) >> zip.vbs
@echo        sFolder = .GetAbsolutePathName(sFolder) >> zip.vbs
@echo        With .CreateTextFile(zipFile, True) >> zip.vbs
@echo            .Write Chr(80) ^& Chr(75) ^& Chr(5) ^& Chr(6) ^& String(18, chr(0)) >> zip.vbs
@echo        End With >> zip.vbs
@echo    End With >> zip.vbs
@echo    With CreateObject("Shell.Application") >> zip.vbs
@echo        .NameSpace(zipFile).CopyHere .NameSpace(sFolder).Items >> zip.vbs
@echo        Do Until .NameSpace(zipFile).Items.Count = _ >> zip.vbs
@echo                 .NameSpace(sFolder).Items.Count - 1 ' script itself LOL >> zip.vbs
@echo            WScript.Sleep 1000  >> zip.vbs
@echo        Loop >> zip.vbs
@echo    End With >> zip.vbs

@for %%* in (.) do set project=%%~nx*
@set datetemp=%date:~-10% 
@Set yyyy=%DATE:~-4%& 
@set mm=%DATE:~3,2%%
@set dd=%DATE:~,2%
@set yyyy_mm_dd=%yyyy%_%mm%_%dd%
@del /f /q .\..\%project%_#%yyyy_mm_dd%.zip
@cscript.exe zip.vbs ".\..\%project%_#%yyyy_mm_dd%.zip" "%cd%"

pause
@del /f /q zip.vbs
