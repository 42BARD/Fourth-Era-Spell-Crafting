Scriptname EM03LearnEffectsScript extends ActiveMagicEffect  

EM03QuestScript Property QuestScriptRef Auto

Event OnItemAdded(Form akBaseItem, int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)
    If (UI.IsMenuOpen("Crafting Menu") && akBaseItem.GetType() == 46)
        EM03SKSEFunctions.AddEffectsInSCMenu(akBaseItem)
    EndIf
EndEvent

Event OnSpellCast(Form akSpell)
    utility.wait(0.1)
    EM03SKSEFunctions.AddEffectsInSCMenu(akSpell)
EndEvent