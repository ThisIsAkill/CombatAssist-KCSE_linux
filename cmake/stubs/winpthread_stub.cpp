// Minimal pthread stubs for MinGW posix-thread-model DLLs.
//
// GCC's emutls (emulated thread-local storage) uses a small, fixed set of
// pthread primitives. By providing them ourselves and telling the linker to
// prefer our static definitions over libwinpthread-1.dll, the final DLL needs
// no MinGW-specific runtime DLL at all.
//
// CombatAssist is always invoked on WHGame's main thread; for our purposes
// a per-key "TLS slot" can be a single global value — no multi-thread safety
// needed. We do implement the mutex using CRITICAL_SECTION so the code is
// correct even if anything ever races.

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdlib>  // malloc/free

// ── Types ──────────────────────────────────────────────────────────────────

struct _pthread_mutex_t {
    CRITICAL_SECTION cs;
    bool init = false;
};
using pthread_mutex_t = _pthread_mutex_t;

struct _pthread_once_t {
    volatile LONG done = 0;
};
using pthread_once_t = _pthread_once_t;

using pthread_key_t  = DWORD;   // reuse TLS slot index
using pthread_mutexattr_t = int;

// ── Mutex ─────────────────────────────────────────────────────────────────

extern "C" int pthread_mutex_init(pthread_mutex_t* m, const pthread_mutexattr_t*) {
    InitializeCriticalSection(&m->cs);
    m->init = true;
    return 0;
}
extern "C" int pthread_mutex_destroy(pthread_mutex_t* m) {
    if (m->init) { DeleteCriticalSection(&m->cs); m->init = false; }
    return 0;
}
extern "C" int pthread_mutex_lock(pthread_mutex_t* m) {
    if (!m->init) InitializeCriticalSection(&m->cs);
    EnterCriticalSection(&m->cs);
    return 0;
}
extern "C" int pthread_mutex_unlock(pthread_mutex_t* m) {
    LeaveCriticalSection(&m->cs);
    return 0;
}

// ── Once ──────────────────────────────────────────────────────────────────

extern "C" int pthread_once(pthread_once_t* o, void (*init)()) {
    if (InterlockedCompareExchange(&o->done, 1, 0) == 0)
        init();
    return 0;
}

// ── TLS key (used by emutls for per-key storage) ──────────────────────────

extern "C" int pthread_key_create(pthread_key_t* key, void (*/*dtor*/)(void*)) {
    DWORD slot = TlsAlloc();
    if (slot == TLS_OUT_OF_INDEXES) return 1;
    *key = slot;
    return 0;
}
extern "C" int pthread_key_delete(pthread_key_t key) {
    TlsFree(key);
    return 0;
}
extern "C" void* pthread_getspecific(pthread_key_t key) {
    return TlsGetValue(key);
}
extern "C" int pthread_setspecific(pthread_key_t key, const void* val) {
    return TlsSetValue(key, const_cast<void*>(val)) ? 0 : 1;
}
