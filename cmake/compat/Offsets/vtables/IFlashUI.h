#pragma once
// MinGW compat stub for Offsets/vtables/IFlashUI.h.
// The real header pulls in SUITypes.h -> CryArray.h -> platform.h macros
// (CryModuleMalloc, Align, SPU_MAIN_PTR, CryStringT) that don't compile
// under MinGW without the full CryEngine build environment.
// CombatAssist never calls any IFlashUI methods; a forward declaration
// of Offsets::IFlashUI satisfies the SSystemGlobalEnvironment::pFlashUI
// pointer member.
namespace Offsets {
struct IFlashUI;
}
// Hollow dispatcher stubs (normally appended by the real IFlashUI.h for
// users of the header; not needed by CombatAssist).
template<class TEventType> struct SUIEventReceiverDispatcher {};
template<class TEventType> struct SUIEventSenderDispatcher {};
