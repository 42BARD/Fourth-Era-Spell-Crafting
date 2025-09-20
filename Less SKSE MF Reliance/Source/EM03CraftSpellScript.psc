Scriptname EM03CraftSpellScript extends ObjectReference  

EM03QuestScript Property QuestScriptRef Auto 

Message Property ChoseSpellOrNot  Auto  
;Many properties and functions have changed their purpose over the course of development, so that the orginial name is no longer accurate, above is one such case
Quest Property SCScriptQuest  Auto  

SPELL Property LearnEffectsSpell  Auto  

Message Property LearnEffectsBox  Auto  

Event OnRead()
    Utility.WaitMenuMode(1 as float)
    int choice = ChoseSpellOrNot.Show()
    If choice == 0
        QuestScriptRef.GetSettings()
        EM03SKSEFunctions.OpenSpellCraftMenu(SCScriptQuest, QuestScriptRef.GetLearnedEffects(), LearnEffectsSpell)

    Elseif(choice == 1);Erase and Forget a Spell
        EM03SKSEFunctions.OpenChoseSpellMenu(SCScriptQuest, LearnEffectsSpell)

    ElseIf (choice == 2);Learn new Magic Effect
        Game.GetPlayer().AddSpell(LearnEffectsSpell, false)
        UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", "Book Menu")
        Game.DisablePlayerControls()
        Utility.Wait(0.1)
        Game.EnablePlayerControls()

        int QuestStage = SCScriptQuest.GetStage() 
        If (QuestStage < 20 && QuestStage != 5)
            If (LearnEffectsBox.Show() == 1)
                If (QuestStage < 5)
                    SCScriptQuest.SetStage(5)
                ElseIf (QuestStage == 5)
                    SCScriptQuest.SetStage(20)
                Else ;should be just when QuestStage > 5
                    SCScriptQuest.SetStage(20)
                EndIf
            EndIf
        EndIf
        ;return

    ElseIf (choice == 3);Stop Learning new Effects.
        Game.GetPlayer().RemoveSpell(LearnEffectsSpell)
        UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", "Book Menu")
        ;return
    ElseIf(choice == 4);Cancel
        UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", "Book Menu")
        ;return
    EndIf  
endEvent


