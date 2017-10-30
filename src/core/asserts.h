#include <windows.h>


#define DEBUG_BREAK() __debugbreak()
#define DEBUGGER_LOG(text) OutputDebugString(text)


#define ASSERT(cond) if(!(cond)) { DEBUG_BREAK(); DEBUGGER_LOG(#cond); }
#define ASSERT2(cond, text) if(!(cond)) { DEBUG_BREAK(); DEBUGGER_LOG(text); }