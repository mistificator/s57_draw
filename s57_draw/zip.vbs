Set objArgs = WScript.Arguments 
zipFile = objArgs(0) 
sFolder = objArgs(1) 
   With CreateObject("Scripting.FileSystemObject") 
       zipFile = .GetAbsolutePathName(zipFile) 
       sFolder = .GetAbsolutePathName(sFolder) 
       With .CreateTextFile(zipFile, True) 
           .Write Chr(80) & Chr(75) & Chr(5) & Chr(6) & String(18, chr(0)) 
       End With 
   End With 
   With CreateObject("Shell.Application") 
       .NameSpace(zipFile).CopyHere .NameSpace(sFolder).Items 
       Do Until .NameSpace(zipFile).Items.Count = _ 
                .NameSpace(sFolder).Items.Count - 1 ' script itself LOL 
           WScript.Sleep 1000  
       Loop 
   End With 
