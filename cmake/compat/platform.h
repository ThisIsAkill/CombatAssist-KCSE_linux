#pragma once
// Minimal CryEngine platform.h stub for MinGW cross-compilation.
// The real platform.h pulls in CryMemoryManager.h -> CryThread.h -> StlUtils.h
// (deprecated hash_map, CryCreateCriticalSectionInplace, etc.) which don't
// compile cleanly under MinGW libstdc++.
// int8/int16/int32/int64/uint8/uint16/uint32/uint64 are provided by
// BaseTypes.h, which is included in PCH.h before any SDK header is reached.
#ifndef ILINE
#  define ILINE inline __attribute__((always_inline))
#endif

// Used as an empty class-body member in CryEngine SDK vtable classes.
#ifndef DEVIRTUALIZATION_VTABLE_FIX
#  define DEVIRTUALIZATION_VTABLE_FIX
#endif
#ifndef DEVIRTUALIZATION_VTABLE_FIX_IMPL
#  define DEVIRTUALIZATION_VTABLE_FIX_IMPL(CLASS)
#endif

// CryEngine SPU pointer macros — identity on non-SPU targets.
#ifndef SPU_MAIN_PTR
#  define SPU_MAIN_PTR(PTR) (PTR)
#endif
#ifndef SPU_MAIN_REF
#  define SPU_MAIN_REF(REF) (REF)
#endif

// CryStringT — minimal stub (the real CryString.h pulls in check_cast, _usedMemory,
// and other CryEngine internals that don't compile under MinGW).
// CryStringT<char> is just a pointer wrapper (sizeof == 8, same as MSVC x64).
// Pre-define CryString.h's traditional guard so the real header is skipped if
// anything else tries to include it.
#define __CryString_h__
#define CRY_STRING
#define CRY_STRING_DEBUG(s)
template<typename T>
class CryStringT {
public:
    T* m_str = nullptr;
    CryStringT() = default;
    CryStringT(const T* s) : m_str(const_cast<T*>(s)) {}
    operator const T*() const { return m_str; }
    const T* c_str() const { return m_str; }
};
typedef CryStringT<char>    CryStringLocal;
typedef CryStringT<wchar_t> CryStringLocalW;

// CryEngine TypeInfo reflection macros — empty when type-info generation is off.
// smartptr.h uses AUTO_STRUCT_INFO inside its class body and requires it defined.
#ifndef AUTO_STRUCT_INFO
#  define AUTO_STRUCT_INFO
#endif
#ifndef NULL_STRUCT_INFO
#  define NULL_STRUCT_INFO
#endif
#ifndef AUTO_TYPE_INFO
#  define AUTO_TYPE_INFO(T)
#endif
#ifndef AUTO_STRUCT_INFO_LOCAL
#  define AUTO_STRUCT_INFO_LOCAL
#endif
