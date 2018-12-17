#include "editor.h"

#include <bgfx/bgfx.h>

#include "core/allocators.h"
#include "core/engine.h"
#include "core/logs.h"
#include "core/asserts.h"
#include "core/file/blob.h"
#include "core/containers/associative_array.h"


#include "renderer/renderer.h"////////////////////////
#include "script/script.h"////////////////////////////
#include "core/file/path.h"////////////////////////////////
#include "core/math/math.h"////////////////////////////////
#include "core/file/file.h"////////////////////
#include "core/file/file_system.h"/////////////////
#include "core/memory.h"/////////////////////
#include "core/time.h"////////////////////////////
#include "core/math/matrix.h"////////////////////////////////////

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"
#include "core/resource/resource_management.h"
#include "renderer/resource_managers/shader_manager.h"
#include "renderer/resource_managers/material_manager.h"
#include "renderer/resource_managers/model_manager.h"
#include "renderer/resource_managers/texture_manager.h"

#include "core/input/devices/input_device_keyboard.h"

#include <bgfx/bgfx.h>
#include "../external/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"

#include "editor/event_queue.h"
#include "editor/widget_base.h"
#include "editor/widget_register.h"

#include <core/os/win/simple_windows.h>



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

	explicit BGFXAllocator(IAllocator& source)
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


	IAllocator& m_source;
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
	Veng::IAllocator* allocator = static_cast<Veng::IAllocator*>(userData);
	return allocator->Allocate(sz, alignof(char));
}

void Deallocate(void* ptr, void* userData)
{
	if (ptr != nullptr)
	{
		Veng::IAllocator* allocator = static_cast<Veng::IAllocator*>(userData);
		allocator->Deallocate(ptr);
	}
}

}

namespace ImGui
{

typedef Veng::u32 MouseButtonFlags;
enum MouseButtonBitOffset : MouseButtonFlags
{
	MB_BO_LEFT = 0,
	MB_BO_RIGHT,
	MB_BO_MIDDLE,
	MB_BO_EXTRA4,
	MB_BO_EXTRA5,
};
enum MouseButtonBits : MouseButtonFlags
{
	MB_NONE = 0,
	MB_LEFT_BIT = 1 << MB_BO_LEFT,
	MB_RIGHT_BIT = 1 << MB_BO_RIGHT,
	MB_MIDDLE_BIT = 1 << MB_BO_MIDDLE,
	MB_EXTRA4_BIT = 1 << MB_BO_EXTRA4,
	MB_EXTRA5_BIT = 1 << MB_BO_EXTRA5,
};

typedef Veng::u32 ModifierKeyFlags;
enum ModifierKeyBitOffset : ModifierKeyFlags
{
	MK_BO_CTRL = 0,
	MK_BO_SHIFT,
	MK_BO_ALT,
	MK_BO_SUPER,
};
enum ModifierKeyBits : ModifierKeyFlags
{
	MK_NONE = 0,
	MK_CTRL_BIT = 1 << MK_BO_CTRL,
	MK_SHIFT_BIT = 1 << MK_BO_SHIFT,
	MK_ALT_BIT = 1 << MK_BO_ALT,
	MK_SUPER_BIT = 1 << MK_BO_SUPER,
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


RenderSystem* m_renderSystem = nullptr;////////////////////////////////
ScriptSystem* m_scriptSystem = nullptr;////////////////////////////////

ShaderInternalManager* m_shaderInternalManager = nullptr;////////////////////////////////
ShaderManager* m_shaderManager = nullptr;////////////////////////////////
MaterialManager* m_materialManager = nullptr;////////////////////////////////
ModelManager* m_modelManager = nullptr;////////////////////////////////
TextureManager* m_textureManager = nullptr;////////////////////////////////


namespace Editor
{

static const Path IMGUI_DOCK_DATA_PATH = Path("imgui_dock_data.bin");

struct Input
{
	ImVec2 mousePos = { 0.0f, 0.0f };
	ImGui::MouseButtonFlags mouseButtons = ImGui::MB_NONE;
	ImGui::ModifierKeyFlags modifierKeys = ImGui::MK_NONE;
	static const size_t KEYBOARD_BUFFER_SIZE = 32;
	u8 keyboardBuffer[KEYBOARD_BUFFER_SIZE];
	int keyboardBufferPos = 0;
	static const size_t CHARACTER_BUFFER_SIZE = 16;
	u8 characterBuffer[CHARACTER_BUFFER_SIZE];
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

static bgfx::ProgramHandle LoadProgram(const Path& vertexPath, const Path& fragmentPath, IAllocator& allocator)
{
	static const FileMode fileMode{
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
	EditorAppImpl(IAllocator& allocator, App& app)
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

		InitWidgets();
	}

	void Deinit() override
	{
		DeinitWidgets();
		DeinitEngine();//TODO: shut down engine gracefully

		DeinitImgui();
		DeinitRender();
	}

	void Update(float deltaTime) override
	{
		UpdateImguiInput(deltaTime);

		m_engine->Update(deltaTime);
		m_eventQueue.FrameUpdate();

		for(WidgetBase* widget : m_widgets)
		{
			widget->Update(m_eventQueue);
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

		if (handle == m_app.GetMainWindowHandle())
		{
			ImGuiIO& io = ImGui::GetIO();
			switch (buttonId)
			{
			case KeyboardDevice::Button::ControlLeft:
			case KeyboardDevice::Button::ControlRight:
				m_inputBuffer.modifierKeys = (m_inputBuffer.modifierKeys & ~ImGui::MK_CTRL_BIT) | (pressed << ImGui::MK_BO_CTRL); break;
			case KeyboardDevice::Button::ShiftLeft:
			case KeyboardDevice::Button::ShiftRight:
				m_inputBuffer.modifierKeys = (m_inputBuffer.modifierKeys & ~ImGui::MK_SHIFT_BIT) | (pressed << ImGui::MK_BO_SHIFT); break;
			case KeyboardDevice::Button::AltLeft:
			case KeyboardDevice::Button::AltRight:
				m_inputBuffer.modifierKeys = (m_inputBuffer.modifierKeys & ~ImGui::MK_ALT_BIT) | (pressed << ImGui::MK_BO_ALT); break;
			case KeyboardDevice::Button::GUILeft:
			case KeyboardDevice::Button::GUIRight:
				m_inputBuffer.modifierKeys = (m_inputBuffer.modifierKeys & ~ImGui::MK_SUPER_BIT) | (pressed << ImGui::MK_BO_SUPER); break;
			default:
				if (pressed)
					m_inputBuffer.keyboardBuffer[m_inputBuffer.keyboardBufferPos++] = (u8)buttonId;
			}
		}
		else
		{
			m_engine->GetInputSystem()->RegisterButtonEvent(deviceHandle, buttonId, pressed);
		}
	}

	void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, MouseDevice::Button buttonId, bool pressed) override
	{
		if (m_inputEnabled)
		{
			if (handle == m_app.GetMainWindowHandle())
			{
				switch (buttonId)
				{
				case MouseDevice::Button::Left:
					m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MB_LEFT_BIT) | (pressed << ImGui::MB_BO_LEFT); break;
				case MouseDevice::Button::Right:
					m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MB_RIGHT_BIT) | (pressed << ImGui::MB_BO_RIGHT); break;
				case MouseDevice::Button::Middle:
					m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MB_MIDDLE_BIT) | (pressed << ImGui::MB_BO_MIDDLE); break;
				case MouseDevice::Button::Extra4:
					m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MB_EXTRA4_BIT) | (pressed << ImGui::MB_BO_EXTRA4); break;
				case MouseDevice::Button::Extra5:
					m_inputBuffer.mouseButtons = (m_inputBuffer.mouseButtons & ~ImGui::MB_EXTRA5_BIT) | (pressed << ImGui::MB_BO_EXTRA5); break;
				}
			}
			else
			{
				m_engine->GetInputSystem()->RegisterButtonEvent(deviceHandle, buttonId, pressed);
			}
		}
	}

	void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, GamepadDevice::Button buttonId, bool pressed) override
	{
		if (m_inputEnabled)
		{
			if (handle != m_app.GetMainWindowHandle())
			{
				m_engine->GetInputSystem()->RegisterButtonEvent(deviceHandle, buttonId, pressed);
			}
		}
	}

	void RegisterAxisEvent(windowHandle handle, inputDeviceHandle deviceHandle, MouseDevice::Axis axisId, const Vector3& delta) override
	{
		if (m_inputEnabled)
		{
			if (handle == m_app.GetMainWindowHandle())
			{
				switch (axisId)
				{
				case Veng::MouseDevice::Axis::Wheel:
					m_inputBuffer.scroll += delta.x;
					break;
				}
			}
			else
			{
				m_engine->GetInputSystem()->RegisterAxisEvent(deviceHandle, axisId, delta);
			}
		}
	}

	void RegisterAxisEvent(windowHandle handle, inputDeviceHandle deviceHandle, GamepadDevice::Axis axisId, const Vector3& delta) override
	{
		if (m_inputEnabled)
		{
			if (handle != m_app.GetMainWindowHandle())
			{
				m_engine->GetInputSystem()->RegisterAxisEvent(deviceHandle, axisId, delta);
			}
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
		WidgetRegistry* registry = GetRegistries();

		while (registry != nullptr)
		{
			WidgetBase* widget = registry->creator(m_allocator);
			widget->Init(m_allocator, *m_engine);
			m_widgets.PushBack(widget);
			registry = registry->next;
		}
	}

	void DeinitWidgets()
	{
		for (WidgetBase* widget : m_widgets)
		{
			widget->Deinit();
			DELETE_OBJECT(m_allocator, widget);
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
		
		//resource managers
		ResourceManagement* resourceManagement = m_engine->GetResourceManagement();
		m_shaderInternalManager = NEW_OBJECT(m_allocator, ShaderInternalManager)(m_allocator, *m_engine->GetFileSystem(), resourceManagement);
		m_shaderManager = NEW_OBJECT(m_allocator, ShaderManager)(m_allocator, *m_engine->GetFileSystem(), resourceManagement);
		m_materialManager = NEW_OBJECT(m_allocator, MaterialManager)(m_allocator, *m_engine->GetFileSystem(), resourceManagement);
		m_modelManager = NEW_OBJECT(m_allocator, ModelManager)(m_allocator, *m_engine->GetFileSystem(), resourceManagement);
		m_textureManager = NEW_OBJECT(m_allocator, TextureManager)(m_allocator, *m_engine->GetFileSystem(), resourceManagement);

		resourceManagement->RegisterManager(ResourceType::ShaderInternal, m_shaderInternalManager);
		resourceManagement->RegisterManager(ResourceType::Shader, m_shaderManager);
		resourceManagement->RegisterManager(ResourceType::Material, m_materialManager);
		resourceManagement->RegisterManager(ResourceType::Model, m_modelManager);
		resourceManagement->RegisterManager(ResourceType::Texture, m_textureManager);

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

		DELETE_OBJECT(m_allocator, m_modelManager);
		DELETE_OBJECT(m_allocator, m_materialManager);
		DELETE_OBJECT(m_allocator, m_shaderManager);
		DELETE_OBJECT(m_allocator, m_shaderInternalManager);
		DELETE_OBJECT(m_allocator, m_textureManager);
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
		io.IniFilename = nullptr;

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

		ImGui::CreateDockContext();

		LoadImguiSettings();
	}

	void DeinitImgui()
	{
		SaveImguiSettings();

		ImGui::DestroyDockContext();
		ImGui::DestroyContext(m_imgui);
		bgfx::destroy(m_imguiBgfxData.program);
		bgfx::destroy(m_imguiBgfxData.textureUniform);
		bgfx::destroy(m_imguiBgfxData.textureFont);
	}

	void UpdateImgui()
	{
		ImGui::NewFrame();

		ImGui::RootDock(ImVec2(0, 0), ImGui::GetIO().DisplaySize);

		for (WidgetBase* widget : m_widgets)
		{
			widget->Render(m_eventQueue);
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
				io.AddInputCharacter(inputChar); // ASCII or GTFO! :(
			}
		}

		memory::Set(io.KeysDown, 0, sizeof(io.KeysDown));
		for (int i = 0; i < m_inputBuffer.keyboardBufferPos; ++i)
		{
			io.KeysDown[m_inputBuffer.keyboardBuffer[i]] = true;
		}

		io.DeltaTime = SecFromMSec(deltaTime); //msec to sec

		io.MousePos = m_inputBuffer.mousePos;
		io.MouseDown[0] = 0 != (m_inputBuffer.mouseButtons & ImGui::MB_LEFT_BIT);
		io.MouseDown[1] = 0 != (m_inputBuffer.mouseButtons & ImGui::MB_RIGHT_BIT);
		io.MouseDown[2] = 0 != (m_inputBuffer.mouseButtons & ImGui::MB_MIDDLE_BIT);
		io.MouseWheel = m_inputBuffer.scroll;

		io.KeyCtrl = 0 != (m_inputBuffer.modifierKeys & ImGui::MK_CTRL_BIT);
		io.KeyShift = 0 != (m_inputBuffer.modifierKeys & ImGui::MK_SHIFT_BIT);
		io.KeyAlt = 0 != (m_inputBuffer.modifierKeys & ImGui::MK_ALT_BIT);
		io.KeySuper = 0 != (m_inputBuffer.modifierKeys & ImGui::MK_SUPER_BIT);
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

	void SaveImguiSettings()
	{
		static const FileMode fileMode{
			FileMode::Access::Write,
			FileMode::ShareMode::ShareWrite,
			FileMode::CreationDisposition::CreateAlways,
			FileMode::FlagNone
		};

		nativeFileHandle fHandle;
		if(FS::OpenFileSync(fHandle, IMGUI_DOCK_DATA_PATH, fileMode))
		{
			OutputBlob blob(m_allocator);
			ImGui::SerializeDock(blob);

			ASSERT(FS::WriteFileSync(fHandle, 0, blob.GetData(), blob.GetSize()));
			ASSERT(FS::CloseFileSync(fHandle));
		}
	}

	void LoadImguiSettings()
	{
		static const FileMode fileMode{
			FileMode::Access::Read,
			FileMode::ShareMode::ShareRead,
			FileMode::CreationDisposition::OpenExisting,
			FileMode::FlagNone
		};

		nativeFileHandle fHandle;
		if(FS::OpenFileSync(fHandle, IMGUI_DOCK_DATA_PATH, fileMode))
		{
			size_t fileSize = FS::GetFileSize(fHandle);
			u8* data = (u8*)m_allocator.Allocate(fileSize, alignof(u8));
			size_t fileSizeRead = 0;
			ASSERT(FS::ReadFileSync(fHandle, 0, data, fileSize, fileSizeRead));
			ASSERT(fileSize == fileSizeRead);
			ASSERT(FS::CloseFileSync(fHandle));

			InputBlob blob(data, fileSize);
			ImGui::DeserializeDock(blob);

			m_allocator.Deallocate(data);
		}
	}


private:
	IAllocator& m_allocator;
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
	Array<WidgetBase*> m_widgets;

	//bgfx for imgui
	ImguiBgfxData m_imguiBgfxData;

	//bgfx stuff ////////////////////////////////////////////////
	BGFXAllocator m_bgfxAllocator;
	BGFXCallback m_bgfxCallback;

	WindowSize m_windowSize = { 0, 0 };//doesn't need yet, should remove ?

};


EditorApp* EditorApp::Create(IAllocator& allocator, App& app)
{
	return NEW_OBJECT(allocator, EditorAppImpl)(allocator, app);
}

void EditorApp::Destroy(EditorApp* editor, IAllocator& allocator)
{
	DELETE_OBJECT(allocator, editor);
}


}

}
