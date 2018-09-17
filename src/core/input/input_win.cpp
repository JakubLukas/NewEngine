#include "input_win.h"

#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h>

#include "core\asserts.h"


namespace Veng
{


namespace Keyboard
{


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
	Scancode_USBHID::None,
	Scancode_USBHID::Escape,
	Scancode_USBHID::N1,
	Scancode_USBHID::N2,
	Scancode_USBHID::N3,
	Scancode_USBHID::N4,
	Scancode_USBHID::N5,
	Scancode_USBHID::N6,
	Scancode_USBHID::N7,
	Scancode_USBHID::N8,
	Scancode_USBHID::N9,
	Scancode_USBHID::N0,
	Scancode_USBHID::Minus,
	Scancode_USBHID::Equals,
	Scancode_USBHID::Backspace,
	Scancode_USBHID::Tab,
	Scancode_USBHID::Q,
	Scancode_USBHID::W,
	Scancode_USBHID::E,
	Scancode_USBHID::R,
	Scancode_USBHID::T,
	Scancode_USBHID::Y,
	Scancode_USBHID::U,
	Scancode_USBHID::I,
	Scancode_USBHID::O,
	Scancode_USBHID::P,
	Scancode_USBHID::BracketLeft,
	Scancode_USBHID::BracketRight,
	Scancode_USBHID::Return,
	Scancode_USBHID::ControlLeft,
	Scancode_USBHID::A,
	Scancode_USBHID::S,
	Scancode_USBHID::D,
	Scancode_USBHID::F,
	Scancode_USBHID::G,
	Scancode_USBHID::H,
	Scancode_USBHID::J,
	Scancode_USBHID::K,
	Scancode_USBHID::L,
	Scancode_USBHID::Semicolon,
	Scancode_USBHID::Apostrophe,
	Scancode_USBHID::Grave,
	Scancode_USBHID::ShiftLeft,
	Scancode_USBHID::Backslash,
	Scancode_USBHID::Z,
	Scancode_USBHID::X,
	Scancode_USBHID::C,
	Scancode_USBHID::V,
	Scancode_USBHID::B,
	Scancode_USBHID::N,
	Scancode_USBHID::M,
	Scancode_USBHID::Comma,
	Scancode_USBHID::Period,
	Scancode_USBHID::Slash,
	Scancode_USBHID::ShiftRight,
	Scancode_USBHID::NumpadMultiply,
	Scancode_USBHID::AltLeft,
	Scancode_USBHID::Space,
	Scancode_USBHID::CapsLock,
	Scancode_USBHID::F1,
	Scancode_USBHID::F2,
	Scancode_USBHID::F3,
	Scancode_USBHID::F4,
	Scancode_USBHID::F5,
	Scancode_USBHID::F6,
	Scancode_USBHID::F7,
	Scancode_USBHID::F8,
	Scancode_USBHID::F9,
	Scancode_USBHID::F10,
	Scancode_USBHID::NumLock,
	Scancode_USBHID::ScrollLock,
	Scancode_USBHID::Numpad7,
	Scancode_USBHID::Numpad8,
	Scancode_USBHID::Numpad9,
	Scancode_USBHID::NumpadMinus,
	Scancode_USBHID::Numpad4,
	Scancode_USBHID::Numpad5,
	Scancode_USBHID::Numpad6,
	Scancode_USBHID::NumpadPlus,
	Scancode_USBHID::Numpad1,
	Scancode_USBHID::Numpad2,
	Scancode_USBHID::Numpad3,
	Scancode_USBHID::Numpad0,
	Scancode_USBHID::NumpadPeriod,
	Scancode_USBHID::None, //AltPrintScreen
	Scancode_USBHID::None,
	Scancode_USBHID::Europe2, //BracketAngle, Key between the left shift and Z
	Scancode_USBHID::F11,
	Scancode_USBHID::F12,
	Scancode_USBHID::NumpadResolution,
	Scancode_USBHID::None, //Oem1
	Scancode_USBHID::None, //Oem2
	Scancode_USBHID::KeyboardIntl6, //Oem3
	Scancode_USBHID::None, //EraseEOF
	Scancode_USBHID::None, //Oem4
	Scancode_USBHID::None, //Oem5
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None, //Zoom
	Scancode_USBHID::None, //Help
	Scancode_USBHID::F13,
	Scancode_USBHID::F14,
	Scancode_USBHID::F15,
	Scancode_USBHID::F16,
	Scancode_USBHID::F17,
	Scancode_USBHID::F18,
	Scancode_USBHID::F19,
	Scancode_USBHID::F20,
	Scancode_USBHID::F21,
	Scancode_USBHID::F22,
	Scancode_USBHID::F23,
	Scancode_USBHID::None, //Oem6
	Scancode_USBHID::KeyboardIntl2, //Katakana
	Scancode_USBHID::None, //Oem7
	Scancode_USBHID::None,
	Scancode_USBHID::KeyboardIntl1,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::F24,
	Scancode_USBHID::KeyboardLang4, //Sbcschar
	Scancode_USBHID::KeyboardLang3,
	Scancode_USBHID::KeyboardIntl4, //Convert
	Scancode_USBHID::None,
	Scancode_USBHID::KeyboardIntl5 //Nonconvert
};

static const u32 E1_MASK = 0x00FF0000;
static const u32 E0_E1_MASK = 0x00FFFF00;
static const u8 tableUSBHIDFromPS2E0[]
{
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::ScanPreviousTrack, //MediaPrevious
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::ScanNextTrack, //MediaNext
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::NumpadEnter,
	Scancode_USBHID::ControlRight,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::Mute, //VolumeMute
	Scancode_USBHID::Calculator, //LaunchApp2
	Scancode_USBHID::Play_Pause, //MediaPlay
	Scancode_USBHID::None,
	Scancode_USBHID::Stop, //MediaStop
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::VolumeDown,
	Scancode_USBHID::None,
	Scancode_USBHID::VolumeUp,
	Scancode_USBHID::None,
	Scancode_USBHID::WWWHome, //BrowserHome
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::NumpadDivide,
	Scancode_USBHID::None,
	Scancode_USBHID::PrintScreen,
	Scancode_USBHID::AltRight,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::Break, // CTRL + Pause
	Scancode_USBHID::Home,
	Scancode_USBHID::ArrowUp,
	Scancode_USBHID::PageUp,
	Scancode_USBHID::None,
	Scancode_USBHID::ArrowLeft,
	Scancode_USBHID::None,
	Scancode_USBHID::ArrowRight,
	Scancode_USBHID::None,
	Scancode_USBHID::End,
	Scancode_USBHID::ArrowDown,
	Scancode_USBHID::PageDown,
	Scancode_USBHID::Insert,
	Scancode_USBHID::Delete,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::GUILeft, //MetaLeft
	Scancode_USBHID::GUIRight, //MetaRight
	Scancode_USBHID::Application,
	Scancode_USBHID::KeyboardPower, //Power
	Scancode_USBHID::SystemSleep, //Sleep
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::None,
	Scancode_USBHID::SystemWake, //Wake
	Scancode_USBHID::None,
	Scancode_USBHID::WWWSearch, //BrowserSearch = 0xE065,
	Scancode_USBHID::WWWFavorites, //BrowserFavorites = 0xE066,
	Scancode_USBHID::WWWRefresh, //BrowserRefresh = 0xE067,
	Scancode_USBHID::WWWStop, //BrowserStop = 0xE068,
	Scancode_USBHID::WWWForward, //BrowserForward = 0xE069,
	Scancode_USBHID::WWWBack, //BrowserBack = 0xE06A,
	Scancode_USBHID::MyComputer, //LaunchApp1 = 0xE06B,
	Scancode_USBHID::Mail, //LaunchEmail = 0xE06C,
	Scancode_USBHID::MediaSelect //LaunchMedia = 0xE06D,
};


static const u8 tableAsciiCharFromUSBHID[]
{
	'\0', //None
	'\0', //Overrun_Error
	'\0', //POST_Fail
	'\0', //ErrorUndefined
	'a', //A = 0x04
	'b', //B = 0x05
	'c', //C = 0x06
	'd', //D = 0x07
	'e', //E = 0x08
	'f', //F = 0x09
	'g', //G = 0x0A
	'h', //H = 0x0B
	'i', //I = 0x0C
	'j', //J = 0x0D
	'k', //K = 0x0E
	'l', //L = 0x0F
	'm', //M = 0x10
	'n', //N = 0x11
	'o', //O = 0x12
	'p', //P = 0x13
	'q', //Q = 0x14
	'r', //R = 0x15
	's', //S = 0x16
	't', //T = 0x17
	'u', //U = 0x18
	'v', //V = 0x19
	'w', //W = 0x1A
	'x', //X = 0x1B
	'y', //Y = 0x1C
	'z', //Z = 0x1D
	'1', //N1 = 0x1E
	'2', //N2 = 0x1F
	'3', //N3 = 0x20
	'4', //N4 = 0x21
	'5', //N5 = 0x22
	'6', //N6 = 0x23
	'7', //N7 = 0x24
	'8', //N8 = 0x25
	'9', //N9 = 0x26
	'0', //N0 = 0x27
	'\r', //Return = 0x28
	'\0', //Escape = 0x29
	'\0', //Backspace = 0x2A
	'\t', //Tab = 0x2B
	' ', //Space = 0x2C
	'-', //Minus = 0x2D
	'=', //Equals = 0x2E
	'(', //BracketLeft = 0x2F, //1A
	')', //BracketRight = 0x30, //1B
	'\\', //Backslash = 0x31, //2B
	'\0', //Europe1 = 0x32, //WTF?
	';', //Semicolon = 0x33, //27
	'\'', //Apostrophe = 0x34, //28
	'\0', //Grave = 0x35, //29
	',', //Comma = 0x36, //33
	'.', //Period = 0x37, //34
	'/', //Slash = 0x38, //35
	'\0', //CapsLock = 0x39, //3A
	'\0', //F1 = 0x3A, //3B
	'\0', //F2 = 0x3B, //3C
	'\0', //F3 = 0x3C, //3D
	'\0', //F4 = 0x3D, //3E
	'\0', //F5 = 0x3E, //3F
	'\0', //F6 = 0x3F, //40
	'\0', //F7 = 0x40, //41
	'\0', //F8 = 0x41, //42
	'\0', //F9 = 0x42, //43
	'\0', //F10 = 0x43, //44
	'\0', //F11 = 0x44, //57
	'\0', //F12 = 0x45, //58
	'\0', //PrintScreen = 0x46, //E037
	'\0', //ScrollLock = 0x47, //46
	'\0', //Break = 0x48, //E046
	'\0', //Pause = 0x48, //E11D45
	'\0', //Insert = 0x49, //E052
	'\0', //Home = 0x4A, //E047
	'\0', //PageUp = 0x4B, //E049
	'\0', //Delete = 0x4C, //E053
	'\0', //End = 0x4D, //E04F
	'\0', //PageDown = 0x4E, //E051
	'\0', //ArrowRight = 0x4F, //E04D
	'\0', //ArrowLeft = 0x50, //E04B
	'\0', //ArrowDown = 0x51, //E050
	'\0', //ArrowUp = 0x52, //E048
	'\0', //NumLock = 0x53, //45
	'/', //NumpadDivide = 0x54, //E035
	'*', //NumpadMultiply = 0x55, //37
	'-', //NumpadMinus = 0x56, //4A
	'+', //NumpadPlus = 0x57, //4E
	'\r', //NumpadEnter = 0x58, //E01C
	'1', //Numpad1 = 0x59, //4F
	'2', //Numpad2 = 0x5A, //50
	'3', //Numpad3 = 0x5B, //51
	'4', //Numpad4 = 0x5C, //4B
	'5', //Numpad5 = 0x5D, //4C
	'6', //Numpad6 = 0x5E, //4D
	'7', //Numpad7 = 0x5F, //47
	'8', //Numpad8 = 0x60, //48
	'9', //Numpad9 = 0x61, //49
	'0', //Numpad0 = 0x62, //52
	'.', //NumpadPeriod = 0x63, //53
	'\0', //Europe2 = 0x64, //56
	'\0', //Application = 0x65, //E05D
	'\0', //KeyboardPower = 0x66, //E05E
	'\0', //NumpadResolution = 0x67, //59
};


namespace Scancode_PS2
{

u32 FromUSBHID(u8 scUSB)
{
	ASSERT(scUSB < sizeof(tablePS2FromUSBHID) / sizeof(tablePS2FromUSBHID[0]));
	return tablePS2FromUSBHID[scUSB];
}

}


namespace Scancode_USBHID
{

u8 FromPS2(u32 scPS2)
{
	if ((scPS2 & E0_E1_MASK) == 0)
	{
		ASSERT(scPS2 < sizeof(tableUSBHIDFromPS2) / sizeof(tableUSBHIDFromPS2[0]));
		return tableUSBHIDFromPS2[scPS2];
	}
	else if ((scPS2 & E1_MASK) == 0)
	{
		scPS2 = scPS2 & SC_MASK;
		ASSERT(scPS2 < sizeof(tableUSBHIDFromPS2E0) / sizeof(tableUSBHIDFromPS2E0[0]));
		return tableUSBHIDFromPS2E0[scPS2];
	}
	else if (scPS2 == Scancode_PS2::Pause)
	{
		return Scancode_USBHID::Pause;
	}
	else
	{
		ASSERT2(false, "Wrong PS2 scancode");
		return 0;
	}
}


u8 ToAsciiChar(u8 usbHid)
{
	if(usbHid < NumpadResolution)
		return tableAsciiCharFromUSBHID[usbHid];
	else
		return '@';
}

}


u32 getScancodeFromRawInput(const tagRAWKEYBOARD* keyboard)
{
	static bool pauseScancodeRead = false;
	u32 scancode = keyboard->MakeCode; // MakeCode is USHORT
	u16 flags = keyboard->Flags;
	ASSERT(scancode <= Scancode_PS2::MAX_MAKECODE);

	if (flags & RI_KEY_E0)
		scancode |= Scancode_PS2::E0;
	else if (flags & RI_KEY_E1)
		scancode |= Scancode_PS2::E1;

	if (scancode == Scancode_PS2::PAUSE_PART1) //The pause scancode is in 2 parts: a WM_INPUT with 0xE11D and one WM_INPUT with 0x45
	{
		pauseScancodeRead = true;
	}
	else if (pauseScancodeRead)
	{
		if (scancode == Scancode_PS2::PAUSE_PART2)
			scancode = Scancode_PS2::Pause;
		pauseScancodeRead = false;
	}
	else if (scancode == Scancode_PS2::AltPrintScreen) //Alt + print screen return scancode 0x54 but we want it to return 0xE037 because 0x54 will not return a name for the key
	{
		scancode = Scancode_PS2::PrintScreen;
	}

	//some of those a break scancode, so we ignore them
	if (scancode == Scancode_PS2::PAUSE_PART1
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
	if (keyboard->VKey == VK_CONTROL)
	{
		if (e0)	updateKeyState(VK_RCONTROL);
		else	updateKeyState(VK_LCONTROL);
		keyState[VK_CONTROL] = keyState[VK_RCONTROL] | keyState[VK_LCONTROL];
	}
	else if (keyboard->VKey == VK_SHIFT)
	{
		// because why should any api be consistent lol
		// (because we get different scancodes for l/r-shift but not for l/r ctrl etc... but still)
		updateKeyState(MapVirtualKey(keyboard->MakeCode, MAPVK_VSC_TO_VK_EX));
		keyState[VK_SHIFT] = keyState[VK_LSHIFT] | keyState[VK_RSHIFT];
	}
	else if (keyboard->VKey == VK_MENU)
	{
		if (e0)	updateKeyState(VK_LMENU);
		else	updateKeyState(VK_RMENU);
		keyState[VK_MENU] = keyState[VK_RMENU] | keyState[VK_LMENU];
	}
	else if (keyboard->VKey == VK_RETURN)
	{
		if (e0) updateKeyState(VK_RRETURN);
		else	updateKeyState(VK_LRETURN);
		keyState[VK_RETURN] = keyState[VK_RRETURN] | keyState[VK_LRETURN];
	}
	else
	{
		updateKeyState(keyboard->VKey);
	}

	if (pressed)
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

	if (extended != 0)
	{
		if (extended == 0xE11D00)
			lParam = 0x45 << 16;
		else
			lParam = (0x100 | (scancode & 0xff)) << 16;
	}
	else
	{
		lParam = scancode << 16;

		if (scancode == 0x45)
			lParam |= (0x1 << 24);
	}

	result = GetKeyNameText(lParam, buffer, bufferLength);
	return result;
}


}


}