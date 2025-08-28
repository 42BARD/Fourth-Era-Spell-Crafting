Scriptname EM03LearnEffectsScript extends ActiveMagicEffect  

EM03QuestScript Property QuestScriptRef Auto

Event OnItemAdded(Form akBaseItem, int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)
    If (UI.IsMenuOpen("Crafting Menu") && akBaseItem.GetType() == 46)
        QuestScriptRef.LearnTheEffect(akBaseItem as Potion)
    EndIf
EndEvent

Event OnSpellCast(Form akSpell)
    Spell theSpell = akSpell as Spell
    int index = 0
    While (index < theSpell.GetNumEffects())
        MagicEffect FstEffect = theSpell.GetNthEffectMagicEffect(index)
        bool isit = FstEffect.IsEffectFlagSet(0x00000001)
        index += 1
    Debug.MessageBox(FstEffect.GetName() + isit)
    EndWhile
    Perk thePerk = theSpell.GetPerk()
    Debug.MessageBox(thePerk + thePerk.GetName())
    ;/Spell NewSpell = DynamicPersistentForms.Create(theSpell) as Spell
    NewSpell.SetName(2)
    Game.GetPlayer().AddSpell(NewSpell)/;
EndEvent