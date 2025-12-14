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
Form[] ESLSpells
Form[] SpellstoEdit
int[] LastEffect
Form[] SpellModels
int [] Magnitudes
int [] Areas
int [] Durations
float [] Costs

Function GetArrays()
    EM03SKSEFunctions.ReturnESLEffects(SpellstoEdit, ESLSpells, LastEffect, spellModels, Magnitudes, Areas, Durations, Costs)
EndFunction

Function GetSettings()
    EM03SKSEFunctions.GiveSettings(TomeCostGlobal.GetValueInt(), EffectCost, MustKnowPerk, FullScreen, ShowOgCost, MagExp, DMult, AMult, sliderMax)
EndFunction

Function ApplyArrays(Form[] SpellstoChange, Form[] ESLSpells1,  int[] LastEffect1, Form[] spellModels1, int[] Magnitudes1, int[] Areas1, int[] Durations1, Float[] Costs1)
    SpellstoEdit = Utility.ResizeFormArray(SpellstoEdit, SpellstoChange.Length)
    SpellstoEdit = SpellstoChange
    ESLSpells = ESLSpells1
    LastEffect = LastEffect1
    spellModels = spellModels1
    Magnitudes = Magnitudes1
    Areas = Areas1
    Durations = Durations1
    Costs = Costs1
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


Function CraftTheSpell(string SpellName1, MagicEffect[] SpellEffects, Int[] Magnitude, Int[] Area, Int[] Duration, int School, Float[] Cost, Spell SpellModel, int CostliestEI, bool Overiding, Spell[] SourceSpells)
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
    
    int ESLCount = 0
    bool hasESLEffect = false
    if Overiding
        if(EM03SKSEFunctions.CheckifESLFlagged(CostliestEffect))
            hasESLEffect = true
            If (!ESLSpells)
                ESLSpells = new Form[1]
                SpellstoEdit = new Form[1]
                LastEffect = new int[1]
                SpellModels = new Form[1]
                Magnitudes = new int[1]
                Areas = new int[1]
                Durations = new int[1]
                Costs = new float[1]
            EndIf
            ESLCount = ESLSpells.Length - 1
        endif
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
        MagicEffect mgef = SpellEffects[index]
        int id = mgef.GetFormID()
        if(EM03SKSEFunctions.CheckifESLFlagged(mgef))
            hasESLEffect = true
            If (!ESLSpells)
                ESLSpells = new Form[1]
                SpellstoEdit = new Form[1]
                LastEffect = new int[1]
                SpellModels = new Form[1]
                Magnitudes = new int[1]
                Areas = new int[1]
                Durations = new int[1]
                Costs = new float[1]
            EndIf
            
            ESLCount = ESLSpells.Length
            
            if ESLCount == 350
            Debug.MessageBox("You've reached 350 effects from ESL-Flagged Spells currently being specially saved. As this number increases it can extend load time by seconds and potentially becomes unstable at higher amounts (600+ saved.) You may want to erase/ forget some crafted spells with ESL-Flagged Effects that you no longer use.")
            Endif
            ESLSpells = Utility.ResizeFormArray(ESLSpells, ESLCount + 1)
            Magnitudes = Utility.ResizeintArray(Magnitudes, ESLCount + 1)
            Areas = Utility.ResizeintArray(Areas, ESLCount + 1)
            Durations = Utility.ResizeintArray(Durations, ESLCount + 1)
            Costs = Utility.ResizefloatArray(Costs, ESLCount + 1)
            
            int PlacetoPutBack = ESLSpells.Length - 2
            ESLSpells[PlacetoPutBack] = mgef
            Magnitudes[PlacetoPutBack] = Magnitude[index]
            Areas[PlacetoPutBack] = Area[index]
            Durations[PlacetoPutBack] = Duration[index]
            Costs[PlacetoPutBack] = cost[index]
        endif
        DynamicPersistentForms.AddMagicEffect(NewSpell, SpellEffects[index], Magnitude[index], Area[index], Duration[index], Cost[index])
        index+= 1
    EndWhile

    CostliestEffect = NewSpell.GetNthEffectMagicEffect(CostliestEI)
    DynamicPersistentForms.SetSpellCastingPerk(NewSpell, GetRigtPerk(CostliestEffect.GetAssociatedSkill(), CostliestEffect.GetSkillLevel()))
    Game.GetPlayer().AddSpell(NewSpell)
    
   If (hasESLEffect)
    int SavedCount = SpellstoEdit.Length
    SpellstoEdit = Utility.ResizeFormArray(SpellstoEdit, SavedCount + 1)
    SpellModels = Utility.ResizeFormArray(SpellModels, SavedCount + 1)
    LastEffect = Utility.ResizeIntArray(LastEffect, SavedCount + 1)
    ;For some fucking reason setting the form to the spell wouldn't work if a spell was just erased, it would stay none which was not the case any other time, 
    ;but when I set the element to it after resizing the arrays, it worked. It's called PlacetoSetAgain cause earlier I was setting it both before and after resizing.
    int PlacetoSetAgain = SpellstoEdit.Length - 2
    SpellstoEdit[PlacetoSetAgain] = NewSpell
    LastEffect[PlacetoSetAgain] = ESLCount - 1
    SpellModels[PlacetoSetAgain] = SpellModel
   EndIf

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
        EM03SKSEFunctions.EraseArrayData(SpellstoEdit, ESLSpells, LastEffect, spellModels, Magnitudes, Areas, Durations, Costs, DelSpell)
        MagicEffect CostliestEffect = DelSpell.GetNthEffectMagicEffect(DelSpell.GetCostliestEffectIndex())
        DynamicPersistentForms.Dispose(DelSpell)
        If (CostliestEffect.GetName() == "EM03ID_")
            DynamicPersistentForms.Dispose(CostliestEffect)
        EndIf
        Debug.Notification("Erased " + DelSpell.GetName())
    UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", "Book Menu")
EndFunction

Function CloseBook()
    UI.InvokeString("HUD Menu", "_global.skse.CloseMenu", "Book Menu")
EndFunction

Function ShowHowManySaved()
    int TrueLength = ESLSpells.length - 1
    If (TrueLength < 1)
        TrueLength = 0
    EndIf
    int TrueLength2 = SpellstoEdit.length - 1
    If (TrueLength2 < 1)
        TrueLength2 = 0
    EndIf
    Debug.Notification("There are " + TrueLength  + " effects from ESL-Flagged Mods and " + TrueLength2 + " spells being specially saved.")
EndFunction