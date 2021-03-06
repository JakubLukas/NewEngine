#include "editor.h"

#include "editor/editor_interface.h"
#include <bgfx/bgfx.h>

#include "core/allocators.h"
#include "core/engine.h"
#include "core/logs.h"
#include "core/asserts.h"
#include "core/file/blob.h"
#include "core/containers/associative_array.h"

#include "core/os/os_utils.h"


#include "renderer/renderer.h"////////////////////////
#include "script/script.h"////////////////////////////
#include "core/file/path.h"////////////////////////////////
#include "core/math/math.h"////////////////////////////////
#include "core/file/file.h"////////////////////
#include "core/file/file_system.h"/////////////////
#include "core/memory.h"/////////////////////
#include "core/time.h"////////////////////////////
#include "core/math/matrix.h"////////////////////////////////////

#include "core/input/devices/input_device_keyboard.h"

#include <bgfx/bgfx.h>
#include "../external/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"

#include "editor/event_queue.h"
#include "editor/widget_base.h"
#include "editor/widget_register.h"


namespace bx
{

/// Abstract allocator interface.
struct AllocatorI
{
	virtual ~AllocatorI() {}

	/// Allocates, resizes memory block, or frees memory.
	///
	/// @param[in] _ptr If _ptr is NULL new block will be allocated.
	/// @param[in] _size If _ptr is set, and _size is 0, memory will be freed.
	/// @param[in] _align Alignment.
	/// @param[in] _file Debug file path info.
	/// @param[in] _line Debug file line info.
	virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) = 0;

};

}


namespace bgfx
{

struct PlatformData
{
	void* ndt;          //!< Native display type.
	void* nwh;          //!< Native window handle.
	void* context;      //!< GL context, or D3D device.
	void* backBuffer;   //!< GL backbuffer, or D3D render target view.
	void* backBufferDS; //!< Backbuffer depth/stencil.
	void* session;      //!< ovrSession, for Oculus SDK
};

void setPlatformData(const PlatformData& _data);

inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexDecl& _decl, uint32_t _numIndices)
{
	return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _decl)
		&& (0 == _numIndices || _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices))
		;
}

}


namespace Veng
{

struct BGFXAllocator : public bx::AllocatorI
{
	static const size_t NATURAL_ALIGNEMENT = 8;

	explicit BGFXAllocator(Allocator& source)
		: m_source(source)
	{}

	void* realloc(void* _ptr, size_t _size, size_t _alignment, const char* _file, u32 _line) override
	{
		if (_size == 0)
		{
			if (_ptr != nullptr)
			{
				m_source.Deallocate(_ptr);
			}
			return nullptr;
		}
		else
		{
			_alignment = (_alignment < NATURAL_ALIGNEMENT) ? NATURAL_ALIGNEMENT : _alignment;

			if (_ptr == nullptr)
			{
				return m_source.Allocate(_size, _alignment);
			}
			else
			{
				return m_source.Reallocate(_ptr, _size, _alignment);
			}
		}
	}


	Allocator& m_source;
};


struct BGFXCallback : public bgfx::CallbackI
{
	void fatal(bgfx::Fatal::Enum _code, const char* _str) override
	{
		switch (_code)
		{
		case bgfx::Fatal::DebugCheck:
			Log(LogType::Error, "Error: bgfx: DebugCheck: %s\n", _str);
			break;
		case bgfx::Fatal::InvalidShader:
			Log(LogType::Error, "Error: bgfx: InvalidShader: %s\n", _str);
			break;
		case bgfx::Fatal::UnableToInitialize:
			Log(LogType::Error, "Error: bgfx: UnableToInitialize: %s\n", _str);
			break;
		case bgfx::Fatal::UnableToCreateTexture:
			Log(LogType::Error, "Error: bgfx: UnableToCreateTexture: %s\n", _str);
			break;
		case bgfx::Fatal::DeviceLost:
			Log(LogType::Error, "Error: bgfx: DeviceLost: %s\n", _str);
			break;
		}
	}
	void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList) override {}
	void profilerBegin(const char* _name, uint32_t _abgr, const char* _filePath, uint16_t _line) override {}
	void profilerBeginLiteral(const char* _name, uint32_t _abgr, const char* _filePath, uint16_t _line) override {}
	void profilerEnd() override {}
	uint32_t cacheReadSize(uint64_t _id) override { return 0; }
	bool cacheRead(uint64_t _id, void* _data, uint32_t _size) override { return false; }
	void cacheWrite(uint64_t _id, const void* _data, uint32_t _size) override {}
	void screenShot(const char* _filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void* _data, uint32_t _size, bool _yflip) override {}
	void captureBegin(uint32_t _width, uint32_t _height, uint32_t _pitch, bgfx::TextureFormat::Enum _format, bool _yflip) override {}
	void captureEnd() override {}
	void captureFrame(const void* _data, uint32_t _size) override {}
};

}


namespace ImGui
{

void* Allocate(size_t sz, void* userData)
{
	Veng::Allocator* allocator = static_cast<Veng::Allocator*>(userData);
	return allocator->Allocate(sz, alignof(char));
}

void Deallocate(void* ptr, void* userData)
{
	if (ptr != nullptr)
	{
		Veng::Allocator* allocator = static_cast<Veng::Allocator*>(userData);
		allocator->Deallocate(ptr);
	}
}

}

namespace ImGui
{

typedef Veng::u32 MouseButtonFlags;
enum MouseButtonBitOffset : MouseButtonFlags
{
	MouseButtonBitOffset_Left = 0,
	MouseButtonBitOffset_Right,
	MouseButtonBitOffset_Middle,
	MouseButtonBitOffset_Extra4,
	MouseButtonBitOffset_Extra5,
};
enum MouseButtonBits : MouseButtonFlags
{
	MouseButton_None = 0,
	MouseButton_Left = 1 << MouseButtonBitOffset_Left,
	MouseButton_Right = 1 << MouseButtonBitOffset_Right,
	MouseButton_Middle = 1 << MouseButtonBitOffset_Middle,
	MouseButton_Extra4 = 1 << MouseButtonBitOffset_Extra4,
	MouseButton_Extra5 = 1 << MouseButtonBitOffset_Extra5,
};

typedef Veng::u32 ModifierKeyFlags;
enum ModifierKeyBitOffset : ModifierKeyFlags
{
	ModifierKeyBitOffset_Ctrl = 0,
	ModifierKeyBitOffset_Shift,
	ModifierKeyBitOffset_Alt,
	ModifierKeyBitOffset_Super,
};
enum ModifierKeyBits : ModifierKeyFlags
{
	ModifierKey_None = 0,
	ModifierKey_Ctrl = 1 << ModifierKeyBitOffset_Ctrl,
	ModifierKey_Shift = 1 << ModifierKeyBitOffset_Shift,
	ModifierKey_Alt = 1 << ModifierKeyBitOffset_Alt,
	ModifierKey_Super = 1 << ModifierKeyBitOffset_Super,
};

enum class TextEditSpecials : int
{
	SelectAll = 0x100,	// for text edit CTRL+A: select all
	Copy = 0x101,				// for text edit CTRL+C: copy
	Paste = 0x102,			// for text edit CTRL+V: paste
	Cut = 0x103,				// for text edit CTRL+X: cut
	Redo = 0x104,				// for text edit CTRL+Y: redo
	Undo = 0x105,				// for text edit CTRL+Z: undo
};


static const float FRAME_ROUNDING = 4.0f;
static const float WINDOW_BORDER_SIZE = 0.0f;

static const bgfx::ViewId VIEW_ID = 0;
static const Veng::u32 CLEAR_COLOR = 0x408040ff;

}


namespace Veng
{


static bool FileOpenDialog(Path& path)
{
	os::FileDialogData data;
	data.filter = "Project file,.neproj";
	os::GetWorkingDir(data.fileName, Path::MAX_LENGTH);
	string::Copy(data.fileName + string::Length(data.fileName), "/*.neproj");
	os::PathToNativePath(data.fileName);

	bool result = ShowOpenFileDialog(data);
	if (result)
		path.SetPath(data.fileName);

	return result;
}

static bool FileSaveDialog(Path& path)
{
	os::FileDialogData data;
	data.filter = "Project file,.neproj";
	os::GetWorkingDir(data.fileName, Path::MAX_LENGTH);
	string::Copy(data.fileName + string::Length(data.fileName), "/*.neproj");
	os::PathToNativePath(data.fileName);

	bool result = ShowSaveFileDialog(data);
	if (result)
		path.SetPath(data.fileName);

	return result;
}


RenderSystem* m_renderSystem = nullptr;////////////////////////////////
ScriptSystem* m_scriptSystem = nullptr;////////////////////////////////


namespace Editor
{


struct Input
{
	ImVec2 mousePos = { 0.0f, 0.0f };
	ImGui::MouseButtonFlags mouseButtons = ImGui::MouseButton_None;
	ImGui::ModifierKeyFlags modifierKeys = ImGui::ModifierKey_None;
	static const size_t KEYBOARD_BUFFER_SIZE = 32;
	u8 keyboardBuffer[KEYBOARD_BUFFER_SIZE] = { 0 };
	int keyboardBufferPos = 0;
	static const size_t CHARACTER_BUFFER_SIZE = 16;
	u8 characterBuffer[CHARACTER_BUFFER_SIZE] = { 0 };
	int characterBufferPos = 0;
	float scroll = 0.0f;
};

struct InputKeyboardFiltering
{
	u32 filter[8] = { 0 };
};

struct ImguiBgfxData
{
	bgfx::ViewId viewId = 0;
	bgfx::VertexDecl vertexDecl;
	bgfx::ProgramHandle program;
	bgfx::TextureHandle textureFont;
	bgfx::UniformHandle textureUniform;
};

static bgfx::ProgramHandle LoadProgram(const Path& vertexPath, const Path& fragmentPath, Allocator& allocator)
{
	const FileMode fileMode{
		FileMode::Access::Read,
		FileMode::ShareMode::ShareRead,
		FileMode::CreationDisposition::OpenExisting,
		FileMode::FlagNone
	};

	//vertex shader
	nativeFileHandle vsHandle;
	ASSERT(FS::OpenFileSync(vsHandle, vertexPath, fileMode));
	size_t vsFileSize = FS::GetFileSize(vsHandle);
	u8* vsData = (u8*)allocator.Allocate(vsFileSize, alignof(u8));
	size_t vsFileSizeRead = 0;
	ASSERT(FS::ReadFileSync(vsHandle, 0, vsData, vsFileSize, vsFileSizeRead));
	ASSERT(vsFileSize == vsFileSizeRead);
	ASSERT(FS::CloseFileSync(vsHandle));

	const bgfx::Memory* vsMem = bgfx::alloc((u32)vsFileSize + 1);
	memory::Copy(vsMem->data, vsData, vsFileSize);
	vsMem->data[vsMem->size - 1] = '\0';
	bgfx::ShaderHandle vsHandleBgfx = bgfx::createShader(vsMem);
	ASSERT(bgfx::isValid(vsHandleBgfx));
	allocator.Deallocate(vsData);

	//fragment shader
	nativeFileHandle fsHandle;
	ASSERT(FS::OpenFileSync(fsHandle, fragmentPath, fileMode));
	size_t fsFileSize = FS::GetFileSize(fsHandle);
	u8* fsData = (u8*)allocator.Allocate(fsFileSize, alignof(u8));
	size_t fsFileSizeRead = 0;
	ASSERT(FS::ReadFileSync(fsHandle, 0, fsData, fsFileSize, fsFileSizeRead));
	ASSERT(fsFileSize == fsFileSizeRead);
	ASSERT(FS::CloseFileSync(fsHandle));

	const bgfx::Memory* fsMem = bgfx::alloc((u32)fsFileSize + 1);
	memory::Copy(fsMem->data, fsData, fsFileSize);
	fsMem->data[fsMem->size - 1] = '\0';
	bgfx::ShaderHandle fsHandleBgfx = bgfx::createShader(fsMem);
	ASSERT(bgfx::isValid(fsHandleBgfx));
	allocator.Deallocate(fsData);

	bgfx::ProgramHandle programHandle = bgfx::createProgram(vsHandleBgfx, fsHandleBgfx, true);
	ASSERT(bgfx::isValid(programHandle));
	return programHandle;
}


class EditorAppImpl : public EditorApp
{
public:
	EditorAppImpl(Allocator& allocator, App& app)
		: m_allocator(allocator)
		, m_app(app)
		, m_engineAllocator(m_allocator)
		, m_bgfxAllocator(m_allocator)
		, m_imguiAllocator(m_allocator)
		, m_inputKeyboardFilter(m_allocator)
		, m_eventQueue(m_allocator)
		, m_widgets(m_allocator)
	{
		m_allocator.SetDebugName("Editor");
		m_engineAllocator.SetDebugName("Engine");
		m_imguiAllocator.SetDebugName("ImGui");
	}

	void Init() override
	{
		InitRender();
		InitImgui();
		InitEngine();
		m_editorInterface = NEW_OBJECT(m_allocator, EditorInterface)(*m_engine, m_eventQueue);
		InitWidgets();
	}

	void Deinit() override
	{
		DeinitWidgets();
		DELETE_OBJECT(m_allocator, m_editorInterface);
		DeinitEngine();//TODO: shut down engine gracefully

		DeinitImgui();
		DeinitRender();
	}

	void Update(float deltaTime) override
	{
		UpdateImguiInput(deltaTime);

		m_engine->Update(deltaTime);
		m_eventQueue.FrameUpdate();

		for(WidgetItem& widget : m_widgets)
		{
			widget.instance->Update(m_eventQueue);
		}
		
		UpdateImgui();
		RenderImgui();

		bgfx::frame();//flip buffers
	}


	void Resize(windowHandle handle, i32 width, i32 height) override
	{
		if (handle == m_app.GetMainWindowHandle())
		{
			m_windowSize = { width, height };
			bgfx::reset(width, height, BGFX_RESET_VSYNC);
			bgfx::setViewFrameBuffer(m_imguiBgfxData.viewId, BGFX_INVALID_HANDLE);
			bgfx::setViewRect(m_imguiBgfxData.viewId, 0, 0, uint16_t(width), uint16_t(height));
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2((float)width, (float)height);
		}
		/*else if (handle == m_engineWindow.hwnd)
		{
			bgfx::destroy(m_engineWindow.fbh);
			m_engineWindow.fbh = bgfx::createFrameBuffer(m_engineWindow.hwnd, uint16_t(width), uint16_t(height));
			bgfx::setViewRect(m_engineWindow.viewId, 0, 0, uint16_t(width), uint16_t(height));////////////////////////////////
			m_renderSystem->Resize(width, height);
		}*/
		else
		{
			//ASSERT2(false, "Handle other windows");
		}
	}

	void SetFocus(windowHandle handle, bool hasFocus) override
	{
		handle;///////////////////////////////////////////////////////////////////////
		if (hasFocus)
			m_inputEnabled = true;
		else
			m_inputEnabled = false;
	}

	inputDeviceID RegisterDevice(inputDeviceHandle deviceHandle, InputDeviceCategory category, const String& name) override
	{
		if (category == InputDeviceCategory::Keyboard)
		{
			InputKeyboardFiltering& filter = *m_inputKeyboardFilter.Insert(deviceHandle, InputKeyboardFiltering());
		}

		return m_engine->GetInputSystem()->RegisterDevice(deviceHandle, category, name);
	}

	void UnregisterDevice(inputDeviceHandle deviceHandle) override
	{
		m_inputKeyboardFilter.Erase(deviceHandle);

		m_engine->GetInputSystem()->UnregisterDevice(deviceHandle);
	}

	void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, KeyboardDevice::Button buttonId, bool pressed) override
	{
		if (!m_inputEnabled)
			return;

		//filter key down / up to be handled only once
		InputKeyboardFiltering& filter = m_inputKeyboardFilter[deviceHandle];
		u32& section = filter.filter[(u8)buttonId / 32];
		u32 keyBit = 1 << (u8)buttonId % 32;
		if (pressed)
		{
			if (section & keyBit)
				return;
			else
				section |= keyBit;
		}
		else
		{
			if (!(section & keyBit))
				return;
			else
				section &= ~keyBit;
		}

		//if (handle == m_app.GetMainWindowHandle())
		{
			ImGuiIO& io = ImGui::GetIO();
			switch (buttonId)
			{
			case KeyboardDevice::Button::ControlLeft:
			case KeyboardDevice::Button::ControlRight:
				m_inputBuffer.modifierKeys = (m_inputBuffer.modifierKeys & ~ImGui::ModifierKey_Ctrl) | (pressed << ImGui::ModifierKeyBitOffset_Ctrl); break;
			case KeyboardDevice::Button::ShiftLeft:
			case KeyboardDevice::Button::ShiftRight:
				m_inputBuffer.modifierKeys = (m_inputBuffer.modifierKeys & ~ImGui::ModifierKey_Shift) | (pressed << ImGui::ModifierKeyBitOffset_Shift); break;
			case KeyboardDevice::Button::AltLeft:
			case KeyboardDevice::Button::AltRight:
				m_inputBuffer.modifierKeys = (m_inputBuffer.modifierKeys & ~ImGui::ModifierKey_Alt) | (pressed << ImGui::ModifierKeyBitOffset_Alt); break;
			case KeyboardDevice::Button::GUILeft:
			case KeyboardDevice::Button::GUIRight:
				m_inputBuffer.modifierKeys = (m_inputBuffer.modifierKeys & ~ImGui::ModifierKey_Super) | (pressed << ImGui::ModifierKeyBitOffset_Super); break;
			default:
				if (pressed)
				{
					if(buttonId == KeyboardDevice::Button::NumpadEnter)
						m_inputBuffer.keyboardBuffer[m_inputBuffer.keyboardBufferPos++] = (u8)KeyboardDevice::Button::Return;
					else
						m_inputBuffer.keyboardBuffer[m_inputBuffer.keyboardBufferPos++] = (u8)buttonId;
				}
			}
		}
		//else
		{
			m_engine->GetInputSystem()->RegisterButtonEvent(deviceHandle, buttonId, pressed);
		}
	}

	void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, MouseDevice::Button buttonId, bool pressed) override
	{
		if (!m_inputEnabled)
			return;

		//if (handle == m_app.GetMainWindowHandle())
		{
			switch (buttonId)
			{
			case MouseDevice::Button::Left:
				m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MouseButton_Left) | (pressed << ImGui::MouseButtonBitOffset_Left); break;
			case MouseDevice::Button::Right:
				m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MouseButton_Right) | (pressed << ImGui::MouseButtonBitOffset_Right); break;
			case MouseDevice::Button::Middle:
				m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MouseButton_Middle) | (pressed << ImGui::MouseButtonBitOffset_Middle); break;
			case MouseDevice::Button::Extra4:
				m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MouseButton_Extra4) | (pressed << ImGui::MouseButtonBitOffset_Extra4); break;
			case MouseDevice::Button::Extra5:
				m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MouseButton_Extra5) | (pressed << ImGui::MouseButtonBitOffset_Extra5); break;
			}
		}
		//else
		{
			m_engine->GetInputSystem()->RegisterButtonEvent(deviceHandle, buttonId, pressed);
		}

	}

	void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, GamepadDevice::Button buttonId, bool pressed) override
	{
		if (!m_inputEnabled)
			return;

		//if (handle != m_app.GetMainWindowHandle())
		{
			m_engine->GetInputSystem()->RegisterButtonEvent(deviceHandle, buttonId, pressed);
		}
	}

	void RegisterAxisEvent(windowHandle handle, inputDeviceHandle deviceHandle, MouseDevice::Axis axisId, const Vector3& delta) override
	{
		if (!m_inputEnabled)
			return;

		//if (handle == m_app.GetMainWindowHandle())
		{
			switch (axisId)
			{
			case Veng::MouseDevice::Axis::Wheel:
				m_inputBuffer.scroll += delta.x;
				break;
			}
		}
		//else
		{
			m_engine->GetInputSystem()->RegisterAxisEvent(deviceHandle, axisId, delta);
		}
	}

	void RegisterAxisEvent(windowHandle handle, inputDeviceHandle deviceHandle, GamepadDevice::Axis axisId, const Vector3& delta) override
	{
		if (!m_inputEnabled)
			return;

		//if (handle != m_app.GetMainWindowHandle())
		{
			m_engine->GetInputSystem()->RegisterAxisEvent(deviceHandle, axisId, delta);
		}
	}

	void MouseMove(i32 xPos, i32 yPos) override
	{
		m_inputBuffer.mousePos.x = (float)xPos;
		m_inputBuffer.mousePos.y = (float)yPos;
	}

	void InputChar(u8 character) override
	{
		if (m_inputEnabled)
		{
			if(0x1f < character && character < 0x7f)
				m_inputBuffer.characterBuffer[m_inputBuffer.characterBufferPos++] = character;
		}
	}

	// WIDGETS

	void InitWidgets()
	{
		WidgetRegistry* registry = GetWidgetRegistries();

		while (registry != nullptr)
		{
			WidgetBase* widget = registry->creator(m_allocator);
			widget->Init(*m_engine, *m_editorInterface);
			m_widgets.PushBack({ true, widget });
			registry = registry->next;
		}
	}

	void DeinitWidgets()
	{
		for (WidgetItem& widget : m_widgets)
		{
			widget.instance->Deinit();
			DELETE_OBJECT(m_allocator, widget.instance);
		}
	}

	// ENGINE

	void InitEngine()
	{
		m_engine = Engine::Create(m_engineAllocator);
		InitSystems();
	}

	void DeinitEngine()
	{
		DeinitSystems();
		Engine::Destroy(m_engine, m_engineAllocator);
	}

	void InitSystems()
	{
		ASSERT(m_engine != nullptr);

		//systems
		m_renderSystem = RenderSystem::Create(*m_engine);
		m_scriptSystem = ScriptSystem::Create(*m_engine);
		m_engine->AddSystem(m_renderSystem);
		m_engine->AddSystem(m_scriptSystem);
		m_renderSystem->Init();
		m_scriptSystem->Init();
	}

	void DeinitSystems()
	{
		m_engine->RemoveSystem(m_scriptSystem->GetName());
		m_engine->RemoveSystem(m_renderSystem->GetName());
		ScriptSystem::Destroy(m_scriptSystem);
		RenderSystem::Destroy(m_renderSystem);
	}

	// RENDERING

	void InitRender()
	{
		windowHandle hwnd = m_app.GetMainWindowHandle();

		bgfx::PlatformData d = { 0 };
		d.nwh = hwnd;
		bgfx::setPlatformData(d);

		bgfx::Init bgfxInit;
		bgfxInit.type = bgfx::RendererType::Count;
		bgfxInit.vendorId = BGFX_PCI_ID_NONE;
		bgfxInit.deviceId = 0;
		bgfxInit.debug = false;
		bgfxInit.profile = false;
		//bgfxInit.resolution;
		//bgfxInit.limits;
		bgfxInit.callback = &m_bgfxCallback;
		bgfxInit.allocator = &m_bgfxAllocator;

		ASSERT(bgfx::init(bgfxInit));

		bgfx::setDebug(BGFX_DEBUG_NONE);//TODO/////////////////////////////////////////
	}

	void DeinitRender()
	{
		bgfx::shutdown();
	}

	// IMGUI

	void InitImgui()
	{
		ImGui::SetAllocatorFunctions(&ImGui::Allocate, &ImGui::Deallocate, &m_imguiAllocator);
		m_imgui = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.IniFilename = "imgui.ini";

		io.KeyMap[ImGuiKey_Tab] = (int)KeyboardDevice::Button::Tab;
		io.KeyMap[ImGuiKey_LeftArrow] = (int)KeyboardDevice::Button::ArrowLeft;
		io.KeyMap[ImGuiKey_RightArrow] = (int)KeyboardDevice::Button::ArrowRight;
		io.KeyMap[ImGuiKey_UpArrow] = (int)KeyboardDevice::Button::ArrowUp;
		io.KeyMap[ImGuiKey_DownArrow] = (int)KeyboardDevice::Button::ArrowDown;
		io.KeyMap[ImGuiKey_PageUp] = (int)KeyboardDevice::Button::PageUp;
		io.KeyMap[ImGuiKey_PageDown] = (int)KeyboardDevice::Button::PageDown;
		io.KeyMap[ImGuiKey_Home] = (int)KeyboardDevice::Button::Home;
		io.KeyMap[ImGuiKey_End] = (int)KeyboardDevice::Button::End;
		io.KeyMap[ImGuiKey_Insert] = (int)KeyboardDevice::Button::Insert;
		io.KeyMap[ImGuiKey_Delete] = (int)KeyboardDevice::Button::Delete;
		io.KeyMap[ImGuiKey_Backspace] = (int)KeyboardDevice::Button::Backspace;
		io.KeyMap[ImGuiKey_Space] = (int)KeyboardDevice::Button::Space;
		io.KeyMap[ImGuiKey_Enter] = (int)KeyboardDevice::Button::Return;
		io.KeyMap[ImGuiKey_Escape] = (int)KeyboardDevice::Button::Escape;

		io.KeyMap[ImGuiKey_A] = (int)ImGui::TextEditSpecials::SelectAll;
		io.KeyMap[ImGuiKey_C] = (int)ImGui::TextEditSpecials::Copy;
		io.KeyMap[ImGuiKey_V] = (int)ImGui::TextEditSpecials::Paste;
		io.KeyMap[ImGuiKey_X] = (int)ImGui::TextEditSpecials::Cut;
		io.KeyMap[ImGuiKey_Y] = (int)ImGui::TextEditSpecials::Redo;
		io.KeyMap[ImGuiKey_Z] = (int)ImGui::TextEditSpecials::Undo;

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&style);
		style.FrameRounding = ImGui::FRAME_ROUNDING;
		style.WindowBorderSize = ImGui::WINDOW_BORDER_SIZE;

		m_imguiBgfxData.viewId = ImGui::VIEW_ID;

		bgfx::setViewName(m_imguiBgfxData.viewId, "ImGui");
		bgfx::setViewMode(m_imguiBgfxData.viewId, bgfx::ViewMode::Sequential);
		bgfx::setViewClear(m_imguiBgfxData.viewId
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, ImGui::CLEAR_COLOR
			, 1.0f
			, 0
		);

		Path vsPath("shaders/compiled/imgui/imgui.vs");
		Path fsPath("shaders/compiled/imgui/imgui.fs");
		m_imguiBgfxData.program = LoadProgram(vsPath, fsPath, m_allocator);
		ASSERT(isValid(m_imguiBgfxData.program));

		m_imguiBgfxData.vertexDecl
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();

		m_imguiBgfxData.textureUniform = bgfx::createUniform("s_tex", bgfx::UniformType::Int1);
		ASSERT(isValid(m_imguiBgfxData.textureUniform));

		io.Fonts->AddFontDefault();
		unsigned char* fontTextureData = nullptr;
		int fontTextureWidth;
		int fontTextureHeight;
		int fontTextureBPP;
		io.Fonts->GetTexDataAsRGBA32(&fontTextureData, &fontTextureWidth, &fontTextureHeight, &fontTextureBPP);

		m_imguiBgfxData.textureFont = bgfx::createTexture2D(
			(uint16_t)fontTextureWidth
			, (uint16_t)fontTextureHeight
			, false
			, 1
			, bgfx::TextureFormat::BGRA8
			, 0
			, bgfx::copy(fontTextureData, fontTextureWidth * fontTextureHeight * fontTextureBPP)
		);
		ASSERT(isValid(m_imguiBgfxData.textureFont));
	}

	void DeinitImgui()
	{
		ImGui::DestroyContext(m_imgui);
		bgfx::destroy(m_imguiBgfxData.program);
		bgfx::destroy(m_imguiBgfxData.textureUniform);
		bgfx::destroy(m_imguiBgfxData.textureFont);
	}

	void UpdateImgui()
	{
		ImGui::NewFrame();

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoDocking;
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowSize(io.DisplaySize);
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace", nullptr, flags);
		ImGui::PopStyleVar();

		float menu_height = 0;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					Path path;
					if (FileSaveDialog(path))
					{
						static const FileMode fileMode{
							FileMode::Access::Write,
							FileMode::ShareMode::ShareWrite,
							FileMode::CreationDisposition::CreateAlways,
							FileMode::FlagNone
						};

						nativeFileHandle fHandle;
						if (FS::OpenFileSync(fHandle, path, fileMode))
						{
							OutputBlob blob(m_allocator);
							ASSERT(m_engine != nullptr);
							m_engine->Serialize(blob);

							if (!FS::WriteFileSync(fHandle, 0, blob.GetData(), blob.GetSize())) {
								ASSERT(false);
								Log(LogType::Error, "Could not write to save file");
							}
							if (!FS::CloseFileSync(fHandle)) {
								ASSERT(false);
								Log(LogType::Error, "Could not close save file");
							}
						}
						else
						{
							Log(LogType::Error, "Could not open save file");
						}
					}
				}
				if (ImGui::MenuItem("Load"))
				{
					Path path;
					if (FileOpenDialog(path))
					{
						const FileMode fMode{
							FileMode::Access::Read,
							FileMode::ShareMode::ShareRead,
							FileMode::CreationDisposition::OpenExisting,
							FileMode::FlagNone
						};

						nativeFileHandle fh;
						if (FS::OpenFileSync(fh, path, fMode))
						{

							size_t fSize = FS::GetFileSize(fh);
							void* fileBuffer = m_allocator.Allocate(fSize, alignof(u8));
							size_t bytesRead = 0;
							if (FS::ReadFileSync(fh, 0, fileBuffer, fSize, bytesRead))
							{
								ASSERT2(fSize == bytesRead, "Read size is not equal file size of input file");

								InputBlob blob(fileBuffer, fSize);
								ASSERT(m_engine != nullptr);
								m_engine->Deserialize(blob);
							}
							else
							{
								ASSERT(false);
								Log(LogType::Error, "Could not read from save file");
							}

							if (!FS::CloseFileSync(fh)) {
								ASSERT(false);
								Log(LogType::Error, "Could not close load file");
							}

							m_allocator.Deallocate(fileBuffer);
						}
						else
						{
							ASSERT(false);
							Log(LogType::Error, "Could not open input file");
						}
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Widgets"))
			{
				for (WidgetItem& widget : m_widgets)
					ImGui::MenuItem(widget.instance->GetName(), nullptr, &widget.isOpen);
				ImGui::EndMenu();
			}
			menu_height = ImGui::GetWindowSize().y;
			ImGui::EndMainMenuBar();
		}

		ImGui::SetCursorPosY(menu_height);
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0, 0));
		ImGui::End();

		for (WidgetItem& widget : m_widgets)
		{
			if (!widget.isOpen) continue;

			ImGuiWindowFlags flags = widget.instance->HasMenuBar() ? ImGuiWindowFlags_MenuBar : ImGuiWindowFlags_None;
			if (ImGui::Begin(widget.instance->GetName(), &widget.isOpen, flags)) {
				widget.instance->Render(m_eventQueue);
			}
			ImGui::End();
		}

		ImGui::Render();
	}

	void UpdateImguiInput(float deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();

		for(int i = 0; i < m_inputBuffer.characterBufferPos; ++i)
		{
			u8 inputChar = m_inputBuffer.characterBuffer[i];
			if(inputChar < 0x7f)
			{
				io.AddInputCharacter(inputChar); // ASCII only! :(
			}
		}

		memory::Set(io.KeysDown, 0, sizeof(io.KeysDown));
		for (int i = 0; i < m_inputBuffer.keyboardBufferPos; ++i)
		{
			io.KeysDown[m_inputBuffer.keyboardBuffer[i]] = true;
		}

		io.DeltaTime = SecFromMSec(deltaTime); //msec to sec

		io.MousePos = m_inputBuffer.mousePos;
		io.MouseDown[0] = 0 != (m_inputBuffer.mouseButtons & ImGui::MouseButton_Left);
		io.MouseDown[1] = 0 != (m_inputBuffer.mouseButtons & ImGui::MouseButton_Right);
		io.MouseDown[2] = 0 != (m_inputBuffer.mouseButtons & ImGui::MouseButton_Middle);
		io.MouseWheel = m_inputBuffer.scroll;

		io.KeyCtrl = 0 != (m_inputBuffer.modifierKeys & ImGui::ModifierKey_Ctrl);
		io.KeyShift = 0 != (m_inputBuffer.modifierKeys & ImGui::ModifierKey_Shift);
		io.KeyAlt = 0 != (m_inputBuffer.modifierKeys & ImGui::ModifierKey_Alt);
		io.KeySuper = 0 != (m_inputBuffer.modifierKeys & ImGui::ModifierKey_Super);
		///* bool    */io.KeysDown[512];                  // Keyboard keys that are pressed (ideally left in the "native" order your engine has access to keyboard keys, so you can use your own defines/enums for keys).
		///* ImWchar */io.InputCharacters[16 + 1];          // List of characters input (translated by user from keypress+keyboard state). Fill using AddInputCharacter() helper.
		///* float   */io.NavInputs[ImGuiNavInput_COUNT]; // Gamepad inputs (keyboard keys will be auto-mapped and be written here by ImGui::NewFrame, all values will be cleared back to zero in ImGui::EndFrame)

		m_inputBuffer.keyboardBufferPos = 0;
		m_inputBuffer.characterBufferPos = 0;
		m_inputBuffer.scroll = 0;
	}

	void RenderImgui()
	{
		ImDrawData* drawData = ImGui::GetDrawData();

		const bgfx::Caps* caps = bgfx::getCaps();
		{
			//float ortho[16];
			//bx::mtxOrtho(ortho, 0.0f, (float)m_windowSize.x, (float)m_windowSize.y, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			//bgfx::setViewTransform(m_imguiBgfxData.viewId, nullptr, ortho);
			//bgfx::setViewRect(m_imguiBgfxData.viewId, 0, 0, uint16_t(m_windowSize.x), uint16_t(m_windowSize.y));/////////////////////

			Matrix44 mat;
			mat.SetOrthogonal(0.0f, (float)m_windowSize.x, (float)m_windowSize.y, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			bgfx::setViewTransform(m_imguiBgfxData.viewId, nullptr, &mat.m11);
		}

		// Render command lists
		for (int32_t ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* drawList = drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices = (uint32_t)drawList->IdxBuffer.size();

			if (!bgfx::checkAvailTransientBuffers(numVertices, m_imguiBgfxData.vertexDecl, numIndices))
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_imguiBgfxData.vertexDecl);
			bgfx::allocTransientIndexBuffer(&tib, numIndices);

			ImDrawVert* verts = (ImDrawVert*)tvb.data;
			memory::Copy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert));

			ImDrawIdx* indices = (ImDrawIdx*)tib.data;
			memory::Copy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));

			uint32_t offset = 0;
			for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
			{
				if (cmd->UserCallback)
				{
					cmd->UserCallback(drawList, cmd);
				}
				else if (0 != cmd->ElemCount)
				{
					uint64_t state = 0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_MSAA
						;

					bgfx::TextureHandle th = m_imguiBgfxData.textureFont;

					if (nullptr != cmd->TextureId)
					{
						//TODO: image blending
						//TODO: image lods
						//TODO: image alpha
						th = bgfx::getTexture(*(bgfx::FrameBufferHandle*)cmd->TextureId);
					}
					else
					{
						state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
					}

					const uint16_t xx = uint16_t(Max(cmd->ClipRect.x, 0.0f));
					const uint16_t yy = uint16_t(Max(cmd->ClipRect.y, 0.0f));
					bgfx::setScissor(xx, yy
						, uint16_t(Min(cmd->ClipRect.z, 65535.0f) - xx)
						, uint16_t(Min(cmd->ClipRect.w, 65535.0f) - yy)
					);

					bgfx::setState(state);
					bgfx::setTexture(0, m_imguiBgfxData.textureUniform, th);
					bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
					bgfx::setIndexBuffer(&tib, offset, cmd->ElemCount);
					bgfx::submit(m_imguiBgfxData.viewId, m_imguiBgfxData.program);
				}

				offset += cmd->ElemCount;
			}
		}
	}

private:
	struct WidgetItem
	{
		bool isOpen;
		WidgetBase* instance;
	};

private:
	Allocator& m_allocator;
	EditorInterface* m_editorInterface = nullptr;
	App& m_app;
	ProxyAllocator m_engineAllocator;
	Engine* m_engine = nullptr;
	bool m_inputEnabled = false;
	AssociativeArray<inputDeviceHandle, InputKeyboardFiltering> m_inputKeyboardFilter;

	//imgui
	ProxyAllocator m_imguiAllocator;
	Input m_inputBuffer;
	ImGuiContext* m_imgui;

	//widgets
	EventQueue m_eventQueue;
	Array<WidgetItem> m_widgets;

	//bgfx for imgui
	ImguiBgfxData m_imguiBgfxData;

	//bgfx stuff
	BGFXAllocator m_bgfxAllocator;
	BGFXCallback m_bgfxCallback;

	WindowSize m_windowSize = { 0, 0 };//doesn't need yet, should remove ?

};


EditorApp* EditorApp::Create(Allocator& allocator, App& app)
{
	return NEW_OBJECT(allocator, EditorAppImpl)(allocator, app);
}

void EditorApp::Destroy(EditorApp* editor, Allocator& allocator)
{
	DELETE_OBJECT(allocator, editor);
}


}

}
