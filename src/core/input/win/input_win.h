#pragma once

#include "core/int.h"

struct tagRAWKEYBOARD;
typedef tagRAWKEYBOARD RAWKEYBOARD;
struct tagRAWMOUSE;
typedef tagRAWMOUSE RAWMOUSE;


namespace Veng
{


namespace Input
{


namespace Keyboard
{


/*
- Pause will send 0xE11D then 0x45, the value I use is 0xE11D45. You'll need to remember reading 0xE11D to be able to differenciate between "Pause" and "Num lock"
- Print screen will send 0xE02A then 0xE037
- Insert, Delete, Home, End, Page Up, Page Down, Up, Down, Left, Right: when Num Lock is on or one or both shift keys are pressed, 0xE02A, 0xE0AA, 0xE036, 0xE0B6 can be as postfix or prefix
- Num pad divide and one or both shift are pressed: 0xE02A, 0xE0AA, 0xE036, 0xE0B6 can be as postfix or prefix
*/

//Scancodes PS/2 version 1
namespace Scancode_PS2
{

static const unsigned int MAX_MAKECODE = 0xff;
static const unsigned int E0 = 0xE000; //E0 bits
static const unsigned int E1 = 0xE100; //E1 bits
static const unsigned int PAUSE_PART1 = 0xE11D; //The pause scancode consists from 2 parts
static const unsigned int PAUSE_PART2 = 0x45; //WM_INPUT with 0xE11D and one WM_INPUT with 0x45

static const unsigned int IGNORE1 = 0xE02A; //0xE02A: first part of the Print Screen scancode if no Shift, Control or Alt keys are pressed
static const unsigned int IGNORE2 = 0xE0AA; //0xE02A, 0xE0AA, 0xE036, 0xE0B6: generated in addition of Insert, Delete, Home, End, Page Up, Page Down, Up, Down, Left, Right when num lock is on; or when num lock is off but one or both shift keys are pressed
static const unsigned int IGNORE3 = 0xE036; //0xE02A, 0xE0AA, 0xE036, 0xE0B6: generated in addition of Numpad Divide and one or both Shift keys are pressed
static const unsigned int IGNORE4 = 0xE0B6; //some of those a break scancode, so we ignore them

enum : u32
{
	None = 0x00,
	Escape = 0x01,
	N1 = 0x02,
	N2 = 0x03,
	N3 = 0x04,
	N4 = 0x05,
	N5 = 0x06,
	N6 = 0x07,
	N7 = 0x08,
	N8 = 0x09,
	N9 = 0x0A,
	N0 = 0x0B,
	Minus = 0x0C,
	Equals = 0x0D,
	Backspace = 0x0E,
	Tab = 0x0F,
	Q = 0x10,
	W = 0x11,
	E = 0x12,
	R = 0x13,
	T = 0x14,
	Y = 0x15,
	U = 0x16,
	I = 0x17,
	O = 0x18,
	P = 0x19,
	BracketLeft = 0x1A,
	BracketRight = 0x1B,
	Enter = 0x1C,
	ControlLeft = 0x1D,
	A = 0x1E,
	S = 0x1F,
	D = 0x20,
	F = 0x21,
	G = 0x22,
	H = 0x23,
	J = 0x24,
	K = 0x25,
	L = 0x26,
	Semicolon = 0x27,
	Apostrophe = 0x28,
	Grave = 0x29,
	ShiftLeft = 0x2A,
	Backslash = 0x2B,
	Z = 0x2C,
	X = 0x2D,
	C = 0x2E,
	V = 0x2F,
	B = 0x30,
	N = 0x31,
	M = 0x32,
	Comma = 0x33,
	Period = 0x34,
	Slash = 0x35,
	ShiftRight = 0x36,
	NumpadMultiply = 0x37,
	AltLeft = 0x38,
	Space = 0x39,
	CapsLock = 0x3A,
	F1 = 0x3B,
	F2 = 0x3C,
	F3 = 0x3D,
	F4 = 0x3E,
	F5 = 0x3F,
	F6 = 0x40,
	F7 = 0x41,
	F8 = 0x42,
	F9 = 0x43,
	F10 = 0x44,
	NumLock = 0x45,
	ScrollLock = 0x46,
	Numpad7 = 0x47,
	Numpad8 = 0x48,
	Numpad9 = 0x49,
	NumpadMinus = 0x4A,
	Numpad4 = 0x4B,
	Numpad5 = 0x4C,
	Numpad6 = 0x4D,
	NumpadPlus = 0x4E,
	Numpad1 = 0x4F,
	Numpad2 = 0x50,
	Numpad3 = 0x51,
	Numpad0 = 0x52,
	NumpadPeriod = 0x53,
	AltPrintScreen = 0x54, /* Alt + print screen. MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54. */
	BracketAngle = 0x56, /* Key between the left shift and Z. */
	F11 = 0x57,
	F12 = 0x58,
	Oem1 = 0x5A, /* VK_OEM_WSCTRL */
	Oem2 = 0x5B, /* VK_OEM_FINISH */
	Oem3 = 0x5C, /* VK_OEM_JUMP */
	EraseEOF = 0x5D,
	Oem4 = 0x5E, /* VK_OEM_BACKTAB */
	Oem5 = 0x5F, /* VK_OEM_AUTO */
	Zoom = 0x62,
	Help = 0x63,
	F13 = 0x64,
	F14 = 0x65,
	F15 = 0x66,
	F16 = 0x67,
	F17 = 0x68,
	F18 = 0x69,
	F19 = 0x6A,
	F20 = 0x6B,
	F21 = 0x6C,
	F22 = 0x6D,
	F23 = 0x6E,
	Oem6 = 0x6F, /* VK_OEM_PA3 */
	Katakana = 0x70,
	Oem7 = 0x71, /* VK_OEM_RESET */
	F24 = 0x76,
	Sbcschar = 0x77,
	Convert = 0x79,
	Nonconvert = 0x7B, /* VK_OEM_PA1 */

	MediaPrevious = 0xE010,
	MediaNext = 0xE019,
	NumpadEnter = 0xE01C,
	ControlRight = 0xE01D,
	VolumeMute = 0xE020,
	LaunchApp2 = 0xE021,
	MediaPlay = 0xE022,
	MediaStop = 0xE024,
	VolumeDown = 0xE02E,
	VolumeUp = 0xE030,
	BrowserHome = 0xE032,
	NumpadDivide = 0xE035,
	PrintScreen = 0xE037,
	/*
	PrintScreen:
	- make: 0xE02A 0xE037
	- break: 0xE0B7 0xE0AA
	- MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54;
	- There is no VK_KEYDOWN with VK_SNAPSHOT.
	*/
	AltRight = 0xE038,
	Break = 0xE046, /* CTRL + Pause */
	Home = 0xE047,
	ArrowUp = 0xE048,
	PageUp = 0xE049,
	ArrowLeft = 0xE04B,
	ArrowRight = 0xE04D,
	End = 0xE04F,
	ArrowDown = 0xE050,
	PageDown = 0xE051,
	Insert = 0xE052,
	Delete = 0xE053,
	MetaLeft = 0xE05B,
	MetaRight = 0xE05C,
	Application = 0xE05D,
	Power = 0xE05E,
	Sleep = 0xE05F,
	Wake = 0xE063,
	BrowserSearch = 0xE065,
	BrowserFavorites = 0xE066,
	BrowserRefresh = 0xE067,
	BrowserStop = 0xE068,
	BrowserForward = 0xE069,
	BrowserBack = 0xE06A,
	LaunchApp1 = 0xE06B,
	LaunchEmail = 0xE06C,
	LaunchMedia = 0xE06D,

	Pause = 0xE11D45,
	/*
	Pause:
	- make: 0xE11D 45 0xE19D C5
	- make in raw input: 0xE11D 0x45
	- break: none
	- No repeat when you hold the key down
	- There are no break so I don't know how the key down/up is expected to work. Raw input sends "keydown" and "keyup" messages, and it appears that the keyup message is sent directly after the keydown message (you can't hold the key down) so depending on when GetMessage or PeekMessage will return messages, you may get both a keydown and keyup message "at the same time". If you use VK messages most of the time you only get keydown messages, but some times you get keyup messages too.
	- when pressed at the same time as one or both control keys, generates a 0xE046 (sc_cancel) and the string for that scancode is "break".
	*/
};

u32 FromUSBHID(u8 scUSB);

}


namespace Scancode_USBHID
{

enum : u8
{
	//HID Usage Page 07
	None = 0x00, //None
	Overrun_Error = 0x01, //FF
	POST_Fail = 0x02, //FC
	ErrorUndefined = 0x03, //NA
	A = 0x04, //1E
	B = 0x05, //30
	C = 0x06, //2E
	D = 0x07, //20
	E = 0x08, //12
	F = 0x09, //21
	G = 0x0A, //22
	H = 0x0B, //23
	I = 0x0C, //17
	J = 0x0D, //24
	K = 0x0E, //25
	L = 0x0F, //26
	M = 0x10, //32
	N = 0x11, //31
	O = 0x12, //18
	P = 0x13, //19
	Q = 0x14, //10
	R = 0x15, //13
	S = 0x16, //1F
	T = 0x17, //14
	U = 0x18, //16
	V = 0x19, //2F
	W = 0x1A, //11
	X = 0x1B, //2D
	Y = 0x1C, //15
	Z = 0x1D, //2C
	N1 = 0x1E, //02
	N2 = 0x1F, //03
	N3 = 0x20, //04
	N4 = 0x21, //05
	N5 = 0x22, //06
	N6 = 0x23, //07
	N7 = 0x24, //08
	N8 = 0x25, //09
	N9 = 0x26, //0A
	N0 = 0x27, //0B
	Return = 0x28, //1C
	Escape = 0x29, //01
	Backspace = 0x2A, //0E
	Tab = 0x2B, //0F
	Space = 0x2C, //39
	Minus = 0x2D, //0C
	Equals = 0x2E, //0D
	BracketLeft = 0x2F, //1A
	BracketRight = 0x30, //1B
	Backslash = 0x31, //2B
	Europe1 = 0x32, //WTF?
	Semicolon = 0x33, //27
	Apostrophe = 0x34, //28
	Grave = 0x35, //29
	Comma = 0x36, //33
	Period = 0x37, //34
	Slash = 0x38, //35
	CapsLock = 0x39, //3A
	F1 = 0x3A, //3B
	F2 = 0x3B, //3C
	F3 = 0x3C, //3D
	F4 = 0x3D, //3E
	F5 = 0x3E, //3F
	F6 = 0x3F, //40
	F7 = 0x40, //41
	F8 = 0x41, //42
	F9 = 0x42, //43
	F10 = 0x43, //44
	F11 = 0x44, //57
	F12 = 0x45, //58
	PrintScreen = 0x46, //E037
	ScrollLock = 0x47, //46
	Break = 0x48, //E046
	Pause = 0x48, //E11D45
	Insert = 0x49, //E052
	Home = 0x4A, //E047
	PageUp = 0x4B, //E049
	Delete = 0x4C, //E053
	End = 0x4D, //E04F
	PageDown = 0x4E, //E051
	ArrowRight = 0x4F, //E04D
	ArrowLeft = 0x50, //E04B
	ArrowDown = 0x51, //E050
	ArrowUp = 0x52, //E048
	NumLock = 0x53, //45
	NumpadDivide = 0x54, //E035
	NumpadMultiply = 0x55, //37
	NumpadMinus = 0x56, //4A
	NumpadPlus = 0x57, //4E
	NumpadEnter = 0x58, //E01C
	Numpad1 = 0x59, //4F
	Numpad2 = 0x5A, //50
	Numpad3 = 0x5B, //51
	Numpad4 = 0x5C, //4B
	Numpad5 = 0x5D, //4C
	Numpad6 = 0x5E, //4D
	Numpad7 = 0x5F, //47
	Numpad8 = 0x60, //48
	Numpad9 = 0x61, //49
	Numpad0 = 0x62, //52
	NumpadPeriod = 0x63, //53
	Europe2 = 0x64, //56
	Application = 0x65, //E05D
	KeyboardPower = 0x66, //E05E
	NumpadResolution = 0x67, //59
	F13 = 0x68, //64
	F14 = 0x69, //65
	F15 = 0x6A, //66
	F16 = 0x6B, //67
	F17 = 0x6C, //68
	F18 = 0x6D, //69
	F19 = 0x6E, //6A
	F20 = 0x6F, //6B
	F21 = 0x70, //6C
	F22 = 0x71, //6D
	F23 = 0x72, //6E
	F24 = 0x73, //76
	KeyboardExecute = 0x74, //UNASSIGNED
	KeyboardHelp = 0x75, //UNASSIGNED
	KeyboardMenu = 0x76, //UNASSIGNED
	KeyboardSelect = 0x77, //UNASSIGNED
	KeyboardStop = 0x78, //UNASSIGNED
	KeyboardAgain = 0x79, //UNASSIGNED
	KeyboardUndo = 0x7A, //UNASSIGNED
	KeyboardCut = 0x7B, //UNASSIGNED
	KeyboardCopy = 0x7C, //UNASSIGNED
	KeyboardPaste = 0x7D, //UNASSIGNED
	KeyboardFind = 0x7E, //UNASSIGNED
	KeyboardMute = 0x7F, //UNASSIGNED
	KeyboardVolumeUp = 0x80, //UNASSIGNED
	KeyboardVolumeDn = 0x81, //UNASSIGNED
	KeyboardLockingCapsLock = 0x82, //UNASSIGNED
	KeyboardLockingNumLock = 0x83, //UNASSIGNED
	KeyboardLockingScrollLock = 0x84, //UNASSIGNED
	NumpadComa = 0x85, //7E
	KeyboardEquals = 0x86, //UNASSIGNED
	KeyboardIntl1 = 0x87, //73
	KeyboardIntl2 = 0x88, //70
	KeyboardIntl3 = 0x89, //7D
	KeyboardIntl4 = 0x8A, //79
	KeyboardIntl5 = 0x8B, //7B
	KeyboardIntl6 = 0x8C, //5C
	KeyboardIntl7 = 0x8D, //UNASSIGNED
	KeyboardIntl8 = 0x8E, //UNASSIGNED
	KeyboardIntl9 = 0x8F, //UNASSIGNED
	KeyboardLang1 = 0x90, //F2
	KeyboardLang2 = 0x91, //F1
	KeyboardLang3 = 0x92, //78
	KeyboardLang4 = 0x93, //77
	KeyboardLang5 = 0x94, //76
	KeyboardLang6 = 0x95, //UNASSIGNED
	KeyboardLang7 = 0x96, //UNASSIGNED
	KeyboardLang8 = 0x97, //UNASSIGNED
	KeyboardLang9 = 0x98, //UNASSIGNED
	KeyboardAlternateErase = 0x99, //UNASSIGNED
	KeyboardSysReq_Attention = 0x9A, //UNASSIGNED
	KeyboardCancel = 0x9B, //UNASSIGNED
	KeyboardClear = 0x9C, //UNASSIGNED
	KeyboardPrior = 0x9D, //UNASSIGNED
	KeyboardReturn = 0x9E, //UNASSIGNED
	KeyboardSeparator = 0x9F, //UNASSIGNED
	KeyboardOut = 0xA0, //UNASSIGNED
	KeyboardOper = 0xA1, //UNASSIGNED
	KeyboardClear_Again = 0xA2, //UNASSIGNED
	KeyboardCrSel_Props = 0xA3, //UNASSIGNED
	KeyboardExSel = 0xA4, //UNASSIGNED
	//RESERVED A5 - DF
	ControlLeft = 0xE0, //1D
	ShiftLeft = 0xE1, //2A
	AltLeft = 0xE2, //38
	GUILeft = 0xE3, //E05B
	ControlRight = 0xE4, //E01D
	ShiftRight = 0xE5, //36
	AltRight = 0xE6, //E038
	GUIRight = 0xE7, //E05C
	//RESERVED = 0xE8 - FFFF

	//Next values should be reserved, but we use it because of HID Usage Page conflicts

	//HID Usage Page 01
	SystemPower = 0xA5, // original value is 0x81, //0xE05E
	SystemSleep = 0xA6, // original value is 0x82, //0xE05F
	SystemWake = 0xA7, // original value is 0x83, //0xE063

	//HID Usage Page 0C
	ScanNextTrack = 0xA8, // original value is 0x00B5, //E019
	ScanPreviousTrack = 0xA9, // original value is 0x00B6, //E010
	Stop = 0xAA, // original value is 0x00B7, //E024
	Play_Pause = 0xAB, // original value is 0x00CD, //E022
	Mute = 0xAC, // original value is 0x00E2, //E020
	BassBoost = 0xAD, // original value is 0x00E5, //UNASSIGNED
	Loudness = 0xAE, // original value is 0x00E7, //UNASSIGNED
	VolumeUp = 0xAF, // original value is 0x00E9, //E030
	VolumeDown = 0xB0, // original value is 0x00EA, //E02E
	BassUp = 0xB1, // original value is 0x0152, //UNASSIGNED
	BassDown = 0xB2, // original value is 0x0153, //UNASSIGNED
	TrebleUp = 0xB3, // original value is 0x0154, //UNASSIGNED
	TrebleDown = 0xB4, // original value is 0x0155, //UNASSIGNED
	MediaSelect = 0xB5, // original value is 0x0183, //E06D
	Mail = 0xB6, // original value is 0x018A, //E06C
	Calculator = 0xB7, // original value is 0x0192, //E021
	MyComputer = 0xB8, // original value is 0x0194, //E06B
	WWWSearch = 0xB9, // original value is 0x0221, //E065
	WWWHome = 0xBA, // original value is 0x0223, //E032
	WWWBack = 0xBB, // original value is 0x0224, //E06A
	WWWForward = 0xBC, // original value is 0x0225, //E069
	WWWStop = 0xBD, // original value is 0x0226, //E068
	WWWRefresh = 0xBE, // original value is 0x0227, //E067
	WWWFavorites = 0xBF, // original value is 0x022A, //E066
};

u8 FromPS2(u32 scPS2);

}


u32 GetScancodePS2FromRawInput(const RAWKEYBOARD& keyboard);

i32 GetUTF16TextFromRawInput(const RAWKEYBOARD& keyboard, wchar_t* buffer, u32 bufferSize);

u32 GetScancodePS2Name(u32 scancodePS2, char* buffer, u32 bufferLength);


}


namespace Mouse
{


const u32 BUTTON_NONE = 0x0000;


}


}


}