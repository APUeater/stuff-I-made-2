Dim result
result = MsgBox("pipis?", vbYesNo + vbQuestion, "pipis")

If result = vbYes Then 
	MsgBox "I GRANT YOU [[FREEDOM]]"
Else
	MsgBox "[[BIG SHOT]]"
	While True
		WScript.Echo "BIG SHOT"
	Wend
End If
