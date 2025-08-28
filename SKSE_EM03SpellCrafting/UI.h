#pragma once
#include "SKSEMenuFramework.h"

namespace UI {
    inline bool ChangeE1 = false;
    inline bool ShowPerkEffects = true;
    inline float BCostMult = 1.0;
    inline float MagExp = 1.1;
    inline float DMult = 1;
    //inline float DMin = 0.25;
    inline float AMult = 15;
    inline int SliderMax = 100;
    inline RE::TESQuest* SCScriptQuest = nullptr;

    namespace SpellCraftMenu {
        inline bool Empty = false;
        inline bool noSchool = false;
        inline char SpellName[128] = "";
        inline bool OverideEffect = false;
        inline const char* const Schools[] = {"Alteration", "Conjuration", "Destruction", "Illusion", "Restoration", "Not Chosen"};
        inline int OverSchoolInt = 5;
        inline RE::SpellItem* OverideBase = nullptr;
        inline std::vector<RE::EffectSetting*> SpellEffects;
        inline std::vector<float> CostList;
        inline std::vector<float> DList;
        inline std::vector<int32_t> MagList;
        inline std::vector<int32_t> AreaList;
        inline std::vector<int32_t> DurationList;
        inline std::vector<RE::SpellItem*> SourceSpells;
        void __stdcall RenderWindow();
        inline MENU_WINDOW SpellCraftWindow;
    }

    namespace ChoseEffectMenu {
        inline char SearchName[128] = "";
        inline bool Searching = false;
        inline bool Erasing = false;
        inline RE::SpellItem* SpellToErase = nullptr;
        inline std::vector<RE::EffectSetting*> LearnedEffects;
        inline std::vector<RE::SpellItem*> AltSpells;
        inline std::vector<RE::SpellItem*> ConSpells;
        inline std::vector<RE::SpellItem*> DesSpells;
        inline std::vector<RE::SpellItem*> IllSpells;
        inline std::vector<RE::SpellItem*> ResSpells;
        inline std::vector<RE::SpellItem*> ElseSpells;
        void __stdcall RenderWindow();
        inline MENU_WINDOW ChoseEffectWindow;
    }
    
    namespace MCP {
        inline bool ShowOgCost = false;
        inline int tomeCost = 0;
        inline int EffectGCost = 0;
        inline bool FullScreen = false;
        inline bool GotSettings = false;
        void __stdcall Render();
    }
};