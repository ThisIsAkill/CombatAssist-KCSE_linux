#pragma once
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <Windows.h>

namespace VtableHook {

namespace detail {

// A real vtable (or vtable-like slot array reached via a raw RVA) lives in a
// read-only or read+exec mapped page, never on the heap or stack. Rejecting
// anything else catches the case where the address-library offset behind it
// is stale for the running game build: without this check, writing through
// a garbage address silently corrupts whatever unrelated memory is there
// instead of failing loudly.
inline bool IsPlausibleCodePage(const void* p) {
    if (!p) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery(p, &mbi, sizeof(mbi))) return false;
    if (mbi.State != MEM_COMMIT) return false;
    switch (mbi.Protect & 0xFF) {
    case PAGE_READONLY:
    case PAGE_EXECUTE:
    case PAGE_EXECUTE_READ:
        return true;
    default:
        return false;
    }
}

inline void LogRefusal(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
}

}  // namespace detail

template<typename Fn>
Fn Swap(void* pObject, uint32_t vtableByteOffset, Fn newFunc) {
    if (!pObject) {
        detail::LogRefusal("[CombatAssist] VtableHook::Swap: null object; refusing to hook\n");
        return nullptr;
    }
    auto vtable = *reinterpret_cast<uintptr_t**>(pObject);
    if (!detail::IsPlausibleCodePage(vtable) ||
        !detail::IsPlausibleCodePage(reinterpret_cast<const uint8_t*>(vtable) + vtableByteOffset)) {
        detail::LogRefusal(
            "[CombatAssist] VtableHook::Swap: object at %p has no plausible vtable at +0x%x; "
            "refusing to hook (offset is likely stale for this game build)\n",
            pObject, vtableByteOffset);
        return nullptr;
    }
    auto& slot = vtable[vtableByteOffset / 8];

    DWORD oldProt;
    VirtualProtect(&slot, sizeof(slot), PAGE_EXECUTE_READWRITE, &oldProt);
    auto original = reinterpret_cast<Fn>(slot);
    slot = reinterpret_cast<uintptr_t>(newFunc);
    VirtualProtect(&slot, sizeof(slot), oldProt, &oldProt);

    return original;
}

template<typename Fn>
void Restore(void* pObject, uint32_t vtableByteOffset, Fn originalFunc) {
    if (!pObject) return;
    auto vtable = *reinterpret_cast<uintptr_t**>(pObject);
    if (!detail::IsPlausibleCodePage(vtable) ||
        !detail::IsPlausibleCodePage(reinterpret_cast<const uint8_t*>(vtable) + vtableByteOffset))
        return;
    auto& slot = vtable[vtableByteOffset / 8];

    DWORD oldProt;
    VirtualProtect(&slot, sizeof(slot), PAGE_EXECUTE_READWRITE, &oldProt);
    slot = reinterpret_cast<uintptr_t>(originalFunc);
    VirtualProtect(&slot, sizeof(slot), oldProt, &oldProt);
}

// Patch a vtable slot directly by vtable RVA offset + slot index.
// Uses Offsets_VTABLE constexprs -- no instance needed.
template<typename Fn>
Fn SwapByOffset(uintptr_t base, uintptr_t vtableRva, uint32_t slotIndex, Fn newFunc) {
    auto* vtable = reinterpret_cast<uintptr_t*>(base + vtableRva);
    if (!detail::IsPlausibleCodePage(vtable) ||
        !detail::IsPlausibleCodePage(reinterpret_cast<const uint8_t*>(vtable) + slotIndex * sizeof(uintptr_t))) {
        detail::LogRefusal(
            "[CombatAssist] VtableHook::SwapByOffset: WHGame.dll+0x%llx slot %u has no plausible "
            "vtable; refusing to hook (offset is likely stale for this game build)\n",
            static_cast<unsigned long long>(vtableRva), slotIndex);
        return nullptr;
    }
    auto& slot = vtable[slotIndex];

    DWORD oldProt;
    VirtualProtect(&slot, sizeof(slot), PAGE_EXECUTE_READWRITE, &oldProt);
    auto original = reinterpret_cast<Fn>(slot);
    slot = reinterpret_cast<uintptr_t>(newFunc);
    VirtualProtect(&slot, sizeof(slot), oldProt, &oldProt);

    return original;
}

}  // namespace VtableHook
