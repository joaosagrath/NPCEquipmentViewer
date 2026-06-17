Scriptname NPCEquipmentViewerMenu Hidden

Int Function ShowEquipmentMenu() Global
    UIListMenu listMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu

    If !listMenu
        Return -2
    EndIf

    listMenu.ResetMenu()

    Int entryCount = NPCEquipmentViewerNative.GetEntryCount()
    If entryCount <= 0
        Return -2
    EndIf

    Int index = 0
    While index < entryCount
        listMenu.AddEntryItem(NPCEquipmentViewerNative.GetEntryText(index))
        index += 1
    EndWhile

    listMenu.OpenMenu(Game.GetPlayer())
    Return listMenu.GetResultInt()
EndFunction
