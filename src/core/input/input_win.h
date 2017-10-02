#pragma once

/*
- Pause will send 0xE11D then 0x45, the value I use is 0xE11D45. You'll need to remember reading 0xE11D to be able to differenciate between "Pause" and "Num lock"
- Print screen will send 0xE02A then 0xE037
- Insert, Delete, Home, End, Page Up, Page Down, Up, Down, Left, Right: when Num Lock is on or one or both shift keys are pressed, 0xE02A, 0xE0AA, 0xE036, 0xE0B6 can be as postfix or prefix
- Num pad divide and one or both shift are pressed: 0xE02A, 0xE0AA, 0xE036, 0xE0B6 can be as postfix or prefix
*/


static const unsigned int MAX_MAKECODE = 0xff;
static const unsigned int SC_E0 = 0xE000; //E0 bits
static const unsigned int SC_E1 = 0xE100; //E1 bits
static const unsigned int SC_PAUSE_PART1 = 0xE11D; //The pause scancode consists from 2 parts
static const unsigned int SC_PAUSE_PART2 = 0x45; //WM_INPUT with 0xE11D and one WM_INPUT with 0x45

static const unsigned int SC_IGNORE1 = 0xE02A; //0xE02A: first part of the Print Screen scancode if no Shift, Control or Alt keys are pressed
static const unsigned int SC_IGNORE2 = 0xE0AA; //0xE02A, 0xE0AA, 0xE036, 0xE0B6: generated in addition of Insert, Delete, Home, End, Page Up, Page Down, Up, Down, Left, Right when num lock is on; or when num lock is off but one or both shift keys are pressed
static const unsigned int SC_IGNORE3 = 0xE036; //0xE02A, 0xE0AA, 0xE036, 0xE0B6: generated in addition of Numpad Divide and one or both Shift keys are pressed
static const unsigned int SC_IGNORE4 = 0xE0B6; //some of those a break scancode, so we ignore them


enum Scancode
{
	sc_escape =				0x01,
	sc_1 =					0x02,
	sc_2 =					0x03,
	sc_3 =					0x04,
	sc_4 =					0x05,
	sc_5 =					0x06,
	sc_6 =					0x07,
	sc_7 =					0x08,
	sc_8 =					0x09,
	sc_9 =					0x0A,
	sc_0 =					0x0B,
	sc_minus =				0x0C,
	sc_equals =				0x0D,
	sc_backspace =			0x0E,
	sc_tab =				0x0F,
	sc_q =					0x10,
	sc_w =					0x11,
	sc_e =					0x12,
	sc_r =					0x13,
	sc_t =					0x14,
	sc_y =					0x15,
	sc_u =					0x16,
	sc_i =					0x17,
	sc_o =					0x18,
	sc_p =					0x19,
	sc_bracketLeft =		0x1A,
	sc_bracketRight =		0x1B,
	sc_enter =				0x1C,
	sc_controlLeft =		0x1D,
	sc_a =					0x1E,
	sc_s =					0x1F,
	sc_d =					0x20,
	sc_f =					0x21,
	sc_g =					0x22,
	sc_h =					0x23,
	sc_j =					0x24,
	sc_k =					0x25,
	sc_l =					0x26,
	sc_semicolon =			0x27,
	sc_apostrophe =			0x28,
	sc_grave =				0x29,
	sc_shiftLeft =			0x2A,
	sc_backslash =			0x2B,
	sc_z =					0x2C,
	sc_x =					0x2D,
	sc_c =					0x2E,
	sc_v =					0x2F,
	sc_b =					0x30,
	sc_n =					0x31,
	sc_m =					0x32,
	sc_comma =				0x33,
	sc_preiod =				0x34,
	sc_slash =				0x35,
	sc_shiftRight =			0x36,
	sc_numpad_multiply =	0x37,
	sc_altLeft =			0x38,
	sc_space =				0x39,
	sc_capsLock =			0x3A,
	sc_f1 =					0x3B,
	sc_f2 =					0x3C,
	sc_f3 =					0x3D,
	sc_f4 =					0x3E,
	sc_f5 =					0x3F,
	sc_f6 =					0x40,
	sc_f7 =					0x41,
	sc_f8 =					0x42,
	sc_f9 =					0x43,
	sc_f10 =				0x44,
	sc_numLock =			0x45,
	sc_scrollLock =			0x46,
	sc_numpad_7 =			0x47,
	sc_numpad_8 =			0x48,
	sc_numpad_9 =			0x49,
	sc_numpad_minus =		0x4A,
	sc_numpad_4 =			0x4B,
	sc_numpad_5 =			0x4C,
	sc_numpad_6 =			0x4D,
	sc_numpad_plus =		0x4E,
	sc_numpad_1 =			0x4F,
	sc_numpad_2 =			0x50,
	sc_numpad_3 =			0x51,
	sc_numpad_0 =			0x52,
	sc_numpad_period =		0x53,
	sc_alt_printScreen =	0x54, /* Alt + print screen. MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54. */
	sc_bracketAngle =		0x56, /* Key between the left shift and Z. */
	sc_f11 =				0x57,
	sc_f12 =				0x58,
	sc_oem_1 =				0x5a, /* VK_OEM_WSCTRL */
	sc_oem_2 =				0x5b, /* VK_OEM_FINISH */
	sc_oem_3 =				0x5c, /* VK_OEM_JUMP */
	sc_eraseEOF =			0x5d,
	sc_oem_4 =				0x5e, /* VK_OEM_BACKTAB */
	sc_oem_5 =				0x5f, /* VK_OEM_AUTO */
	sc_zoom =				0x62,
	sc_help =				0x63,
	sc_f13 =				0x64,
	sc_f14 =				0x65,
	sc_f15 =				0x66,
	sc_f16 =				0x67,
	sc_f17 =				0x68,
	sc_f18 =				0x69,
	sc_f19 =				0x6a,
	sc_f20 =				0x6b,
	sc_f21 =				0x6c,
	sc_f22 =				0x6d,
	sc_f23 =				0x6e,
	sc_oem_6 =				0x6f, /* VK_OEM_PA3 */
	sc_katakana =			0x70,
	sc_oem_7 =				0x71, /* VK_OEM_RESET */
	sc_f24 =				0x76,
	sc_sbcschar =			0x77,
	sc_convert =			0x79,
	sc_nonconvert =			0x7B, /* VK_OEM_PA1 */

	sc_media_previous =		0xE010,
	sc_media_next =			0xE019,
	sc_numpad_enter =		0xE01C,
	sc_controlRight =		0xE01D,
	sc_volume_mute =		0xE020,
	sc_launch_app2 =		0xE021,
	sc_media_play =			0xE022,
	sc_media_stop =			0xE024,
	sc_volume_down =		0xE02E,
	sc_volume_up =			0xE030,
	sc_browser_home =		0xE032,
	sc_numpad_divide =		0xE035,
	sc_printScreen =		0xE037,
	/*
	sc_printScreen:
	- make: 0xE02A 0xE037
	- break: 0xE0B7 0xE0AA
	- MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54;
	- There is no VK_KEYDOWN with VK_SNAPSHOT.
	*/
	sc_altRight =			0xE038,
	sc_cancel =				0xE046, /* CTRL + Pause */
	sc_home =				0xE047,
	sc_arrowUp =			0xE048,
	sc_pageUp =				0xE049,
	sc_arrowLeft =			0xE04B,
	sc_arrowRight =			0xE04D,
	sc_end =				0xE04F,
	sc_arrowDown =			0xE050,
	sc_pageDown =			0xE051,
	sc_insert =				0xE052,
	sc_delete =				0xE053,
	sc_metaLeft =			0xE05B,
	sc_metaRight =			0xE05C,
	sc_application =		0xE05D,
	sc_power =				0xE05E,
	sc_sleep =				0xE05F,
	sc_wake =				0xE063,
	sc_browser_search =		0xE065,
	sc_browser_favorites =	0xE066,
	sc_browser_refresh =	0xE067,
	sc_browser_stop =		0xE068,
	sc_browser_forward =	0xE069,
	sc_browser_back =		0xE06A,
	sc_launch_app1 =		0xE06B,
	sc_launch_email =		0xE06C,
	sc_launch_media =		0xE06D,

	sc_pause =				0xE11D45,
	/*
	sc_pause:
	- make: 0xE11D 45 0xE19D C5
	- make in raw input: 0xE11D 0x45
	- break: none
	- No repeat when you hold the key down
	- There are no break so I don't know how the key down/up is expected to work. Raw input sends "keydown" and "keyup" messages, and it appears that the keyup message is sent directly after the keydown message (you can't hold the key down) so depending on when GetMessage or PeekMessage will return messages, you may get both a keydown and keyup message "at the same time". If you use VK messages most of the time you only get keydown messages, but some times you get keyup messages too.
	- when pressed at the same time as one or both control keys, generates a 0xE046 (sc_cancel) and the string for that scancode is "break".
	*/
};


enum class Scancode_USB_HID
{
	SystemPower =				0x81, //0xE05E
	SystemSleep =				0x82, //0xE05F
	SystemWake =				0x83, //0xE063

	None =						0x00, //None
	Overrun_Error =				0x01, //FF
	POST_Fail =					0x02, //FC
	ErrorUndefined =			0x03, //NA
	a =							0x04, //1E
	b =							0x05, //30
	c =							0x06, //2E
	d =							0x07, //20
	e =							0x08, //12
	f =							0x09, //21
	g =							0x0A, //22
	h =							0x0B, //23
	i =							0x0C, //17
	j =							0x0D, //24
	k =							0x0E, //25
	l =							0x0F, //26
	m =							0x10, //32
	n =							0x11, //31
	o =							0x12, //18
	p =							0x13, //19
	q =							0x14, //10
	r =							0x15, //13
	s =							0x16, //1F
	t =							0x17, //14
	u =							0x18, //16
	v =							0x19, //2F
	w =							0x1A, //11
	x =							0x1B, //2D
	y =							0x1C, //15
	z =							0x1D, //2C
	n1 =						0x1E, //02
	n2 =						0x1F, //03
	n3 =						0x20, //04
	n4 =						0x21, //05
	n5 =						0x22, //06
	n6 =						0x23, //07
	n7 =						0x24, //08
	n8 =						0x25, //09
	n9 =						0x26, //0A
	n0 =						0x27,//0B
	Return =					0x28, //1C
	Escape =					0x29, //01
	Backspace =					0x2A, //0E
	Tab =						0x2B, //0F
	Space =						0x2C, //39
	Minus =						0x2D, //0C
	Equals =					0x2E, //0D
	BracketLeft =				0x2F, //1A
	BracketRight =				0x30, //1B
	Backslash =					0x31, //2B
	Europe1 =					0x32, //WTF?
	Semicolon =					0x33, //27
	Apostrophe =				0x34, //28
	Grave =						0x35, //29
	Comma =						0x36, //33
	Period =					0x37, //34
	Slash =						0x38, //35
	CapsLock =					0x39, //3A
	F1 =						0x3A, //3B
	F2 =						0x3B, //3C
	F3 =						0x3C, //3D
	F4 =						0x3D, //3E
	F5 =						0x3E, //3F
	F6 =						0x3F, //40
	F7 =						0x40, //41
	F8 =						0x41, //42
	F9 =						0x42, //43
	F10 =						0x43, //44
	F11 =						0x44, //57
	F12 =						0x45, //58
	PrintScreen =				0x46, //E037
	ScrollLock =				0x47, //46
	Break =						0x48, //E046
	Pause =						0x48, //E11D45
	Insert =					0x49, //E052
	Home =						0x4A, //E047
	PageUp =					0x4B, //E049
	Delete =					0x4C, //E053
	End =						0x4D, //E04F
	PageDown =					0x4E, //E051
	RightArrow =				0x4F, //E04D
	LeftArrow =					0x50, //E04B
	DownArrow =					0x51, //E050
	UpArrow =					0x52, //E048
	NumLock =					0x53, //45
	NumpadSlash =				0x54, //E035
	NumpadMultiply =			0x55, //37
	NumpadMinus =				0x56, //4A
	NumpadPlus =				0x57, //4E
	NumpadEnter =				0x58, //E01C
	Numpad1 =					0x59, //4F
	Numpad2 =					0x5A, //50
	Numpad3 =					0x5B, //51
	Numpad4 =					0x5C, //4B
	Numpad5 =					0x5D, //4C
	Numpad6 =					0x5E, //4D
	Numpad7 =					0x5F, //47
	Numpad8 =					0x60, //48
	Numpad9 =					0x61, //49
	Numpad0 =					0x62, //52
	NumpadPeriod =				0x63, //53
	Europe2 =					0x64, //56
	App =						0x65, //E05D
	KeyboardPower =				0x66, //E05E
	NumpadResolution =			0x67, //59
	F13 =						0x68, //64
	F14 =						0x69, //65
	F15 =						0x6A, //66
	F16 =						0x6B, //67
	F17 =						0x6C, //68
	F18 =						0x6D, //69
	F19 =						0x6E, //6A
	F20 =						0x6F, //6B
	F21 =						0x70, //6C
	F22 =						0x71, //6D
	F23 =						0x72, //6E
	F24 =						0x73, //76
	KeyboardExecute =			0x74, //UNASSIGNED
	KeyboardHelp =				0x75, //UNASSIGNED
	KeyboardMenu =				0x76, //UNASSIGNED
	KeyboardSelect =			0x77, //UNASSIGNED
	KeyboardStop =				0x78, //UNASSIGNED
	KeyboardAgain =				0x79, //UNASSIGNED
	KeyboardUndo =				0x7A, //UNASSIGNED
	KeyboardCut =				0x7B, //UNASSIGNED
	KeyboardCopy =				0x7C, //UNASSIGNED
	KeyboardPaste =				0x7D, //UNASSIGNED
	KeyboardFind =				0x7E, //UNASSIGNED
	KeyboardMute =				0x7F, //UNASSIGNED
	KeyboardVolumeUp =			0x80, //UNASSIGNED
	KeyboardVolumeDn =			0x81, //UNASSIGNED
	KeyboardLockingCapsLock =	0x82, //UNASSIGNED
	KeyboardLockingNumLock =	0x83, //UNASSIGNED
	KeyboardLockingScrollLock = 0x84, //UNASSIGNED
	NumPadComa =				0x85, //7E
	NumLockEquals =				0x86, //UNASSIGNED
	KeyboardIntl1 =				0x87, //73
	KeyboardIntl2 =				0x88, //70
	KeyboardIntl2 =				0x89, //7D
	KeyboardIntl4 =				0x8A, //79
	KeyboardIntl5 =				0x8B, //7B
	KeyboardIntl6 =				0x8C, //5C
	KeyboardIntl7 =				0x8D, //UNASSIGNED
	KeyboardIntl8 =				0x8E, //UNASSIGNED
	KeyboardIntl9 =				0x8F, //UNASSIGNED
	KeyboardLang1 =				0x90, //F2
	KeyboardLang2 =				0x91, //F1
	KeyboardLang3 =				0x92, //78
	KeyboardLang4 =				0x93, //77
	KeyboardLang5 =				0x94, //76
	KeyboardLang6 =				0x95, //UNASSIGNED
	KeyboardLang7 =				0x96, //UNASSIGNED
	KeyboardLang8 =				0x97, //UNASSIGNED
	KeyboardLang9 =				0x98, //UNASSIGNED
	KeyboardAlternateErase =	0x99, //UNASSIGNED
	KeyboardSysReq_Attention =	0x9A, //UNASSIGNED
	KeyboardCancel =			0x9B, //UNASSIGNED
	KeyboardClear =				0x9C, //UNASSIGNED
	KeyboardPrior =				0x9D, //UNASSIGNED
	KeyboardReturn =			0x9E, //UNASSIGNED
	KeyboardSeparator =			0x9F, //UNASSIGNED
	KeyboardOut =				0xA0, //UNASSIGNED
	KeyboardOper =				0xA1, //UNASSIGNED
	KeyboardClear_Again =		0xA2, //UNASSIGNED
	KeyboardCrSel_Props =		0xA3, //UNASSIGNED
	KeyboardExSel =				0xA4, //UNASSIGNED
	//RESERVED A5 - DF //RESERVED
	LeftControl =				0xE0, //1D
	LeftShift =					0xE1, //2A
	LeftAlt =					0xE2, //38
	LeftGUI =					0xE3, //E05B
	RightControl =				0xE4, //E01D
	RightShift =				0xE5, //36
	RightAlt =					0xE6, //E038
	RightGUI =					0xE7, //E05C
	//RESERVED = 0xE8 - FFFF RESERVED RESERVED RESERVED RESERVED
	
	ScanNextTrack =				0x00B5, //E019
	ScanPreviousTrack =			0x00B6, //E010
	Stop =						0x00B7, //E024
	Play_Pause =				0x00CD, //E022
	Mute =						0x00E2, //E020
	BassBoost =					0x00E5, //UNASSIGNED
	Loudness =					0x00E7, //UNASSIGNED
	VolumeUp =					0x00E9, //E030
	VolumeDown =				0x00EA, //E02E
	BassUp =					0x0152, //UNASSIGNED
	BassDown =					0x0153, //UNASSIGNED
	TrebleUp =					0x0154, //UNASSIGNED
	TrebleDown =				0x0155, //UNASSIGNED
	MediaSelect =				0x0183, //E06D
	Mail =						0x018A, //E06C
	Calculator =				0x0192, //E021
	MyComputer =				0x0194, //E06B
	WWWSearch =					0x0221, //E065
	WWWHome =					0x0223, //E032
	WWWBack =					0x0224, //E06A
	WWWForward =				0x0225, //E069
	WWWStop =					0x0226, //E068
	WWWRefresh =				0x0227, //E067
	WWWFavorites =				0x022A, //E066
};


/*
- bit 16 - 23 contains the first byte of the scancode
- bit 24 indicates that the scancode is 2 bytes (extended)
*/
inline unsigned int getScancodeName(unsigned int scancode, char* buffer, unsigned int bufferLength)
{

	unsigned int result = 0;
	unsigned int extended = scancode & 0xffff00;
	unsigned int lParam = 0;

	if(extended != 0)
	{
		if(extended == 0xE11D00)
			lParam = 0x45 << 16;
		else
			lParam = (0x100 | (scancode & 0xff)) << 16;
	}
	else
	{
		lParam = scancode << 16;

		if(scancode == 0x45)
			lParam |= (0x1 << 24);
	}

	result = GetKeyNameText(lParam, buffer, bufferLength);
	return result;
}

/*
potrebujem:
handle WM_KEYDOWN, WM_KEYUP pre VK ?
handle WM_INPUT pre raw input
scancode -> USB HID
USB HID -> key name

*/