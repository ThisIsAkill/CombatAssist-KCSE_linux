#include "KCSE/KCSEAPI.h"
#include "Offsets/Offsets.h"
#include "Offsets/Offsets_VTABLE.h"
#include "entitymodule/C_Actor.h"
#include "combatmodule/C_CombatActor.h"
#include "combatmodule/C_CombatActorHuntAttack.h"
#include "game/S_GameContext.h"
#include "vtable_hook.h"
#include <cstdlib>

// -----------------------------------------------
// CVars
// -----------------------------------------------

int alwaysPB = 0;
int alwaysMS = 0;
int chancePB = 0;
int chanceMS = 0;
int alwaysHuntPB = 0;
int alwaysHuntMS = 0;
int autoCounter = 0;
int alwaysTackle = 0;

// -----------------------------------------------
// Helpers
// -----------------------------------------------

static wh::combatmodule::C_CombatActor* GetPlayerCombatActor()
{
    auto* iactor = Offsets::GetCCryAction()->GetClientActor();
    if (!iactor) return nullptr;
    return reinterpret_cast<wh::entitymodule::C_Actor*>(iactor)->m_pCombatActor;
}

static bool RollChance(int percent)
{
    return percent > 0 && (rand() % 100) < percent;
}

static bool IsInHuntAttack(wh::combatmodule::C_CombatActor* actor)
{
    auto&& director = actor->m_pDirector;
    for (auto&& act : director->m_actions) {
        auto* hunt = kcd_cast<wh::combatmodule::C_CombatActorActionSyncHit*>(act.get());
        if (hunt && hunt->m_params.m_pDataRow->GetActionTypeId() == Offsets::ActionTypeId::HuntAttackSlave()) return true;
    }

    return false;
}

// -----------------------------------------------
// Hunt Attack (Tackle) hook
// -----------------------------------------------
// Hooks I_CombatActorHuntAttack vtable slot [1] (TryHuntAttack).
// For the player: bypasses speed/distance/angle checks but keeps
// "is victim in combat?" to avoid showing prompt on random NPCs.

using TryHuntAttackFn = wh::combatmodule::E_HuntAttackResult(__fastcall*)(
    wh::combatmodule::I_CombatActorHuntAttack*, EntityId);
static TryHuntAttackFn g_origTryHuntAttack = nullptr;

static wh::combatmodule::E_HuntAttackResult __fastcall Hooked_TryHuntAttack(
    wh::combatmodule::I_CombatActorHuntAttack* self, EntityId victimEntityId)
{
    if (alwaysTackle) {
        auto* huntObj = static_cast<wh::combatmodule::C_CombatActorHuntAttack*>(self);
        if (huntObj->m_pOwner == GetPlayerCombatActor()) {
            auto* victim = wh::game::S_GameContext::GetInstance()->GetActorById(victimEntityId);
            if (!victim)
                return g_origTryHuntAttack(self, victimEntityId);

            auto* victimCombat = victim->GetOrCreateCombatActor();
            if (!victimCombat || !victimCombat->m_pState || !victimCombat->m_pState->m_isInCombat)
                return g_origTryHuntAttack(self, victimEntityId);
            
            return wh::combatmodule::E_HuntAttackResult::Approved;
        }
    }
    return g_origTryHuntAttack(self, victimEntityId);
}

// -----------------------------------------------
// Auto Perfect Block / Master Strike
// -----------------------------------------------

static void AutoPerfectBlock()
{
    auto* player = GetPlayerCombatActor();
    if (!player) return;

    auto* state = player->m_pState;
    if (!state || !state->m_pOpponent) return;

    if (state->m_pbTriggerCount <= 0 || state->m_isBlocking || state->m_isPerfectBlocking)
        return;

    bool isHunt = IsInHuntAttack(player);

    bool wantMS = alwaysMS || (isHunt && alwaysHuntMS) || RollChance(chanceMS) || autoCounter > 0;
    bool wantPB = alwaysPB || (isHunt && alwaysHuntPB) || RollChance(chancePB) || autoCounter > 0;

    if (!wantMS && !wantPB)
        return;

    if (wantMS && state->m_syncRiposteTriggerCount > 0) {
        wh::combatmodule::I_CombatActorActionPtr outAction;
        player->DispatchCounterAction(&outAction,
            wh::combatmodule::E_CounterActionType::SyncRiposte, 0);
        if (autoCounter > 0) --autoCounter;
        return;
    }
    else if (wantPB) {
        wh::combatmodule::I_CombatActorActionPtr outAction;
        player->DispatchCounterAction(&outAction,
            wh::combatmodule::E_CounterActionType::PerfectBlock, 0);
        if (autoCounter > 0) --autoCounter;
    }
}

// -----------------------------------------------
// Frame tick
// -----------------------------------------------

static void FrameTick()
{
    AutoPerfectBlock();
    KCSE::GetTaskInterface()->AddTask(FrameTick);
}

// -----------------------------------------------
// Plugin entry
// -----------------------------------------------

KCSE_PLUGIN_INFO("Combat Assist", "JerryYOJ", 1);
KCSE_PLUGIN_LOAD(kcse)
{
    kcse->GetMessagingInterface()->RegisterListener([](KCSE::Message* msg) {
        if (msg->type == KCSE::IMessagingInterface::kMessage_DataLoaded) {
            auto* con = SSystemGlobalEnvironment::GetInstance()->pConsole;

            con->RegisterCVarInt("kcse_ca_alwaysPB",     &alwaysPB,     0, VF_NULL, "1 = always auto perfect block");
            con->RegisterCVarInt("kcse_ca_alwaysMS",     &alwaysMS,     0, VF_NULL, "1 = always auto master strike");
            con->RegisterCVarInt("kcse_ca_chancePB",     &chancePB,     0, VF_NULL, "0-100: % chance to auto perfect block");
            con->RegisterCVarInt("kcse_ca_chanceMS",     &chanceMS,     0, VF_NULL, "0-100: % chance to auto master strike");
            con->RegisterCVarInt("kcse_ca_alwaysHuntPB", &alwaysHuntPB, 0, VF_NULL, "1 = auto PB during hunt attacks");
            con->RegisterCVarInt("kcse_ca_alwaysHuntMS", &alwaysHuntMS, 0, VF_NULL, "1 = auto MS during hunt attacks");
            con->RegisterCVarInt("kcse_ca_autoCounter",  &autoCounter,  0, VF_NULL, "N = auto-react N times then stop");
            con->RegisterCVarInt("kcse_ca_alwaysTackle", &alwaysTackle, 0, VF_NULL, "1 = player can always tackle");

            // Hook TryHuntAttack — patch I_CombatActorHuntAttack vtable slot [1]
            using HuntVT = wh::combatmodule::C_CombatActorHuntAttack;
            g_origTryHuntAttack = VtableHook::SwapByOffset<TryHuntAttackFn>(
                Offsets::GetBase(), HuntVT::VTABLE[1], 1, Hooked_TryHuntAttack);

            SSystemGlobalEnvironment::GetInstance()->pLog->LogAlways("[CombatAssist] CVars registered, hooks installed");
        }
    });

    KCSE::GetTaskInterface()->AddTask(FrameTick);
    return true;
}
