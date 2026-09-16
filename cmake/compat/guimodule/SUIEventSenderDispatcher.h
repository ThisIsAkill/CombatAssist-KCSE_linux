#pragma once
// MinGW compat stub: the real template contains std::map which is 48 bytes
// under libstdc++ vs 16 bytes under MSVC, causing the size assertion to fail.
// CombatAssist doesn't use SUIEventSenderDispatcher; provide a hollow stub.
template<class TEventType>
struct SUIEventSenderDispatcher {};
