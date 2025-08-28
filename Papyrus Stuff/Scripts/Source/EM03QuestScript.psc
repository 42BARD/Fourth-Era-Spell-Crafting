Scriptname EM03QuestScript extends Quest  

MiscObject Property Gold Auto

Perk[] Property SpellPerks  Auto

GlobalVariable Property TomeCostGlobal  Auto

Book Property TomeOfSC  Auto

SPELL[] Property BaseSpells  Auto

MagicEffect[] Property BaseEffects Auto

int EffectCost = 0
bool MustKnowPerk = true
bool FullScreen = false
bool ShowOgCost = false
float MagExp = 1.1
float DMult = 1.0
float AMult = 15.0
int sliderMax = 100

Form[] LearnedEffects
int EffectCount = 0

Function GetSettings()
    EM03SKSEFunctions.GiveSettings(TomeCostGlobal.GetValueInt(), EffectCost, MustKnowPerk, FullScreen, ShowOgCost, MagExp, DMult, AMult, sliderMax)
EndFunction

Function ApplySettings(int TomeCost1, int EffctCost, bool MustKnowPerk1, bool FullScreen1, bool ShowOgCost1, float ME, float DM, Float AM, int SliderMax1)
    TomeCostGlobal.SetValueInt(TomeCost1)
    UpdateCurrentInstanceGlobal(TomeCostGlobal)
    EffectCost = EffctCost
    MustKnowPerk = MustKnowPerk1
    FullScreen = FullScreen1
    ShowOgCost = ShowOgCost1
    MagExp = ME
    DMult = DM
    AMult = AM
    SliderMax = SliderMax1
    Debug.Notification("Settings Saved")
EndFunction


Function CraftTheSpell(string SpellName1, MagicEffect[] SpellEffects, Int[] Magnitude, Int[] Area, Int[] Duration, int School, Float[] Cost, Spell SpellModel, int CostliestEI, bool Overiding)
    If (EffectCost > 0)
        int i = 0
        int GoldCost = 0
        While (i < Cost.Length)
            GoldCost = GoldCost + (EffectCost * Cost[i] as int)
            i += 1
        EndWhile

        Actor Player = Game.GetPlayer()
        If (Player.GetItemCount(Gold) >= GoldCost)
            Player.RemoveItem(Gold, GoldCost)
        Else
            Debug.MessageBox("You don't have enouph gold to craft that spell!")
            Return
        EndIf
    EndIf
    
    MagicEffect CostliestEffect = SpellModel.GetNthEffectMagicEffect(SpellModel.GetCostliestEffectIndex())
        bool isHostile = False ;Hostility of spell should be determined by effects added but its only by the effects there when the new spell is created by dpf
        int index2 = 0
        While (index2 < SpellEffects.Length) ;Determine if at least one effect is Hostile
            if SpellEffects[index2].IsEffectFlagSet(0x00000001)
                isHostile = true
                index2 = SpellEffects.Length
            Endif
                index2 += 1
        EndWhile 
    Spell NewSpell = DynamicPersistentForms.Create(GetRightBaseSpell(CostliestEffect.GetCastingType(), CostliestEffect.GetDeliveryType(), isHostile)) as Spell
    DynamicPersistentForms.CopyAppearance(SpellModel, NewSpell)
    NewSpell.SetName(SpellName1)
    DynamicPersistentForms.ClearMagicEffects(NewSpell)
    
    if Overiding
        int HighestSkillLvl = 0
        If (SpellEffects[CostliestEI].GetAssociatedSkill() != "")
            HighestSkillLvl = SpellEffects[CostliestEI].GetSkillLevel()
        Else
            HighestSkillLvl = CostliestEffect.GetSkillLevel()
        EndIf 
        int BaseEffectIndex = (school*5) + (HighestSkillLvl / 25)
        MagicEffect NewEffect = DynamicPersistentForms.Create(BaseEffects[BaseEffectIndex]) as MagicEffect
        DynamicPersistentForms.CopyAppearance(CostliestEffect, NewEffect)
        EM03SKSEFunctions.SetEffectStuff(NewEffect)
        NewEffect.SetName("EM03ID_")
        DynamicPersistentForms.AddMagicEffect(NewSpell, NewEffect, 0, Area[CostliestEI], 0, Cost[CostliestEI])
        Cost[CostliestEI] = 0
        CostliestEI = 0
    endif

    int index = 0
    While (index < SpellEffects.Length)
        DynamicPersistentForms.AddMagicEffect(NewSpell, SpellEffects[index], Magnitude[index], Area[index], Duration[index], Cost[index])
        index+= 1
    EndWhile

    CostliestEffect = NewSpell.GetNthEffectMagicEffect(CostliestEI)
    DynamicPersistentForms.SetSpellCastingPerk(NewSpell, GetRigtPerk(CostliestEffect.GetAssociatedSkill(), CostliestEffect.GetSkillLevel()))
    Game.GetPlayer().AddSpell(NewSpell)
    UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", "Book Menu")
    Game.DisablePlayerControls()
    Utility.Wait(0.1)
    Game.EnablePlayerControls()
EndFunction

Function LearnTheEffect(Potion thePotion)
    If (!LearnedEffects)
        LearnedEffects = new Form[1]
    EndIf
   
    int index = 0
    while index< thePotion.GetNumEffects()
        MagicEffect theEffect =  thePotion.GetNthEffectMagicEffect(index)
        If (!theEffect)
            return
        EndIf
        int index2 = 0
        bool alreadyKnow = false
        While (index2 < EffectCount)
            If (LearnedEffects[index2] == theEffect)
                debug.Notification("You already know " + theEffect.GetName())
                alreadyKnow = true
                index2 = EffectCount
            EndIf
            index2 += 1
        EndWhile

        If (EffectCount == 150)
            Debug.MessageBox("You have reached 150 known alchemical effects. While this much should be ok, you may want to start slowing down how many you learn, as getting too high may cause performance issues.")
        EndIf
        If (!alreadyKnow)
            LearnedEffects[EffectCount] = theEffect
            EffectCount += 1
            LearnedEffects = Utility.ResizeFormArray(LearnedEffects, EffectCount + 1)
            Debug.Notification("Learned " + theEffect.GetName())
        EndIf
        index += 1
    endwhile
EndFunction

Form[] Function GetLearnedEffects()
    Return LearnedEffects
EndFunction

Perk Function GetRigtPerk(String Skill, int SkillLvl)
    int index1
    If (Skill == "Alteration")
        index1 = 0
    ElseIf (Skill == "Conjuration")
        index1 = 5
    ElseIf (Skill == "Destruction")
        index1 = 10
    ElseIf (Skill == "Illusion")
        index1 = 15
    ElseIf (Skill == "Restoration")
        index1 = 20
    EndIf

    SkillLvl = SkillLvl / 25
    index1 = index1 + SkillLvl
    Return SpellPerks[index1]
EndFunction

Function AddSCTome()
    Game.GetPlayer().AddItem(TomeOfSC, 1)
EndFunction

Spell Function GetRightBaseSpell(int CastType, int Delivery, bool Hostile)
    CastType = ((CastType - 1) * 5) + Delivery
    if Hostile
        CastType = CastType + 10
    Endif
    Return BaseSpells[CastType]
EndFunction

Function EraseSpell(Spell DelSpell)
    Actor Deleter = Game.GetPlayer()
    If Deleter.GetEquippedSpell(0) == DelSpell
        Deleter.UnequipSpell(DelSpell as spell, 0)
    Endif
    If Deleter.GetEquippedSpell(1) == DelSpell
        Deleter.UnequipSpell(DelSpell as spell, 1)
    EndIf
    If (Deleter.HasSpell(DelSpell))
        Deleter.RemoveSpell(DelSpell)
    EndIf
    If (Deleter.HasSpell(DelSpell))
        Debug.MessageBox("Could not Remove the spell (Know that starting spells can not be removed)")
    else
        MagicEffect CostliestEffect = DelSpell.GetNthEffectMagicEffect(DelSpell.GetCostliestEffectIndex())
        DynamicPersistentForms.Dispose(DelSpell)
        If (CostliestEffect.GetName() == "EM03ID_")
            DynamicPersistentForms.Dispose(CostliestEffect)
        EndIf
        Debug.Notification("Erased " + DelSpell.GetName())
    EndIf
    UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", "Book Menu")
EndFunction

Function CloseBook()
    UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", "Book Menu")
EndFunction

String Function TestingStuff()
    Return "Settings 1"
EndFunction