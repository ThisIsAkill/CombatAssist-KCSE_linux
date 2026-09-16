#pragma once

#if defined(__GNUC__) && !defined(_MSC_VER)
// ── MinGW cross-compilation compatibility ─────────────────────────────────
// plugin.cpp relies on the full kcd.h umbrella PCH (Cry_Math, MultiThread,
// RTTI, etc.) which is MSVC-only.  This header provides targeted replacements.

#include <cassert>
#include <windows.h>   // LONG, InterlockedDecrement/Increment

// ── CryEngine base type definitions ──────────────────────────────────────
// Must come before any SDK header that uses int64/uint64/uint32 etc.
// (IConsole.h and ILog.h SDK use these without pulling in BaseTypes.h.)
#include "CryEngine/CryCommon/BaseTypes.h"

// ── CryMath type stubs ───────────────────────────────────────────────────
// Pre-define Cry_Math.h guard so it's skipped when transitively pulled in;
// provide minimal versions of the types the game headers need.
#define CRYMATH_H

struct Vec2 { float x, y; };
struct Vec3 {
    float x, y, z;
    Vec3() = default;
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};
struct Quat    { float w, x, y, z; };
struct Matrix34 { float m[3][4]; };

// ── CryEngine macro stubs ────────────────────────────────────────────────
// PRINTF_PARAMS is from platform.h; smartptr.h uses it unconditionally.
#define PRINTF_PARAMS(...)

// ── CryInterlockedDecrement/Increment ────────────────────────────────────
// MultiThread.h declares these as external for Windows — the definitions live
// in the game EXE and are not linkable from a plugin DLL.  Pre-define the
// MultiThread.h include guard and supply inline Win32 implementations instead.
#define __MultiThread_h__

inline LONG CryInterlockedDecrement(int volatile* p) {
    return InterlockedDecrement(reinterpret_cast<volatile LONG*>(p));
}
inline LONG CryInterlockedIncrement(int volatile* p) {
    return InterlockedIncrement(reinterpret_cast<volatile LONG*>(p));
}

// ── __RTDynamicCast — MSVC RTTI helper used by kcd_cast ──────────────────
// Under MSVC this is compiler-predefined; under MinGW we load it at runtime
// from vcruntime140.dll (already mapped into the WHGame process).
// Must be declared before RTTI.h is included.
extern "C" inline void* __cdecl __RTDynamicCast(
        void* inptr, long vfDelta, void* srcType, void* targetType, int isRef) noexcept {
    using Fn = void* (__cdecl*)(void*, long, void*, void*, int);
    static Fn fn = reinterpret_cast<Fn>(
        GetProcAddress(GetModuleHandleA("vcruntime140.dll"), "__RTDynamicCast"));
    return fn ? fn(inptr, vfDelta, srcType, targetType, isRef) : nullptr;
}

// ── libKCD1 headers plugin.cpp relies on from the kcd.h umbrella PCH ─────
#include "Offsets/Offsets_RTTI.h"             // Offsets::RTTI_* constants
#include "Offsets/RTTI.h"                      // kcd_cast<>
#include "crysystem/SSystemGlobalEnvironment.h" // SSystemGlobalEnvironment, VF_NULL
#include "combatmodule/C_CombatActorDirector.h"
#include "combatmodule/C_CombatActionSyncHitData.h"  // full type for m_pDataRow->GetActionTypeId()
#include "combatmodule/C_CombatActorActionSyncHit.h"

#endif  // __GNUC__ && !_MSC_VER
