#include "UI.h"
#include<cmath>
#include<algorithm>
#include<string_view>
#include <spdlog/sinks/basic_file_sink.h>
#include <string>
#include <sstream>

namespace logger = SKSE::log;

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::err);
    spdlog::flush_on(spdlog::level::err);
}


void CallPapyrusFunction(const char* FunctionName, RE::BSScript::IFunctionArguments* args) {
    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        logger::error("VM not found");
        return;
    }

    const RE::BSFixedString scriptName("EM03QuestScript");
    const RE::BSFixedString functionName(FunctionName);

    auto* quest = UI::SCScriptQuest;
    if (!quest) {
        logger::error("Quest not found");
        return;
    }

    auto* policy = vm->GetObjectHandlePolicy();
    if (!policy) {
        logger::error("HandlePolicy not found");
        return;
    }

    RE::VMHandle questHandle = policy->GetHandleForObject(quest->GetFormType(), quest);
    auto callback = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>();

    // Call the function
    bool success = vm->DispatchMethodCall(questHandle, scriptName, functionName, args, callback);
    // VM handle to the object (quest), name of script attached to the object, name of Papyrus function, arguments

    if (!success) {
        logger::error("Failed to dispatch method call to Papyrus.");
    }
}


//function OpenSpellCraftMenu() global native
void OpenSpellCraftMenu(RE::StaticFunctionTag*, RE::TESQuest* theQuest,
                        const std::vector<RE::TESForm*> formArray) {
    logger::info("OpenSpellCraftMenu Called"); 

    if (theQuest) {
        UI::SCScriptQuest = theQuest;
    } else {
        logger::error("couldn't get the Quest");
    }

    RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    const auto& allSpells = dataHandler->GetFormArray<RE::SpellItem>();
    for (auto* spell : allSpells) {
        if (spell->GetSpellType() == RE::MagicSystem::SpellType::kSpell && player->HasSpell(spell)) {
            auto Skill = spell->GetAssociatedSkill();
            switch (Skill) {
                case RE::ActorValue::kAlteration:
                    UI::ChoseEffectMenu::AltSpells.push_back(spell);
                    break;
                case RE::ActorValue::kConjuration:
                    UI::ChoseEffectMenu::ConSpells.push_back(spell);
                    break;
                case RE::ActorValue::kDestruction:
                    UI::ChoseEffectMenu::DesSpells.push_back(spell);
                    break;
                case RE::ActorValue::kIllusion:
                    UI::ChoseEffectMenu::IllSpells.push_back(spell);
                    break;
                case RE::ActorValue::kRestoration:
                    UI::ChoseEffectMenu::ResSpells.push_back(spell);
                    break;
                default:
                    UI::ChoseEffectMenu::ElseSpells.push_back(spell);
            }
        }
    }

    UI::ChoseEffectMenu::LearnedEffects.clear();
     for (auto* form : formArray) {
        if (form && form->GetFormType() == RE::FormType::MagicEffect) {
            auto* effect = form->As<RE::EffectSetting>();
            if (effect) {
                UI::ChoseEffectMenu::LearnedEffects.push_back(effect);
            }
        }
    }

    UI::SpellCraftMenu::OverideEffect = false;
    UI::SpellCraftMenu::OverideBase = nullptr;
    UI::SpellCraftMenu::OverSchoolInt = 5;
    UI::SpellCraftMenu::SpellCraftWindow->IsOpen = true;
}

//Spell function OpenChoseSpellMenu() global native
void OpenChoseSpellMenu(RE::StaticFunctionTag*, RE::TESQuest* theQuest) {
    if (theQuest) {
        UI::SCScriptQuest = theQuest;
    } else {
        logger::error("couldn't get the Quest");
    }

    logger::info("OpenChoseSpellMenu Called");
    RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
    for (auto spell : player->GetActorRuntimeData().addedSpells) {
        if (spell->GetSpellType() == RE::MagicSystem::SpellType::kSpell) {
            auto Skill = spell->GetAssociatedSkill();
            switch (Skill) {
                case RE::ActorValue::kAlteration:
                    UI::ChoseEffectMenu::AltSpells.push_back(spell);
                    break;
                case RE::ActorValue::kConjuration:
                    UI::ChoseEffectMenu::ConSpells.push_back(spell);
                    break;
                case RE::ActorValue::kDestruction:
                    UI::ChoseEffectMenu::DesSpells.push_back(spell);
                    break;
                case RE::ActorValue::kIllusion:
                    UI::ChoseEffectMenu::IllSpells.push_back(spell);
                    break;
                case RE::ActorValue::kRestoration:
                    UI::ChoseEffectMenu::ResSpells.push_back(spell);
                    break;
                default:
                    UI::ChoseEffectMenu::ElseSpells.push_back(spell);
            }
        }
    }
    UI::ChoseEffectMenu::Erasing = true;
    UI::ChoseEffectMenu::ChoseEffectWindow->IsOpen = true;
}   

//function SetEffectStuff(MagicEffect theEffect) global native
void SetEffectStuff(RE::StaticFunctionTag*, RE::EffectSetting* theEffect) {
    logger::info("SetEffectStuff Called");
    if (!theEffect) {
        logger::error("Invalid EffectSetting pointer(s)");
        return;
    }

    theEffect->data.delivery = UI::SpellCraftMenu::OverideBase->GetDelivery();
    theEffect->data.castingType = UI::SpellCraftMenu::OverideBase->GetCastingType();
}

//function GiveSettings(int TomeCost, bool MustHavePerk) global native
void GiveSettings(RE::StaticFunctionTag*, int32_t TomeCost, int32_t EffectCost1, bool MustHavePerk, bool FullScreen1, bool ShowogCost1,
                   float ME, float DM, float AM, int32_t SliderMax1) { 
    UI::MCP::tomeCost = TomeCost; 
    UI::MCP::EffectGCost = EffectCost1;
    UI::ShowPerkEffects = MustHavePerk;
    UI::MCP::FullScreen = FullScreen1;
    //UI::BCostMult = BCM;
    UI::MagExp = ME;
    UI::DMult = DM;
    UI::AMult = AM;
    UI::SliderMax = SliderMax1;
    UI::MCP::ShowOgCost = ShowogCost1;
}
//function GiveESLArrays(Form[] SpellstoChange, Form[] ESLSpells1) global native
void ReturnESLEffects(RE::StaticFunctionTag*, const std::vector<RE::TESForm*> SpellstoChange,
                      const std::vector<RE::TESForm*>ESLSpells1, const std::vector<int> LastEffect1,
                      const std::vector<RE::TESForm*> spellModels1, const std::vector<int> Magnitudes1,
                      const std::vector<int> Areas1, const std::vector<int> Durations1,
                      const std::vector<float> Costs1) {
    
    constexpr std::string_view kNameId = "EM03ID_"sv;
    int LastEffectStart = 0;
    for (std::size_t i = 0; i < SpellstoChange.size(); i++) {
        auto* form = SpellstoChange[i];
        RE::SpellItem* SpelltoEdit = nullptr;

        if (form && form->GetFormType() == RE::FormType::Spell) {
            SpelltoEdit = form->As<RE::SpellItem>();

        } else if (i != SpellstoChange.size() - 1) {
            logger::error("Spell is null or not a spell at index {}", i);
            continue;
        } else {
            break;
        }

        auto* form3 = spellModels1[i];
        RE::SpellItem* SpellModel = nullptr;

        if (form3 && form3->GetFormType() == RE::FormType::Spell) {
            SpellModel = form3->As<RE::SpellItem>();
        } else {
            logger::error("SpellModel is null or not a spell at index {}", i);
            continue;
        }

        auto* SpellMenuObject = SpellModel->GetMenuDisplayObject();
        if (SpellMenuObject) {
            SpelltoEdit->menuDispObject = SpellMenuObject;
        } else {
            logger::error("The SpelltoAdd '{}' has no Menu Display Object", SpellModel->GetFullName());
        }
        
        auto* theEffect = SpelltoEdit->effects[0]->baseEffect;
        auto* mgefBase = SpellModel->effects[0]->baseEffect;
        std::string_view Name = theEffect->GetFullName();
        if (Name == kNameId) {
            theEffect->data.delivery = SpelltoEdit->GetDelivery();
            theEffect->data.castingType = SpelltoEdit->GetCastingType();
            if (mgefBase->data.light) {
                theEffect->data.light = mgefBase->data.light;
            }
            if (mgefBase->data.hitVisuals) {
                theEffect->data.hitVisuals = mgefBase->data.hitVisuals;
            }
            if (mgefBase->data.projectileBase) {
                theEffect->data.projectileBase = mgefBase->data.projectileBase;
            }
            if (mgefBase->data.explosion) {
                theEffect->data.explosion = mgefBase->data.explosion;
            }
            if (mgefBase->data.castingArt) {
                theEffect->data.castingArt = mgefBase->data.castingArt;
            }
            if (mgefBase->data.hitEffectArt) {
                theEffect->data.hitEffectArt = mgefBase->data.hitEffectArt;
            }
            if (mgefBase->data.impactDataSet) {
                theEffect->data.impactDataSet = mgefBase->data.impactDataSet;
            }
            if (mgefBase->data.imageSpaceMod) {
                theEffect->data.imageSpaceMod = mgefBase->data.imageSpaceMod;
            }

            theEffect->menuDispObject = mgefBase->GetMenuDisplayObject();

            theEffect->data.castingSoundLevel = mgefBase->data.castingSoundLevel;

            theEffect->effectSounds.clear();
            for (auto& sound : mgefBase->effectSounds) {
                theEffect->effectSounds.push_back(sound);
            }
        }

        for (int i1 = LastEffectStart; i1 <= LastEffect1[i]; i1++) {
            auto* form2 = ESLSpells1[i1];
            RE::EffectSetting* EffecttoAdd = nullptr;
            if (form2) {
                if (form2->GetFormType() == RE::FormType::MagicEffect) {
                    EffecttoAdd = form2->As<RE::EffectSetting>();
                } else {
                    logger::error("Saved Effect Not a Magic Effect at index {}", i1);
                }
            } else {
                logger::error("Saved Effect is null at index {}", i1);
                continue;
            }

            auto newEffect = new RE::Effect();
            
            newEffect->cost = Costs1[i1];
            newEffect->baseEffect = EffecttoAdd;
            newEffect->effectItem.magnitude = Magnitudes1[i1];
            newEffect->effectItem.area = Areas1[i1];
            newEffect->effectItem.duration = Durations1[i1];

            SpelltoEdit->effects.push_back(newEffect);
        }
        LastEffectStart = LastEffect1[i] + 1;
    }
}

void EraseArrayData(RE::StaticFunctionTag*, std::vector<RE::TESForm*> SpellstoChange,
                      std::vector<RE::TESForm*> ESLSpells1, std::vector<int> LastEffect1,
                      std::vector<RE::TESForm*> spellModels1, std::vector<int> Magnitudes1,
                      std::vector<int> Areas1, std::vector<int> Durations1,
                      std::vector<float> Costs1, RE::TESForm* FormtoErase) {
    int i = 0;
    for (RE::TESForm* FormChecked : SpellstoChange) {
        if (!FormChecked) {
            if (i < SpellstoChange.size() - 1) {
                logger::error("nullprt at {}", i);
                i++;
                continue;
            } else {
                return;
            }
        } else if (FormChecked == FormtoErase) {
            break;
        }
        i++;
    }
    
    if ( i>= SpellstoChange.size()) {
        logger::error("SpellstoChange out of range at index ", i);
        return;
    }
    SpellstoChange.erase(SpellstoChange.begin() + i);
    if (i >= spellModels1.size()) {
        logger::error("SpellModels out of range at index ", i);
       return;
    }
    spellModels1.erase(spellModels1.begin() + i);
    
    int LastEffectStart = 0;
    if (i > 0) {
        LastEffectStart = LastEffect1[i - 1] + 1;
    }
    int NumEffects = 0;

     for (int i1 = LastEffect1[i]; i1 >= LastEffectStart; i1--) {
        
         if (i1 >= ESLSpells1.size()) {
            logger::error("ESLSpells out of range at i1: {}", i1);
            return;
         } 
        ESLSpells1.erase(ESLSpells1.begin() + i1);
        if (i1 >= Magnitudes1.size()) {
            logger::error("Magnitudes1 out of range at i1: {}", i1);
            return;
        }
        Magnitudes1.erase(Magnitudes1.begin() + i1);
        if (i1 >= Areas1.size()) {
            logger::error("Areas1 out of range at i1: {}", i1);
            return;
        }
        Areas1.erase(Areas1.begin() + i1);
        if (i1 >= Durations1.size()) {
            logger::error("Durations1 out of range at i1: {}", i1);
            return;
        }
        Durations1.erase(Durations1.begin() + i1);
        if (i1 >= Costs1.size()) {
            logger::error("Costs1 out of range at i1: {}", i1);
            return;
        }
        Costs1.erase(Costs1.begin() + i1);
        NumEffects++;
    }
     for (int i2 = i + 1; i2 < LastEffect1.size(); i2++) {
         LastEffect1[i2] -= NumEffects;
     }
      if (i >= LastEffect1.size()) {
         logger::error("LastEffect1 out of range at index ", i);
         return;
     }
    LastEffect1.erase(LastEffect1.begin() + i);
    
    auto* args = RE::MakeFunctionArguments(
        (std::vector<RE::TESForm*>)SpellstoChange, (std::vector<RE::TESForm*>)ESLSpells1, (std::vector<int>)LastEffect1,
        (std::vector<RE::TESForm*>)spellModels1, (std::vector<int>) Magnitudes1,
                                  (std::vector<int>)Areas1, (std::vector<int>)Durations1, (std::vector<float>)Costs1);
    CallPapyrusFunction("ApplyArrays", args);
}

bool CheckifESLFlagged(RE::StaticFunctionTag*, RE::TESForm* theForm) {
    if (!theForm) {
        return false;
    }
    auto file = theForm->GetFile();
    if (file->IsLight()) {
        auto fileName = file->GetFilename();
        if (fileName.size() > 4) {
            auto ext = fileName.substr(fileName.size() - 4);
            if (ext == ".esp") {
                return true;
            }
        } else {
            logger::error("FileName is smaller than 4 characters?");
        }
    }
    return false;
}




bool PapyrusFunctions(RE::BSScript::IVirtualMachine * vm) { 
    vm->RegisterFunction("OpenSpellCraftMenu", "EM03SKSEFunctions", OpenSpellCraftMenu);
    vm->RegisterFunction("OpenChoseSpellMenu", "EM03SKSEFunctions", OpenChoseSpellMenu);
    vm->RegisterFunction("SetEffectStuff", "EM03SKSEFunctions", SetEffectStuff);
    vm->RegisterFunction("GiveSettings", "EM03SKSEFunctions", GiveSettings);
    vm->RegisterFunction("ReturnESLEffects", "EM03SKSEFunctions", ReturnESLEffects);
    vm->RegisterFunction("EraseArrayData", "EM03SKSEFunctions", EraseArrayData);
    vm->RegisterFunction("CheckifESLFlagged", "EM03SKSEFunctions", CheckifESLFlagged);
    return true; }


const char* GetCastingtypeS(RE::MagicSystem::CastingType EffectCT) { 
    switch (EffectCT) {
        case RE::MagicSystem::CastingType::kConstantEffect:
            return reinterpret_cast<const char*>(u8"常驻效果"); //Constant Effect
            break;
        case RE::MagicSystem::CastingType::kFireAndForget:
            return reinterpret_cast<const char*>(u8"瞬发");  //"Instant";
            break;
        case RE::MagicSystem::CastingType::kConcentration:
            return reinterpret_cast<const char*>(u8"专注");  //"Concentration";
            break;
        default:
            logger::error("Casting Type not one of the used for spells? None?");
            return "Error: don't recognise this";
    }
}

const char* GetDeliverytypeS(RE::MagicSystem::Delivery EffectD) {
    switch (EffectD) {
        case RE::MagicSystem::Delivery::kAimed:
            return reinterpret_cast<const char*>(u8"指向/瞄准");  //"Aimed";
            break;
        case RE::MagicSystem::Delivery::kSelf:
            return reinterpret_cast<const char*>(u8"自身");  //"Self";
            break;
        case RE::MagicSystem::Delivery::kTargetActor:
            return reinterpret_cast<const char*>(u8"目标生物");  //"Target Actor";
            break;
        case RE::MagicSystem::Delivery::kTargetLocation:
            return reinterpret_cast<const char*>(u8"目标位置");  //"Target Location";
            break;
        case RE::MagicSystem::Delivery::kTouch:
            return reinterpret_cast<const char*>(u8"接触");  //"Touch";
            break;
        default:
            logger::error("Delivery not one of the 5 in commonlib? None?");
            return "Error: don't recognise this";
    }
}

const char* GetSchoolS(RE::ActorValue Skill) {
    switch (Skill) {
        case RE::ActorValue::kAlteration:
            return reinterpret_cast<const char*>(u8"派系：变化系");  //"School: Alteration";
            break;
        case RE::ActorValue::kConjuration:
            return reinterpret_cast<const char*>(u8"派系：召唤系");  //"School: Conjuration";
            break;
        case RE::ActorValue::kDestruction:
            return reinterpret_cast<const char*>(u8"派系：毁灭系");  //"School: Destruction";
            break;
        case RE::ActorValue::kIllusion:
            return reinterpret_cast<const char*>(u8"派系：幻术系");  //"School: Illusion";
            break;
        case RE::ActorValue::kRestoration:
            return reinterpret_cast<const char*>(u8"派系：恢复系");  //"School: Restoration";
            break;
        default:
            return reinterpret_cast<const char*>(u8"无派系");  //"No School";
    }
}



void StopSpellCrafting(bool cancelled, bool noName) {
    using namespace UI;
    using namespace SpellCraftMenu;

     if (!cancelled) {
        noSchool = false;
        auto maxIt = std::max_element(CostList.begin(), CostList.end());
        auto HighestCostid = static_cast<int>(std::distance(CostList.begin(), maxIt));
        if (SpellEffects.empty() || (OverideEffect && (OverideBase == nullptr || OverSchoolInt == 5))) {
            Empty = true;
            return;
        } else if (!OverideEffect && SpellEffects[HighestCostid]->GetMagickSkill() == RE::ActorValue::kNone) {
            noSchool = true;
            Empty = true;
            return;
        } else if (SpellName[0] == '\0' && !noName) {
            Empty = true;
            return;
        } else {
            RE::BSFixedString SpellNameStr(SpellName);
            if (!OverideEffect) {
                OverideBase = SourceSpells[HighestCostid];
            }
            auto args = RE::MakeFunctionArguments(
                (RE::BSFixedString)SpellNameStr, (std::vector<RE::EffectSetting*>)SpellEffects,
                (std::vector<int32_t>)MagList, (std::vector<int32_t>)AreaList, (std::vector<int32_t>)DurationList,
                (int32_t)OverSchoolInt, (std::vector<float>)CostList, (RE::SpellItem*)OverideBase,
                (int32_t)HighestCostid, (bool)OverideEffect, (std::vector<RE::SpellItem*>)SourceSpells);
            CallPapyrusFunction("CraftTheSpell", args);
        }
    }
    SpellCraftWindow->IsOpen = false;
    SpellName[0] = '\0';
    SpellEffects.clear();
    CostList.clear();
    MagList.clear();
    AreaList.clear();
    DurationList.clear();
    SourceSpells.clear();
    DList.clear();
    ChoseEffectMenu::AltSpells.clear();
    ChoseEffectMenu::ConSpells.clear();
    ChoseEffectMenu::DesSpells.clear();
    ChoseEffectMenu::IllSpells.clear();
    ChoseEffectMenu::ResSpells.clear();
    ChoseEffectMenu::ElseSpells.clear();
    ChoseEffectMenu::LearnedEffects.clear();
    Empty = false;
    noSchool = false;
}

float AddExtraAD(float ListNum, uint32_t OgNum, int min) {
    float NumExtra = 0;
    if (OgNum < min) {
        if (ListNum < min) {
            NumExtra = (ListNum - OgNum) / 20.0f;
            if (NumExtra < 0) {
                NumExtra = 0;
            }
        } else {
            NumExtra = (min / 20.0f) - (OgNum / 20.0f);
        }
    }
    return NumExtra;
}

void __stdcall UI::SpellCraftMenu::RenderWindow() {
    auto viewport = ImGui::GetMainViewport();
    if (MCP::FullScreen) {
        ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Appearing, ImVec2{0, 0});
        ImGui::SetNextWindowSize(ImVec2{viewport->Size}, ImGuiCond_Appearing);  
    } else {
        auto center = ImGui::ImVec2Manager::Create();
        ImGui::ImGuiViewportManager::GetCenter(center, viewport);
        ImGui::SetNextWindowPos(*center, ImGuiCond_Appearing, ImVec2{0.5f, 0.6f});
        ImGui::ImVec2Manager::Destroy(center);
        ImGui::SetNextWindowSize(ImVec2{viewport->Size.x * 0.6f, viewport->Size.y * 0.6f}, ImGuiCond_Appearing);
    }
    ImGui::Begin(reinterpret_cast<const char*>(u8"无派系##EM03SpellCrafting"), nullptr, ImGuiWindowFlags_None);  //Craft Spell
    

    if (ImGui::Button(reinterpret_cast<const char*>(u8"取消"))) { //Cancel
        StopSpellCrafting(true, false);
        CallPapyrusFunction("CloseBook", RE::MakeFunctionArguments());
    }
    ImGui::SameLine();
    if (ImGui::Button(reinterpret_cast<const char*>(u8"完成"))) {StopSpellCrafting(false, false);}  // Finish
    //at this point it would be easier to make a custom function to do the cast with less words, but sunk cost falacy and I don't want some stuff with the function and some not
    if (Empty) {
        bool Empty1 = SpellEffects.empty();
        bool Empty2 = (OverideEffect && OverideBase == nullptr);
        if (Empty1 && Empty2) {//no effect+ no custom has no base
            ImGui::Text(reinterpret_cast<const char*>(u8"该法术没有任何效果，且自定义效果没有用于复制视觉表现的基础效果。"));
        } else if (Empty1) {//no effects
            ImGui::Text(reinterpret_cast<const char*>(u8"该法术没有任何效果。"));
        } else if (Empty2) {//custom has no base
            ImGui::Text(reinterpret_cast<const char*>(u8"自定义效果没有用于复制视觉表现的基础效果。"));
        } else if (OverideEffect && OverSchoolInt == 5) {//school not chosen 
            ImGui::Text(reinterpret_cast<const char*>(u8"你还没有为自定义效果选择派系。"));
        } else if (noSchool && !OverideEffect) {//if the costliest has no school, override it
            ImGui::Text(reinterpret_cast<const char*>(u8"如果法力消耗最高的效果属于无派系，你必须使用自定义效果来覆盖它。"));
        } else if (SpellName[0] == '\0') {//No name, you sure?
            ImGui::Text(reinterpret_cast<const char*>(u8"该法术没有名称，确定要完成吗？"));
            if (ImGui::Button(reinterpret_cast<const char*>(u8"是"))) {// Yes
                StopSpellCrafting(false, true);
            }
            ImGui::SameLine();
            if (ImGui::Button(reinterpret_cast<const char*>(u8"否"))) { //No
                Empty = false;
            }
        }
    }

    ImGui::BeginChild("Scrolling");
    ImGui::InputText(reinterpret_cast<const char*>(u8"法术名称"), SpellName, 128);  // Spell Name
  
    ImGui::Checkbox(reinterpret_cast<const char*>(u8"使用自定义效果覆盖传递方式、视觉表现和派系？ (否则将使用消耗最高的那个效果)"), &OverideEffect);//Override...
    if (OverideEffect) {
        const char* Lable = reinterpret_cast<const char*>(u8"选择一个法术作为基础");  //"Chose a Spell to use as a base";
        if (OverideBase) {
            auto* Overidemgf = OverideBase->GetCostliestEffectItem()->baseEffect;
            ImGui::Text(reinterpret_cast<const char*>(u8"使用此效果的视觉表现、传递方式和施法类型： %s(%s, %s) "), //Use visuals, ...
                        Overidemgf->GetFullName(), GetDeliverytypeS(Overidemgf->data.delivery),
                        GetCastingtypeS(Overidemgf->data.castingType));
            ImGui::SameLine();
            Lable = reinterpret_cast<const char*>(u8"更换效果");  //"Change the Effect";
        }
        if (ImGui::Button(Lable)) {
            ChangeE1 = true;
            SpellCraftWindow->IsOpen = false;
            ChoseEffectMenu::ChoseEffectWindow->IsOpen = true;
        }
        ImGui::Combo(reinterpret_cast<const char*>(u8"派系"), &OverSchoolInt, Schools, 6);  // School
    }

    ImGui::Text("");
    for (std::size_t i = 0; i< SpellEffects.size(); i++) {
        RE::EffectSetting* Effect = SpellEffects[i];
      
        ImGui::Text(Effect->GetFullName());
        ImGui::SameLine();
        ImGui::PushID(i);
        if (ImGui::Button("Remove")) {//Remove
            SpellEffects.erase(SpellEffects.begin() + i);
            CostList.erase(CostList.begin() + i);
            MagList.erase(MagList.begin() + i);
            AreaList.erase(AreaList.begin() + i);
            DurationList.erase(DurationList.begin() + i);
            SourceSpells.erase(SourceSpells.begin() + i);
            DList.erase(DList.begin() + i);
            break;
        }
        ImGui::PopID();
        ImGui::SameLine;
        ImGui::Text(reinterpret_cast<const char*>(u8"传递方式: %s, 施法类型: %s, %s)"), GetDeliverytypeS(Effect->data.delivery), //Delivery: , Casting Type
                    GetCastingtypeS(Effect->data.castingType), GetSchoolS(Effect->GetMagickSkill()));

        auto Flags = Effect->data.flags;
         if (!(Flags & RE::EffectSetting::EffectSettingData::Flag::kNoMagnitude)) {
            ImGui::PushID(&MagList[i]);
            ImGui::SliderInt(reinterpret_cast<const char*>(u8"强度/幅度"), &MagList[i], 0, SliderMax); //Magnitude
            ImGui::PopID();
        }

        if (!(Flags & RE::EffectSetting::EffectSettingData::Flag::kNoArea)) {
            ImGui::PushID(&AreaList[i]);
            ImGui::SliderInt(reinterpret_cast<const char*>(u8"范围"), &AreaList[i], 0, SliderMax); //Area
            ImGui::PopID();
        }
        if (!(Flags & RE::EffectSetting::EffectSettingData::Flag::kNoDuration)) {
            ImGui::PushID(&DurationList[i]);
            ImGui::SliderInt(reinterpret_cast<const char*>(u8"持续时间"), &DurationList[i], 0, SliderMax); //Duration
            ImGui::PopID();
        }   
        
        float theCost = -1;
        uint32_t OgDur = 0;
        uint32_t OgArea = 0;
        if (SourceSpells[i]) {
            for (auto effect : SourceSpells[i]->effects) {
                if (effect->baseEffect == Effect) {
                    theCost = effect->cost;
                    OgDur = effect->GetDuration();
                    OgArea = effect->GetArea();
                    break;
                }
            }
        }
        auto AreaDiv = AMult;
        if (OgArea > AMult) {AreaDiv = OgArea;}
        float AreaExtra = AddExtraAD(AreaList[i], OgArea, AreaDiv);
        float durExtra = AddExtraAD(DurationList[i], OgDur, 10);

        float Dcost = std::max(DurationList[i] / 10.0f, 1.0f) + durExtra;
        CostList[i] = DList[i] * DMult * powf((std::max(MagList[i], 1) * Dcost), 1.1) *
                      (std::max(AreaList[i] / AreaDiv, 1.0f) + AreaExtra);
        ImGui::Text(reinterpret_cast<const char*>(u8"法力消耗：%.0f"), CostList[i]); //Magicka Cost: 
        if (MCP::EffectGCost != 0) {
            ImGui::SameLine();
            ImGui::Text(reinterpret_cast<const char*>(u8"金币消耗：%.0f"), CostList[i] * MCP::EffectGCost); //Gold Cost:
        }
        if (MCP::ShowOgCost && theCost != -1) {
            ImGui::SameLine();
            ImGui::Text(reinterpret_cast<const char*>(u8"原法术中的消耗：%.0f"), theCost); //Cost in Original Spell: 
        }
        ImGui::Text("");
    }

    if (ImGui::Button(reinterpret_cast<const char*>(u8"添加效果"))) { //Add Effects
        logger::info("Add an Effect");
        SpellCraftWindow->IsOpen = false;
        ChoseEffectMenu::ChoseEffectWindow->IsOpen = true;
    }
    ImGui::EndChild();
    ImGui::End();
}




void GenSpellSchoolMenu(std::vector<RE::SpellItem*> School, ImGuiMCP::ImVec4 Color, const char* Title,
                        std::string_view SearchText) {  // I couldn't think of a name
    using namespace ImGuiMCP;
    ImGui::TextColored(Color, Title);
    for (auto Spell : School) {
        std::string SpellName = Spell->GetFullName();
        if (UI::ChoseEffectMenu::Searching && !SpellName.starts_with(SearchText)) {
            continue;
        }
        
        std::string label = SpellName + "##" + std::string(std::to_string(Spell->formID));
        if (ImGui::Button(label.c_str())) {
            if (UI::ChangeE1) {
                UI::SpellCraftMenu::OverideBase = Spell;
                UI::ChangeE1 = false;
                UI::ChoseEffectMenu::ChoseEffectWindow->IsOpen = false;
                UI::SpellCraftMenu::SpellCraftWindow->IsOpen = true;

            } else if (UI::ChoseEffectMenu::Erasing) {
                UI::ChoseEffectMenu::SpellToErase = Spell;
            }else {
                auto Delivery = Spell->GetDelivery();
                auto Player = RE::PlayerCharacter::GetSingleton();
                bool check = UI::ShowPerkEffects && Delivery == RE::MagicSystem::Delivery::kSelf;
                int OCost = -1;
                bool AEC0 = false; //All Effects Cost 0
                if (!Spell->IsAutoCalc()) {
                    float CostSum = 0;
                    int numImEffects = 0;
                    for (auto& effectItem : Spell->effects) {
                        auto EffectCost = effectItem->cost;
                        CostSum = CostSum + EffectCost; 
                        if (EffectCost > 0.01) {
                            numImEffects++;
                        }
                    }
                    auto theCost = Spell->data.costOverride;
                     if (CostSum != theCost) {
                        if (numImEffects == 0) {
                             logger::info("All effects costed 0");
                            AEC0 = true;
                            OCost = theCost / Spell->effects.size();
                        } else {
                             OCost = theCost / numImEffects;
                        }
                     }
                }
                for (auto& effectItem : Spell->effects) {
                    auto& Conditions = effectItem->conditions;
                    if (check && !Conditions.IsTrue(Player, Player)) {
                        logger::info("the Conditions were not met for the effect to be shown");
                        continue;
                    }
                    auto Duration = effectItem->GetDuration();
                    auto Cost = effectItem->cost;
                    auto Mag = effectItem->GetMagnitude();

                    RE::EffectSetting* mgef = effectItem->baseEffect;
                    UI::SpellCraftMenu::SpellEffects.push_back(mgef);
                    UI::SpellCraftMenu::MagList.push_back(Mag);
                    UI::SpellCraftMenu::AreaList.push_back(effectItem->GetArea());
                    UI::SpellCraftMenu::DurationList.push_back(Duration);
                    UI::SpellCraftMenu::CostList.push_back(Cost);
                    UI::SpellCraftMenu::SourceSpells.push_back(Spell); 

                    float BaseCost1 = mgef->data.baseCost;
                    
                    if ((OCost != -1 && Cost > 0.01) || AEC0) {
                        Cost = OCost;
                    }
                    float TrueBaseCost = 0;
                    if (BaseCost1 > 0.01 || AEC0) {
                        float calcCost = powf((std::max(Mag, 1.0f) * std::max(Duration / 10.0f, 1.0f)), 1.1);
                        TrueBaseCost = (Cost / calcCost) * UI::BCostMult;
                    }
                    UI::SpellCraftMenu::DList.push_back(TrueBaseCost);
                }
                UI::BCostMult = 1;
                UI::ChoseEffectMenu::ChoseEffectWindow->IsOpen = false;
                UI::SpellCraftMenu::SpellCraftWindow->IsOpen = true;
            }
        }
        ImGui::SameLine();
        ImGui::Text("%s, %s", GetDeliverytypeS(Spell->GetDelivery()), GetCastingtypeS(Spell->GetCastingType()));
    }
}

void __stdcall UI::ChoseEffectMenu::RenderWindow() {
    auto viewport = ImGui::GetMainViewport();
    if (MCP::FullScreen) {
        ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Appearing, ImVec2{0, 0});
        ImGui::SetNextWindowSize(ImVec2{viewport->Size}, ImGuiCond_Appearing);
    } else {
        auto center = ImGui::ImVec2Manager::Create();
        ImGui::ImGuiViewportManager::GetCenter(center, viewport);
        ImGui::SetNextWindowPos(*center, ImGuiCond_Appearing, ImVec2{0.5f, 0.6f});
        ImGui::ImVec2Manager::Destroy(center);
        ImGui::SetNextWindowSize(ImVec2{viewport->Size.x * 0.6f, viewport->Size.y * 0.6f},ImGuiCond_Appearing);
    }
    ImGui::Begin(reinterpret_cast<const char*>(u8"已知法术与效果##EM03SpellCrafting"), nullptr, ImGuiWindowFlags_None); //Known Spells and Effects

    if (ImGui::Button(reinterpret_cast<const char*>(u8"取消"))) {//Cancel
        ChangeE1 = false;
        ChoseEffectWindow->IsOpen = false;
        if (Erasing) {
            Erasing = false;
            SpellToErase = nullptr;
            StopSpellCrafting(true, false);
            CallPapyrusFunction("CloseBook", RE::MakeFunctionArguments());
        } else {
            SpellCraftMenu::SpellCraftWindow->IsOpen = true;
        }
    }
    if (SpellToErase) {
        ImGui::Text(reinterpret_cast<const char*>(u8"你确定要擦除并遗忘 %s 吗？"), SpellToErase->GetFullName());//U Sure?
        if (ImGui::Button(reinterpret_cast<const char*>(u8"是"))) {//Yes
            UI::ChoseEffectMenu::ChoseEffectWindow->IsOpen = false;
            CallPapyrusFunction("EraseSpell", RE::MakeFunctionArguments((RE::SpellItem*)SpellToErase));
            UI::ChoseEffectMenu::Erasing = false;
            SpellToErase = nullptr;
            StopSpellCrafting(true, false);
        }
        ImGui::SameLine();
        if (ImGui::Button(reinterpret_cast<const char*>(u8"否"))) {//No
            SpellToErase = nullptr;
        }
    } else {
        ImGui::SameLine();
        ImGui::InputText(reinterpret_cast<const char*>(u8"搜索"), SearchName, 128);//Search
        std::string_view searchText = SearchName;
        if (SearchName[0] != '\0') {
            Searching = true;
        } else {
            Searching = false;
        }
        ImGui::BeginChild(reinterpret_cast<const char*>(u8"Scrolling"));

        if (!AltSpells.empty()) {GenSpellSchoolMenu(AltSpells, ImVec4(0, 0, 1, 1), reinterpret_cast < const char*>(u8"变化系法术"), searchText);}//Alteration Spells
        if (!ConSpells.empty()) {GenSpellSchoolMenu(ConSpells, ImVec4(0.5, 0, 0.5, 1), reinterpret_cast < const char*>(u8"召唤系法术"), searchText);}//Con
        if (!DesSpells.empty()) {GenSpellSchoolMenu(DesSpells, ImVec4(1, 0, 0, 1), reinterpret_cast < const char*>(u8"毁灭系法术"), searchText);}//Des
        if (!IllSpells.empty()) {GenSpellSchoolMenu(IllSpells, ImVec4(0, 1, 1, 1), reinterpret_cast < const char*>(u8"幻术系法术"), searchText);}//Ill
        if (!ResSpells.empty()) {GenSpellSchoolMenu(ResSpells, ImVec4(1, 1, 0, 1), reinterpret_cast < const char*>(u8"恢复系法术"), searchText);}//Res
        if (!ElseSpells.empty()) {GenSpellSchoolMenu(ElseSpells, ImVec4(1, 0, 1, 1), reinterpret_cast < const char*>(u8"其他法术（无派系）"), searchText);} //Other Spells (No School)

        if (!LearnedEffects.empty() && !ChangeE1) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), reinterpret_cast<const char*>(u8"炼金效果"));  // RGBA    Alchemy Effects
            for (auto Effect : LearnedEffects) {
                std::string EffectName = Effect->GetFullName();
                if (UI::ChoseEffectMenu::Searching && !EffectName.starts_with(searchText)) {
                    continue;
                }
                std::string label = EffectName + "##" + std::string(std::to_string(Effect->formID));
                if (ImGui::Button(label.c_str())) {
                    SpellCraftMenu::SpellEffects.push_back(Effect);
                    SpellCraftMenu::CostList.push_back(0);
                    SpellCraftMenu::MagList.push_back(15);
                    SpellCraftMenu::AreaList.push_back(0);
                    SpellCraftMenu::DurationList.push_back(10);
                    SpellCraftMenu::SourceSpells.push_back(nullptr);
                    SpellCraftMenu::DList.push_back(BCostMult * Effect->data.baseCost);
                    BCostMult = 1;
                    ChoseEffectWindow->IsOpen = false;
                    SpellCraftMenu::SpellCraftWindow->IsOpen = true;
                }
                ImGui::SameLine();
                ImGui::Text("%s, %s", GetDeliverytypeS(Effect->data.delivery),
                            GetCastingtypeS(Effect->data.castingType));
            }
        }
       ImGui::End(); 
    }
    ImGui::End();
}

void __stdcall UI::MCP::Render() {
    if (!SCScriptQuest) {
        bool workednow = false;
        RE::TESForm* form = RE::TESForm::LookupByEditorID("EM03SCScriptQuest");
        if (form) {
            if (auto* quest = form->As<RE::TESQuest>()) {
                UI::SCScriptQuest = quest;
                workednow = true;
                logger::error("Didn't have Quest already but sucecessfully found it now.");
            } else {
                logger::error("Form 'EM03SCScriptQuest' is not a TESQuest.");
            }
        } else {
            logger::error("Failed to find form with EditorID 'EM03SCScriptQuest'.");
        }
        if (!workednow) {
            ImGui::Text(reinterpret_cast<const char*>(u8"错误：无法获取保存或加载设置所需的 Papyrus 脚本。"));//Error:...
            logger::error("SCScriptQuest is not filled/ is nullptr");
        }
    } else if (!GotSettings) {
        CallPapyrusFunction("GetSettings", RE::MakeFunctionArguments());
        GotSettings = true;
    }
    

    if (ImGui::Button(reinterpret_cast<const char*>(u8"保存设置"))) {//Save Settings
        auto args = RE::MakeFunctionArguments((int32_t)tomeCost, (int32_t)EffectGCost, (bool)ShowPerkEffects,
                    (bool)FullScreen, (bool)ShowOgCost, (float)MagExp, (float)DMult, (float)AMult, (int32_t)SliderMax);
        CallPapyrusFunction("ApplySettings", args);
        GotSettings = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(reinterpret_cast<const char*>(u8"恢复默认"))) {//Restore Degaults
        ShowPerkEffects = true;
        FullScreen = false;
        ShowOgCost = false;
        BCostMult = 1.0;
        MagExp = 1.1;
        DMult = 1;
        AMult = 15;
        SliderMax = 100;
        tomeCost = 0;
        EffectGCost = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button(reinterpret_cast<const char*>(u8"将“法术制作书”添加到物品栏"))) {//Add Tome
        CallPapyrusFunction("AddSCTome" , RE::MakeFunctionArguments());
    }
    ImGui::Text(reinterpret_cast<const char*>(u8"部分更改会立即生效，但在点击“保存设置”前不会被永久保存。"));//Some Changes...

    ImGui::BeginChild("Scrolling");
    
    ImGui::Text("");
    if (ImGui::Button(reinterpret_cast<const char*>(u8"显示有多少 ESL 标记效果被特别保存"))) {
        CallPapyrusFunction("ShowHowManySaved", RE::MakeFunctionArguments());
    }
    ImGui::Text(reinterpret_cast<const char*>(u8"如果数值过高（600+），可能会导致加载时间延长几秒钟。"));
    ImGui::Text(reinterpret_cast < const char*>(u8"此外，还可能存在稳定性问题。我建议数值保持在 350 以下。（你可以通过移除法术来降低数值）"));

    ImGui::Text("");
    ImGui::Text(reinterpret_cast<const char*>(u8"设定获取制作书所需的金币数量："));
    ImGui::InputInt(reinterpret_cast<const char*>(u8"金币成本"), &tomeCost, 10, 100);//Gold Cost
    ImGui::Text("");

    ImGui::Checkbox(reinterpret_cast<const char*>(u8"若效果来源于“自身”类法术，则必须拥有相应特权(Perk)才能添加"), &ShowPerkEffects);//Must have Perk...
    ImGui::Checkbox(reinterpret_cast<const char*>(u8"法术制作菜单全屏显示（请勿在菜单打开时更改此项）"), &FullScreen);//Fulscreen
    ImGui::Text("");

    ImGui::Text(reinterpret_cast<const char*>(u8"强度、范围和持续时间滑块的最大数值上限："));//Max Number for Sliders
    ImGui::InputInt(reinterpret_cast<const char*>(u8"输入数字"), &SliderMax, 10, 100);//Enter a Number
    ImGui::Text("");

    ImGui::Checkbox(reinterpret_cast<const char*>(u8"显示该效果在原法术中的消耗"), &ShowOgCost);//Show Cost
    ImGui::Text(reinterpret_cast<const char*>(u8"大致法力消耗公式 = B x 基础消耗 x(强度 x (持续时间/10))^M x (范围/A)")); //Equation
    ImGui::SliderFloat("B", &DMult, 0, 5);
    ImGui::SliderFloat("M", &MagExp, 0, 5);
    ImGui::SliderFloat("A", &AMult, 0, 25);
    ImGui::Text(reinterpret_cast<const char*>(u8"仅在该数值之后添加的下一个效果的消耗乘以该倍率："));//Multiply Costs of...
    ImGui::SliderFloat(reinterpret_cast<const char*>(u8"下一个效果消耗倍率"), &BCostMult, 0, 5);//Next Effect Cost Multiplier
    ImGui::Text(reinterpret_cast<const char*>(u8"在制作法术后更改任何设置，不会追溯改变已制作法术的消耗。"));//Changing after
    ImGui::Text("");

    ImGui::Text(reinterpret_cast<const char*>(u8"设置每个效果每单位法力消耗对应的金币成本："));//Set a Gold Cost per...
    ImGui::InputInt(reinterpret_cast<const char*>(u8"金币/法力值"), &EffectGCost, 1, 10); //Gold/ Magicka
    ImGui::EndChild();
}



void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kPostLoad) {
        SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);
        UI::SpellCraftMenu::SpellCraftWindow = SKSEMenuFramework::AddWindow(UI::SpellCraftMenu::RenderWindow);
        UI::ChoseEffectMenu::ChoseEffectWindow = SKSEMenuFramework::AddWindow(UI::ChoseEffectMenu::RenderWindow);
        SKSEMenuFramework::SetSection(reinterpret_cast<const char*>(u8"第四纪元法术制作"));//Fourth Era Spell Crafting
        SKSEMenuFramework::AddSectionItem(reinterpret_cast<const char*>(u8"主页面"), UI::MCP::Render); //Main Page
    }

    else if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        RE::TESForm* form = RE::TESForm::LookupByEditorID("EM03SCScriptQuest");
        if (form) {
            if (auto* quest = form->As<RE::TESQuest>()) {
                UI::SCScriptQuest = quest;
            } else {
                logger::error("Form 'EM03SCScriptQuest' is not a TESQuest.");
            }
        } else {
            logger::error("Failed to find form with EditorID 'EM03SCScriptQuest'.");
        }
    }
     else if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        CallPapyrusFunction("GetArrays", RE::MakeFunctionArguments());
        UI::MCP::GotSettings = false;
        RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
        constexpr std::string_view kNameId = "EM03ID_"sv;
        for (auto Spell : player->GetActorRuntimeData().addedSpells) {
             if (Spell->GetSpellType() == RE::MagicSystem::SpellType::kSpell) {
                 auto mgef = Spell->effects[0]->baseEffect;
                 std::string_view Name = mgef->GetFullName();
                 if (Name == kNameId) {
                     mgef->data.delivery = Spell->GetDelivery();
                     mgef->data.castingType = Spell->GetCastingType();

                 }
             }
        }
    }
}


SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SetupLog();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}
