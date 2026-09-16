// CryEngine stubs for MinGW cross-compilation.
// CryFatalError is declared in smartptr.h but is normally satisfied by the game
// binary or the MSVC CRT.  In a healthy plugin it is never reached; provide a
// hard-abort so crashes are obvious if the ref-counting logic ever fires.
#include <cstdlib>
void CryFatalError(const char* /*fmt*/, ...) { __builtin_abort(); }
