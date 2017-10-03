#pragma once

#include "input_win.h"
#define VC_EXTRALEAN
#include <windows.h>
#include "core\asserts.h"


static const u32 tablePS2FromUSBHID[] =
{
	Scancode_PS2::None,
	Scancode_PS2::None, //Overrun_Error = 0x01, //FF
	Scancode_PS2::None, //POST_Fail = 0x02, //FC
	Scancode_PS2::None, //ErrorUndefined = 0x03, //NA
	Scancode_PS2::A,
	Scancode_PS2::B,
	Scancode_PS2::C,
	Scancode_PS2::D,
	Scancode_PS2::E,
	Scancode_PS2::F,
	Scancode_PS2::G,
	Scancode_PS2::H,
	Scancode_PS2::I,
	Scancode_PS2::J,
	Scancode_PS2::K,
	Scancode_PS2::L,
	Scancode_PS2::M,
	Scancode_PS2::N,
	Scancode_PS2::O,
	Scancode_PS2::P,
	Scancode_PS2::Q,
	Scancode_PS2::R,
	Scancode_PS2::S,
	Scancode_PS2::T,
	Scancode_PS2::U,
	Scancode_PS2::V,
	Scancode_PS2::W,
	Scancode_PS2::X,
	Scancode_PS2::Y,
	Scancode_PS2::Z,
	Scancode_PS2::N1,
	Scancode_PS2::N2,
	Scancode_PS2::N3,
	Scancode_PS2::N4,
	Scancode_PS2::N5,
	Scancode_PS2::N6,
	Scancode_PS2::N7,
	Scancode_PS2::N8,
	Scancode_PS2::N9,
	Scancode_PS2::N0,
	Scancode_PS2::Enter,
	Scancode_PS2::Escape,
	Scancode_PS2::Backspace,
	Scancode_PS2::Tab,
	Scancode_PS2::Space,
	Scancode_PS2::Minus,
	Scancode_PS2::Equals,
	Scancode_PS2::BracketLeft,
	Scancode_PS2::BracketRight,
	Scancode_PS2::Backslash,
	Scancode_PS2::Backslash, //Europe1
	Scancode_PS2::Semicolon,
	Scancode_PS2::Apostrophe,
	Scancode_PS2::Grave,
	Scancode_PS2::Comma,
	Scancode_PS2::Period,
	Scancode_PS2::Slash,
	Scancode_PS2::CapsLock,
	Scancode_PS2::F1,
	Scancode_PS2::F2,
	Scancode_PS2::F3,
	Scancode_PS2::F4,
	Scancode_PS2::F5,
	Scancode_PS2::F6,
	Scancode_PS2::F7,
	Scancode_PS2::F8,
	Scancode_PS2::F9,
	Scancode_PS2::F10,
	Scancode_PS2::F11,
	Scancode_PS2::F12,
	Scancode_PS2::PrintScreen,
	Scancode_PS2::ScrollLock,
	Scancode_PS2::Pause,
	Scancode_PS2::Insert,
	Scancode_PS2::Home,
	Scancode_PS2::PageUp,
	Scancode_PS2::Delete,
	Scancode_PS2::End,
	Scancode_PS2::PageDown,
	Scancode_PS2::ArrowRight,
	Scancode_PS2::ArrowLeft,
	Scancode_PS2::ArrowDown,
	Scancode_PS2::ArrowUp,
	Scancode_PS2::NumLock,
	Scancode_PS2::NumpadDivide,
	Scancode_PS2::NumpadMultiply,
	Scancode_PS2::NumpadMinus,
	Scancode_PS2::NumpadPlus,
	Scancode_PS2::NumpadEnter,
	Scancode_PS2::Numpad1,
	Scancode_PS2::Numpad2,
	Scancode_PS2::Numpad3,
	Scancode_PS2::Numpad4,
	Scancode_PS2::Numpad5,
	Scancode_PS2::Numpad6,
	Scancode_PS2::Numpad7,
	Scancode_PS2::Numpad8,
	Scancode_PS2::Numpad9,
	Scancode_PS2::Numpad0,
	Scancode_PS2::NumpadPeriod,
	Scancode_PS2::BracketAngle, //Europe2
	Scancode_PS2::Application,
	Scancode_PS2::Power, //KeyboardPower
	Scancode_PS2::None, // NumpadResolution
	Scancode_PS2::F13,
	Scancode_PS2::F14,
	Scancode_PS2::F15,
	Scancode_PS2::F16,
	Scancode_PS2::F17,
	Scancode_PS2::F18,
	Scancode_PS2::F19,
	Scancode_PS2::F20,
	Scancode_PS2::F21,
	Scancode_PS2::F22,
	Scancode_PS2::F23,
	Scancode_PS2::F24,
	Scancode_PS2::None, //KeyboardExecute
	Scancode_PS2::None, //KeyboardHelp
	Scancode_PS2::None, //KeyboardMenu
	Scancode_PS2::None, //KeyboardSelect
	Scancode_PS2::None, //KeyboardStop
	Scancode_PS2::None, //KeyboardAgain
	Scancode_PS2::None, //KeyboardUndo
	Scancode_PS2::None, //KeyboardCut
	Scancode_PS2::None, //KeyboardCopy
	Scancode_PS2::None, //KeyboardPaste
	Scancode_PS2::None, //KeyboardFind
	Scancode_PS2::None, //KeyboardMute
	Scancode_PS2::None, //KeyboardVolumeUp
	Scancode_PS2::None, //KeyboardVolumeDn
	Scancode_PS2::None, //KeyboardLockingCapsLock
	Scancode_PS2::None, //KeyboardLockingNumLock
	Scancode_PS2::None, //KeyboardLockingScrollLock
	Scancode_PS2::None, //NumpadComa
	Scancode_PS2::None, //KeyboardEquals
	Scancode_PS2::None, //KeyboardIntl1
	Scancode_PS2::Katakana, //KeyboardIntl2
	Scancode_PS2::None, //KeyboardIntl3
	Scancode_PS2::Convert, //KeyboardIntl4
	Scancode_PS2::Nonconvert, //KeyboardIntl5
	Scancode_PS2::Oem3, //KeyboardIntl6
	Scancode_PS2::None, //KeyboardIntl7
	Scancode_PS2::None, //KeyboardIntl8
	Scancode_PS2::None, //KeyboardIntl9
	Scancode_PS2::None, //KeyboardLang1
	Scancode_PS2::None, //KeyboardLang2
	Scancode_PS2::None, //KeyboardLang3
	Scancode_PS2::Sbcschar, //KeyboardLang4
	Scancode_PS2::None, //KeyboardLang5
	Scancode_PS2::None, //KeyboardLang6
	Scancode_PS2::None, //KeyboardLang7
	Scancode_PS2::None, //KeyboardLang8
	Scancode_PS2::None, //KeyboardLang9
	Scancode_PS2::None, //KeyboardAlternateErase
	Scancode_PS2::None, //KeyboardSysReq_Attention
	Scancode_PS2::None, //KeyboardCancel
	Scancode_PS2::None, //KeyboardClear
	Scancode_PS2::None, //KeyboardPrior
	Scancode_PS2::None, //KeyboardReturn
	Scancode_PS2::None, //KeyboardSeparator
	Scancode_PS2::None, //KeyboardOut
	Scancode_PS2::None, //KeyboardOper
	Scancode_PS2::None, //KeyboardClear_Again
	Scancode_PS2::None, //KeyboardCrSel_Props
	Scancode_PS2::None, //KeyboardExSel

	Scancode_PS2::Power, //SystemPower
	Scancode_PS2::Sleep, //SystemSleep
	Scancode_PS2::Wake, //SystemWake

	Scancode_PS2::MediaNext, //ScanNextTrack
	Scancode_PS2::MediaPrevious, //ScanPreviousTrack
	Scancode_PS2::MediaStop, //Stop
	Scancode_PS2::MediaPlay, //Play_Pause
	Scancode_PS2::VolumeMute, //Mute
	Scancode_PS2::None, //BassBoost
	Scancode_PS2::None, //Loudness
	Scancode_PS2::VolumeUp,
	Scancode_PS2::VolumeDown,
	Scancode_PS2::None, //BassUp
	Scancode_PS2::None, //BassDown
	Scancode_PS2::None, //TrebleUp
	Scancode_PS2::None, //TrebleDown
	Scancode_PS2::LaunchMedia, //MediaSelect
	Scancode_PS2::LaunchEmail, //Mail
	Scancode_PS2::LaunchApp2, //Calculator
	Scancode_PS2::LaunchApp1, //MyComputer
	Scancode_PS2::BrowserSearch, //WWWSearch
	Scancode_PS2::BrowserHome, //WWWHome
	Scancode_PS2::BrowserBack, //WWWBack
	Scancode_PS2::BrowserForward, //WWWForward
	Scancode_PS2::BrowserStop, //WWWStop
	Scancode_PS2::BrowserRefresh, //WWWRefresh
	Scancode_PS2::BrowserFavorites, //WWWFavorites

	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,
	Scancode_PS2::None,

	Scancode_PS2::ControlLeft,
	Scancode_PS2::ShiftLeft,
	Scancode_PS2::AltLeft,
	Scancode_PS2::MetaLeft, //GUILeft
	Scancode_PS2::ControlRight,
	Scancode_PS2::ShiftRight,
	Scancode_PS2::AltRight,
	Scancode_PS2::MetaRight, //GUIRight
							 //last one is 0xE7
};

static const u32 SC_MASK = 0xFF;
static const u8 tableUSBHIDFromPS2[] =
{
	Scancode_USB_HID::None,
	Scancode_USB_HID::Escape,
	Scancode_USB_HID::N1,
	Scancode_USB_HID::N2,
	Scancode_USB_HID::N3,
	Scancode_USB_HID::N4,
	Scancode_USB_HID::N5,
	Scancode_USB_HID::N6,
	Scancode_USB_HID::N7,
	Scancode_USB_HID::N8,
	Scancode_USB_HID::N9,
	Scancode_USB_HID::N0,
	Scancode_USB_HID::Minus,
	Scancode_USB_HID::Equals,
	Scancode_USB_HID::Backspace,
	Scancode_USB_HID::Tab,
	Scancode_USB_HID::Q,
	Scancode_USB_HID::W,
	Scancode_USB_HID::E,
	Scancode_USB_HID::R,
	Scancode_USB_HID::T,
	Scancode_USB_HID::Y,
	Scancode_USB_HID::U,
	Scancode_USB_HID::I,
	Scancode_USB_HID::O,
	Scancode_USB_HID::P,
	Scancode_USB_HID::BracketLeft,
	Scancode_USB_HID::BracketRight,
	Scancode_USB_HID::Return,
	Scancode_USB_HID::ControlLeft,
	Scancode_USB_HID::A,
	Scancode_USB_HID::S,
	Scancode_USB_HID::D,
	Scancode_USB_HID::F,
	Scancode_USB_HID::G,
	Scancode_USB_HID::H,
	Scancode_USB_HID::J,
	Scancode_USB_HID::K,
	Scancode_USB_HID::L,
	Scancode_USB_HID::Semicolon,
	Scancode_USB_HID::Apostrophe,
	Scancode_USB_HID::Grave,
	Scancode_USB_HID::ShiftLeft,
	Scancode_USB_HID::Backslash,
	Scancode_USB_HID::Z,
	Scancode_USB_HID::X,
	Scancode_USB_HID::C,
	Scancode_USB_HID::V,
	Scancode_USB_HID::B,
	Scancode_USB_HID::N,
	Scancode_USB_HID::M,
	Scancode_USB_HID::Comma,
	Scancode_USB_HID::Period,
	Scancode_USB_HID::Slash,
	Scancode_USB_HID::ShiftRight,
	Scancode_USB_HID::NumpadMultiply,
	Scancode_USB_HID::AltLeft,
	Scancode_USB_HID::Space,
	Scancode_USB_HID::CapsLock,
	Scancode_USB_HID::F1,
	Scancode_USB_HID::F2,
	Scancode_USB_HID::F3,
	Scancode_USB_HID::F4,
	Scancode_USB_HID::F5,
	Scancode_USB_HID::F6,
	Scancode_USB_HID::F7,
	Scancode_USB_HID::F8,
	Scancode_USB_HID::F9,
	Scancode_USB_HID::F10,
	Scancode_USB_HID::NumLock,
	Scancode_USB_HID::ScrollLock,
	Scancode_USB_HID::Numpad7,
	Scancode_USB_HID::Numpad8,
	Scancode_USB_HID::Numpad9,
	Scancode_USB_HID::NumpadMinus,
	Scancode_USB_HID::Numpad4,
	Scancode_USB_HID::Numpad5,
	Scancode_USB_HID::Numpad6,
	Scancode_USB_HID::NumpadPlus,
	Scancode_USB_HID::Numpad1,
	Scancode_USB_HID::Numpad2,
	Scancode_USB_HID::Numpad3,
	Scancode_USB_HID::Numpad0,
	Scancode_USB_HID::NumpadPeriod,
	Scancode_USB_HID::None, //AltPrintScreen
	Scancode_USB_HID::None,
	Scancode_USB_HID::Europe2, //BracketAngle, Key between the left shift and Z
	Scancode_USB_HID::F11,
	Scancode_USB_HID::F12,
	Scancode_USB_HID::NumpadResolution,
	Scancode_USB_HID::None, //Oem1
	Scancode_USB_HID::None, //Oem2
	Scancode_USB_HID::KeyboardIntl6, //Oem3
	Scancode_USB_HID::None, //EraseEOF
	Scancode_USB_HID::None, //Oem4
	Scancode_USB_HID::None, //Oem5
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None, //Zoom
	Scancode_USB_HID::None, //Help
	Scancode_USB_HID::F13,
	Scancode_USB_HID::F14,
	Scancode_USB_HID::F15,
	Scancode_USB_HID::F16,
	Scancode_USB_HID::F17,
	Scancode_USB_HID::F18,
	Scancode_USB_HID::F19,
	Scancode_USB_HID::F20,
	Scancode_USB_HID::F21,
	Scancode_USB_HID::F22,
	Scancode_USB_HID::F23,
	Scancode_USB_HID::None, //Oem6
	Scancode_USB_HID::KeyboardIntl2, //Katakana
	Scancode_USB_HID::None, //Oem7
	Scancode_USB_HID::None,
	Scancode_USB_HID::KeyboardIntl1,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::F24,
	Scancode_USB_HID::KeyboardLang4, //Sbcschar
	Scancode_USB_HID::KeyboardLang3,
	Scancode_USB_HID::KeyboardIntl4, //Convert
	Scancode_USB_HID::None,
	Scancode_USB_HID::KeyboardIntl5 //Nonconvert
};

static const u32 E1_MASK = 0x00FF0000;
static const u32 E0_E1_MASK = 0x00FFFF00;
static const u8 tableUSBHIDFromPS2E0[]
{
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::ScanPreviousTrack, //MediaPrevious
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::ScanNextTrack, //MediaNext
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::NumpadEnter,
	Scancode_USB_HID::ControlRight,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::Mute, //VolumeMute
	Scancode_USB_HID::Calculator, //LaunchApp2
	Scancode_USB_HID::Play_Pause, //MediaPlay
	Scancode_USB_HID::None,
	Scancode_USB_HID::Stop, //MediaStop
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::VolumeDown,
	Scancode_USB_HID::None,
	Scancode_USB_HID::VolumeUp,
	Scancode_USB_HID::None,
	Scancode_USB_HID::WWWHome, //BrowserHome
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::NumpadDivide,
	Scancode_USB_HID::None,
	Scancode_USB_HID::PrintScreen,
	Scancode_USB_HID::AltRight,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::Break, // CTRL + Pause
	Scancode_USB_HID::Home,
	Scancode_USB_HID::ArrowUp,
	Scancode_USB_HID::PageUp,
	Scancode_USB_HID::None,
	Scancode_USB_HID::ArrowLeft,
	Scancode_USB_HID::None,
	Scancode_USB_HID::ArrowRight,
	Scancode_USB_HID::None,
	Scancode_USB_HID::End,
	Scancode_USB_HID::ArrowDown,
	Scancode_USB_HID::PageDown,
	Scancode_USB_HID::Insert,
	Scancode_USB_HID::Delete,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::GUILeft, //MetaLeft
	Scancode_USB_HID::GUIRight, //MetaRight
	Scancode_USB_HID::Application,
	Scancode_USB_HID::KeyboardPower, //Power
	Scancode_USB_HID::SystemSleep, //Sleep
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::None,
	Scancode_USB_HID::SystemWake, //Wake
	Scancode_USB_HID::None,
	Scancode_USB_HID::WWWSearch, //BrowserSearch = 0xE065,
	Scancode_USB_HID::WWWFavorites, //BrowserFavorites = 0xE066,
	Scancode_USB_HID::WWWRefresh, //BrowserRefresh = 0xE067,
	Scancode_USB_HID::WWWStop, //BrowserStop = 0xE068,
	Scancode_USB_HID::WWWForward, //BrowserForward = 0xE069,
	Scancode_USB_HID::WWWBack, //BrowserBack = 0xE06A,
	Scancode_USB_HID::MyComputer, //LaunchApp1 = 0xE06B,
	Scancode_USB_HID::Mail, //LaunchEmail = 0xE06C,
	Scancode_USB_HID::MediaSelect //LaunchMedia = 0xE06D,
};



namespace Scancode_PS2
{

u32 FromUSBHID(u8 scUSB)
{
	ASSERT(scUSB < sizeof(tablePS2FromUSBHID) / sizeof(tablePS2FromUSBHID[0]));
	return tablePS2FromUSBHID[scUSB];
}

}


namespace Scancode_USB_HID
{

u8 FromPS2(u32 scPS2)
{
	if((scPS2 & E0_E1_MASK) == 0)
	{
		ASSERT(scPS2 < sizeof(tableUSBHIDFromPS2) / sizeof(tableUSBHIDFromPS2[0]));
		return tableUSBHIDFromPS2[scPS2];
	}
	else if((scPS2 & E1_MASK) == 0)
	{
		scPS2 = scPS2 & SC_MASK;
		ASSERT(scPS2 < sizeof(tableUSBHIDFromPS2E0) / sizeof(tableUSBHIDFromPS2E0[0]));
		return tableUSBHIDFromPS2E0[scPS2];
	}
	else if (scPS2 == Scancode_PS2::Pause)
	{
		return Scancode_USB_HID::Pause;
	}
	else
	{
		ASSERT2(false, "Wrong PS2 scancode");
		return 0;
	}
}

}


u32 getScancodeFromRawInput(const tagRAWKEYBOARD* keyboard)
{
	static bool pauseScancodeRead = false;
	u32 scancode = keyboard->MakeCode; // MakeCode is USHORT
	u16 flags = keyboard->Flags;
	ASSERT(scancode <= Scancode_PS2::MAX_MAKECODE);

	if(flags & RI_KEY_E0)
		scancode |= Scancode_PS2::E0;
	else if(flags & RI_KEY_E1)
		scancode |= Scancode_PS2::E1;

	if(scancode == Scancode_PS2::PAUSE_PART1) //The pause scancode is in 2 parts: a WM_INPUT with 0xE11D and one WM_INPUT with 0x45
	{
		pauseScancodeRead = true;
	}
	else if(pauseScancodeRead)
	{
		if(scancode == Scancode_PS2::PAUSE_PART2)
			scancode = Scancode_PS2::Pause;
		pauseScancodeRead = false;
	}
	else if(scancode == Scancode_PS2::AltPrintScreen) //Alt + print screen return scancode 0x54 but we want it to return 0xE037 because 0x54 will not return a name for the key
	{
		scancode = Scancode_PS2::PrintScreen;
	}

	//some of those a break scancode, so we ignore them
	if(scancode == Scancode_PS2::PAUSE_PART1
		|| scancode == Scancode_PS2::IGNORE1
		|| scancode == Scancode_PS2::IGNORE2
		|| scancode == Scancode_PS2::IGNORE3
		|| scancode == Scancode_PS2::IGNORE4)
		return Scancode_PS2::None;

	return scancode;
}


i32 getUTF16TextFromRawInput(const tagRAWKEYBOARD* keyboard, wchar_t* buffer, u32 bufferSize)
{
	static BYTE keyState[256] = {};

	u16 flags = keyboard->Flags;
	bool pressed = ((flags & RI_KEY_BREAK) == 0);

	bool e0 = (flags & RI_KEY_E0) != 0;
	bool e1 = (flags & RI_KEY_E1) != 0;

	// these are unassigned but not reserved as of now.
	// this is bad but, you know, we'll fix it if it ever breaks.
	#define VK_LRETURN         0x9E
	#define VK_RRETURN         0x9F

	auto updateKeyState = [flags](auto key)
	{
		keyState[key] = (flags & 1) ? 0 : 0xff;
	};

	// note: we set all bits in the byte if the key is down. 
	// This is because windows expects it to be in the high_order_bit (when using it for converting to unicode for example)
	// and I like it to be in the low_order_bit,  
	if(keyboard->VKey == VK_CONTROL)
	{
		if(e0)	updateKeyState(VK_RCONTROL);
		else	updateKeyState(VK_LCONTROL);
		keyState[VK_CONTROL] = keyState[VK_RCONTROL] | keyState[VK_LCONTROL];
	}
	else if(keyboard->VKey == VK_SHIFT)
	{
		// because why should any api be consistent lol
		// (because we get different scancodes for l/r-shift but not for l/r ctrl etc... but still)
		updateKeyState(MapVirtualKey(keyboard->MakeCode, MAPVK_VSC_TO_VK_EX));
		keyState[VK_SHIFT] = keyState[VK_LSHIFT] | keyState[VK_RSHIFT];
	}
	else if(keyboard->VKey == VK_MENU)
	{
		if(e0)	updateKeyState(VK_LMENU);
		else	updateKeyState(VK_RMENU);
		keyState[VK_MENU] = keyState[VK_RMENU] | keyState[VK_LMENU];
	}
	else if(keyboard->VKey == VK_RETURN)
	{
		if(e0) updateKeyState(VK_RRETURN);
		else	updateKeyState(VK_LRETURN);
		keyState[VK_RETURN] = keyState[VK_RRETURN] | keyState[VK_LRETURN];
	}
	else
	{
		updateKeyState(keyboard->VKey);
	}

	if(pressed)
	{
		// get unicode.

		//simulating altgr, assumes all leftalts is algr
		// which seem to work since ctrl is ignored on US versions of ToUnicode. Bad way of doing it, Not sure how to detect if left alt is altgr though.
		BYTE ctrl = keyState[VK_CONTROL];
		keyState[VK_CONTROL] |= keyState[VK_RMENU];
		i32 utf16_len = ToUnicode(keyboard->VKey, keyboard->MakeCode, keyState, buffer, bufferSize, 0);
		keyState[VK_CONTROL] = ctrl;
		return utf16_len;
	}
	else
	{
		return 0;
	}
}


u32 getScancodeName(u32 scancode, char* buffer, u32 bufferLength)
{
	//- bit 16 - 23 contains the first byte of the scancode
	//- bit 24 indicates that the scancode is 2 bytes (extended)

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