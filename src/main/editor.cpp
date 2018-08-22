#include "editor.h"

#include <bgfx/bgfx.h>

#include "core/allocators.h"
#include "core/engine.h"
#include "core/logs.h"
#include "core/asserts.h"


#include "renderer/irenderer.h"////////////////////////
#include "core/file/path.h"////////////////////////////////
#include "core/math.h"////////////////////////////////
#include "core/file/file.h"////////////////////
#include "core/memory.h"/////////////////////
#include "core/time.h"////////////////////////////
#include "core/matrix.h"////////////////////////////////////

#include <bgfx/bgfx.h>///////////////
#include "../external/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"

#include "editor/plugins/worlds_widget.h"
#include "editor/plugins/entities_widget.h"


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

	void mtxOrtho(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _oglNdc);
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
	return allocator->Allocate(sz, ALIGN_OF(char));
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


static const float FRAME_ROUNDING = 4.0f;
static const float WINDOW_BORDER_SIZE = 0.0f;

}


namespace Veng
{


RenderSystem* m_renderSystem = nullptr;////////////////////////////////


namespace Editor
{


struct Input
{
	ImVec2 m_mousePos = { 0.0f, 0.0f };
	ImGui::MouseButtonFlags m_mouseButtons = ImGui::MB_NONE;
	ImGui::ModifierKeyFlags m_modifierKeys = ImGui::MK_NONE;
	static const size_t KEYBOARD_BUFFER_SIZE = 32;
	u8 m_keyboardBuffer[KEYBOARD_BUFFER_SIZE];
	float m_scroll = 0.0f;
};


class EditorAppImpl : public EditorApp
{
public:
	EditorAppImpl(IAllocator& allocator, App& app)
		: m_allocator(allocator)
		, m_app(app)
		, m_bgfxAllocator(m_allocator)
		, m_imguiAllocator(m_allocator)
	{

	}

	void Init() override
	{
		InitRender();
		InitImgui();

		m_engine = Engine::Create(m_allocator);
		Engine::PlatformData platformData;
		platformData.windowHndl = m_subHwnd;
		m_engine->SetPlatformData(platformData);
		InitSystems();
		m_worldsWidget.SetEngine(m_engine);


		{//DUMMY GAMEPLAY CODE //////////////////////////////////////////////////////////////////////////////////////////
			worldId worldHandle = m_engine->AddWorld();
			World* world = m_engine->GetWorld(worldHandle);
			RenderScene* renderScene = static_cast<RenderScene*>(m_renderSystem->GetScene());
			Entity camEnt = world->CreateEntity();
			Transform& camTrans = world->GetEntityTransform(camEnt);
			renderScene->AddCameraComponent(camEnt, worldHandle, 60.0_deg, 0.001f, 100.0f);

			for (unsigned yy = 0; yy < 11; ++yy)
			{
				for (unsigned xx = 0; xx < 11; ++xx)
				{
					Entity entity = world->CreateEntity();
					Transform& trans = world->GetEntityTransform(entity);
					renderScene->AddModelComponent(entity, worldHandle, "models/cubes.model");

					Quaternion rot = Quaternion::IDENTITY;
					rot = rot * Quaternion(Vector3::AXIS_X, xx*0.21f);
					rot = rot * Quaternion(Vector3::AXIS_Y, yy*0.37f);
					Vector3 pos = {
						-15.0f + float(xx) * 3.0f,
						-15.0f + float(yy) * 3.0f,
						0.0f
					};
					trans = Transform(rot, pos);
				}
			}
		}
	}

	void Deinit() override
	{
		//TODO: shut down engine gracefully
		DeinitSystems();
		Engine::Destroy(m_engine, m_allocator);

		DeinitImgui();
		DeinitRender();
	}

	void Update(float deltaTime) override
	{
		UpdateImguiInput(deltaTime);
		UpdateImgui();

		UpdateRender();

		RenderImgui();

		bgfx::setViewFrameBuffer(1, m_fbh);////////////////////////////////////
		bgfx::touch(1);///////////////////////////

		m_engine->Update(deltaTime);

		m_inputBuffer.m_scroll = 0;

		bgfx::frame();//flip buffers
	}


	void Resize(windowHandle handle, i32 width, i32 height) override
	{
		if (m_renderSystem && handle == m_subHwnd)
		{
			bgfx::destroy(m_fbh);
			m_fbh = bgfx::createFrameBuffer(m_subHwnd, uint16_t(width), uint16_t(height));
			bgfx::setViewRect(1, 0, 0, uint16_t(width), uint16_t(height));////////////////////////////////
			m_renderSystem->Resize(width, height);
		}
		else if (handle == m_app.GetMainWindowHandle())
		{
			m_windowSize = { width, height };
			bgfx::reset(width, height, BGFX_RESET_VSYNC);
			bgfx::setViewRect(m_viewId, 0, 0, uint16_t(width), uint16_t(height));
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2((float)width, (float)height);
		}
	}

	void SetFocus(windowHandle handle, bool hasFocus) override
	{
		handle;///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (hasFocus)
			m_inputEnabled = true;
		else
			m_inputEnabled = false;
	}

	inputDeviceID RegisterDevice(inputDeviceHandle deviceHandle, InputDeviceCategory category, const String& name) override
	{
		return m_engine->GetInputSystem()->RegisterDevice(deviceHandle, category, name);
	}

	void UnregisterDevice(inputDeviceHandle deviceHandle) override
	{
		m_engine->GetInputSystem()->UnregisterDevice(deviceHandle);
	}

	void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, KeyboardDevice::Button buttonId, bool pressed) override
	{
		if (m_inputEnabled)
		{
			if (handle == m_app.GetMainWindowHandle())
			{
				ImGuiIO& io = ImGui::GetIO();
				switch (buttonId)
				{
				case KeyboardDevice::Button::ControlLeft:
				case KeyboardDevice::Button::ControlRight:
					m_inputBuffer.m_modifierKeys = (m_inputBuffer.m_modifierKeys & ~ImGui::MK_CTRL_BIT) | (pressed << ImGui::MK_BO_CTRL); break;
				case KeyboardDevice::Button::ShiftLeft:
				case KeyboardDevice::Button::ShiftRight:
					m_inputBuffer.m_modifierKeys = (m_inputBuffer.m_modifierKeys & ~ImGui::MK_SHIFT_BIT) | (pressed << ImGui::MK_BO_SHIFT); break;
				case KeyboardDevice::Button::AltLeft:
				case KeyboardDevice::Button::AltRight:
					m_inputBuffer.m_modifierKeys = (m_inputBuffer.m_modifierKeys & ~ImGui::MK_ALT_BIT) | (pressed << ImGui::MK_BO_ALT); break;
				case KeyboardDevice::Button::GUILeft:
				case KeyboardDevice::Button::GUIRight:
					m_inputBuffer.m_modifierKeys = (m_inputBuffer.m_modifierKeys & ~ImGui::MK_SUPER_BIT) | (pressed << ImGui::MK_BO_SUPER); break;
				}
			}
			else
			{
				m_engine->GetInputSystem()->RegisterButtonEvent(deviceHandle, buttonId, pressed);
			}
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
					m_inputBuffer.m_mouseButtons = (m_inputBuffer.m_mouseButtons & ~ImGui::MB_LEFT_BIT) | (pressed << ImGui::MB_BO_LEFT); break;
				case MouseDevice::Button::Right:
					m_inputBuffer.m_mouseButtons = (m_inputBuffer.m_mouseButtons & ~ImGui::MB_RIGHT_BIT) | (pressed << ImGui::MB_BO_RIGHT); break;
				case MouseDevice::Button::Middle:
					m_inputBuffer.m_mouseButtons = (m_inputBuffer.m_mouseButtons & ~ImGui::MB_MIDDLE_BIT) | (pressed << ImGui::MB_BO_MIDDLE); break;
				case MouseDevice::Button::Extra4:
					m_inputBuffer.m_mouseButtons = (m_inputBuffer.m_mouseButtons & ~ImGui::MB_EXTRA4_BIT) | (pressed << ImGui::MB_BO_EXTRA4); break;
				case MouseDevice::Button::Extra5:
					m_inputBuffer.m_mouseButtons = (m_inputBuffer.m_mouseButtons & ~ImGui::MB_EXTRA5_BIT) | (pressed << ImGui::MB_BO_EXTRA5); break;
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
					m_inputBuffer.m_scroll += delta.x;
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

	virtual void MouseMove(i32 xPos, i32 yPos)
	{
		m_inputBuffer.m_mousePos.x = (float)xPos;
		m_inputBuffer.m_mousePos.y = (float)yPos;
	}


	void InitSystems()
	{
		ASSERT(m_engine != nullptr);
		m_renderSystem = RenderSystem::Create(*m_engine);
		m_renderSystem->Init();
		WindowSize size = m_app.GetWindowSize(m_subHwnd);
		m_renderSystem->Resize(size.x, size.y);
		m_engine->AddPlugin(m_renderSystem);
	}

	void DeinitSystems()
	{
		RenderSystem::Destroy(m_renderSystem);
	}


	void InitRender()
	{
		windowHandle hwnd = m_app.GetMainWindowHandle();
		m_subHwnd = m_app.CreateSubWindow();
		ASSERT(m_subHwnd != nullptr);

		WindowSize subWindowSize = m_app.GetWindowSize(m_subHwnd);

		bgfx::PlatformData d{ 0 };
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

		bgfx::init(bgfxInit);

		bgfx::setDebug(BGFX_DEBUG_NONE);//TODO

		bgfx::setViewRect(1, 0, 0, subWindowSize.x, subWindowSize.y);////////////////////////////////////////////////
		m_fbh = bgfx::createFrameBuffer(m_subHwnd, subWindowSize.x, subWindowSize.y);

		bgfx::setViewClear(m_viewId
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x404040ff
			, 1.0f
			, 0
		);

		bgfx::setViewClear(1////////////////////////////////////////////////////////////////////////////////
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
		);
	}

	void DeinitRender()
	{
		bgfx::destroy(m_fbh);
		bgfx::destroy(m_program);
		bgfx::shutdown();
		//m_app.DestroySubWindow(m_subHwnd);
	}

	void UpdateRender()
	{
		bgfx::setViewFrameBuffer(m_viewId, BGFX_INVALID_HANDLE);
		bgfx::touch(m_viewId);
	}


	void InitImgui()
	{
		ImGui::SetAllocatorFunctions(&ImGui::Allocate, &ImGui::Deallocate, &m_imguiAllocator);
		m_imgui = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&style);
		style.FrameRounding = ImGui::FRAME_ROUNDING;
		style.WindowBorderSize = ImGui::WINDOW_BORDER_SIZE;

		bgfx::RendererType::Enum type = bgfx::getRendererType();


		Path vsPath("shaders/compiled/imgui/imgui.vs");
		Path fsPath("shaders/compiled/imgui/imgui.fs");
		FileMode fileMode{
			FileMode::Access::Read,
			FileMode::ShareMode::ShareRead,
			FileMode::CreationDisposition::OpenExisting,
			FileMode::FlagNone
		};

		//vertex shader
		nativeFileHandle vsHandle;
		ASSERT(FS::OpenFileSync(vsHandle, vsPath, fileMode));
		size_t vsFileSize = FS::GetFileSize(vsHandle);
		u8* vsData = (u8*)m_allocator.Allocate(vsFileSize, ALIGN_OF(u8));
		size_t vsFileSizeRead = 0;
		FS::ReadFileSync(vsHandle, 0, vsData, vsFileSize, vsFileSizeRead);
		ASSERT(vsFileSize == vsFileSizeRead);
		ASSERT(FS::CloseFileSync(vsHandle));

		const bgfx::Memory* vsMem = bgfx::alloc((u32)vsFileSize + 1);
		memory::Copy(vsMem->data, vsData, vsFileSize);
		vsMem->data[vsMem->size - 1] = '\0';
		bgfx::ShaderHandle vsHandleBgfx = bgfx::createShader(vsMem);
		ASSERT(bgfx::isValid(vsHandleBgfx));
		m_allocator.Deallocate(vsData);

		//fragment shader
		nativeFileHandle fsHandle;
		ASSERT(FS::OpenFileSync(fsHandle, fsPath, fileMode));
		size_t fsFileSize = FS::GetFileSize(fsHandle);
		u8* fsData = (u8*)m_allocator.Allocate(fsFileSize, ALIGN_OF(u8));
		size_t fsFileSizeRead = 0;
		FS::ReadFileSync(fsHandle, 0, fsData, fsFileSize, fsFileSizeRead);
		ASSERT(fsFileSize == fsFileSizeRead);
		ASSERT(FS::CloseFileSync(fsHandle));

		const bgfx::Memory* fsMem = bgfx::alloc((u32)fsFileSize + 1);
		memory::Copy(fsMem->data, fsData, fsFileSize);
		fsMem->data[fsMem->size - 1] = '\0';
		bgfx::ShaderHandle fsHandleBgfx = bgfx::createShader(fsMem);
		ASSERT(bgfx::isValid(fsHandleBgfx));
		m_allocator.Deallocate(fsData);


		m_program = bgfx::createProgram(vsHandleBgfx, fsHandleBgfx, true);

		m_decl
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();

		s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Int1);

		//ImGui::InitDockContext();

		io.Fonts->AddFontDefault();
		unsigned char* fontTextureData = nullptr;
		int fontTextureWidth;
		int fontTextureHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontTextureData, &fontTextureWidth, &fontTextureHeight);

		m_texture = bgfx::createTexture2D(
			(uint16_t)fontTextureWidth
			, (uint16_t)fontTextureHeight
			, false
			, 1
			, bgfx::TextureFormat::BGRA8
			, 0
			, bgfx::copy(fontTextureData, fontTextureWidth * fontTextureHeight * 4)
		);

		ImGui::CreateDockContext();
	}

	void DeinitImgui()
	{
		ImGui::DestroyDockContext();
		ImGui::DestroyContext(m_imgui);
		bgfx::destroy(s_tex);
		bgfx::destroy(m_texture);
	}

	void UpdateImgui()
	{
		ImGui::NewFrame();

		ImGui::RootDock(ImVec2(0, 0), ImGui::GetIO().DisplaySize);

		ImGui::BeginDock("Engine");

		ImVec2 windowPosition = ImGui::GetWindowPos();
		if (windowPosition != m_subWindowPosition)
		{
			m_subWindowPosition = windowPosition;
			windowPosition = windowPosition + ImGui::GetCursorPos();
			m_app.SetWindowPosition(m_subHwnd, { (i32)(windowPosition.x), (i32)windowPosition.y });
		}

		/*ImVec2 windowSize = ImGui::GetContentRegionAvail();
		if (windowSize != m_subWindowSize)
		{
			m_subWindowSize = windowSize;
			m_app.SetWindowSize(m_subHwnd, { (i32)windowSize.x, (i32)windowSize.y });
		}*/
		m_app.SetWindowSize(m_subHwnd, { (i32)0, (i32)0 });

		ImGui::EndDock();//Engine

		//for (int i = 0; i < 20; i++)
		{
			//ImGui::PushID(i * 10);
			ImGui::BeginDock("Dummy");
			//ImGui::Text("Dummy window %i", i);
			ImGui::EndDock();
			//ImGui::PopID();
		}

		ImGui::BeginDock("Dummy2");
		//ImGui::Text("Dummy window %i", i);
		ImGui::EndDock();

		m_worldsWidget.Render();

		World* world = m_engine->GetWorld(m_worldsWidget.GetSelected());
		if (nullptr != world)
			m_entitiesWidget.SetWorld(world);

		m_entitiesWidget.Render();

		ImGui::Render();
	}

	void UpdateImguiInput(float deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();
		/*if(_inputChar < 0x7f)
		{
		io.AddInputCharacter(_inputChar); // ASCII or GTFO! :(
		}*/

		io.DeltaTime = SecFromMSec(deltaTime); //msec to sec

		io.MousePos = m_inputBuffer.m_mousePos;
		io.MouseDown[0] = 0 != (m_inputBuffer.m_mouseButtons & ImGui::MB_LEFT_BIT);
		io.MouseDown[1] = 0 != (m_inputBuffer.m_mouseButtons & ImGui::MB_RIGHT_BIT);
		io.MouseDown[2] = 0 != (m_inputBuffer.m_mouseButtons & ImGui::MB_MIDDLE_BIT);
		io.MouseWheel = m_inputBuffer.m_scroll;

		io.KeyCtrl = 0 != (m_inputBuffer.m_modifierKeys & ImGui::MK_CTRL_BIT);
		io.KeyShift = 0 != (m_inputBuffer.m_modifierKeys & ImGui::MK_SHIFT_BIT);
		io.KeyAlt = 0 != (m_inputBuffer.m_modifierKeys & ImGui::MK_ALT_BIT);
		io.KeySuper = 0 != (m_inputBuffer.m_modifierKeys & ImGui::MK_SUPER_BIT);
		/* bool    */io.KeysDown[512];                  // Keyboard keys that are pressed (ideally left in the "native" order your engine has access to keyboard keys, so you can use your own defines/enums for keys).
		/* ImWchar */io.InputCharacters[16 + 1];          // List of characters input (translated by user from keypress+keyboard state). Fill using AddInputCharacter() helper.
		/* float   */io.NavInputs[ImGuiNavInput_COUNT]; // Gamepad inputs (keyboard keys will be auto-mapped and be written here by ImGui::NewFrame, all values will be cleared back to zero in ImGui::EndFrame)
	}

	void RenderImgui()
	{
		ImDrawData* drawData = ImGui::GetDrawData();

		bgfx::setViewName(m_viewId, "ImGui");
		bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

		const bgfx::Caps* caps = bgfx::getCaps();
		{
			float ortho[16];
			bx::mtxOrtho(ortho, 0.0f, (float)m_windowSize.x, (float)m_windowSize.y, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			bgfx::setViewTransform(m_viewId, nullptr, ortho);
			bgfx::setViewRect(m_viewId, 0, 0, uint16_t(m_windowSize.x), uint16_t(m_windowSize.y));/////////////////////
		}

		// Render command lists
		for (int32_t ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* drawList = drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices = (uint32_t)drawList->IdxBuffer.size();

			if (!bgfx::checkAvailTransientBuffers(numVertices, m_decl, numIndices))
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_decl);
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

					bgfx::TextureHandle th = m_texture;
					bgfx::ProgramHandle program = m_program;

					if (NULL != cmd->TextureId)
					{
						ASSERT(false);
						/*#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)
						union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
						state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
						? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
						: BGFX_STATE_NONE
						;
						th = texture.s.handle;
						if (0 != texture.s.mip)
						{
						const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
						bgfx::setUniform(u_imageLodEnabled, lodEnabled);
						program = m_imageProgram;
						}*/
					}
					else
					{
						state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
					}

					const uint16_t xx = uint16_t(max(cmd->ClipRect.x, 0.0f));
					const uint16_t yy = uint16_t(max(cmd->ClipRect.y, 0.0f));
					bgfx::setScissor(xx, yy
						, uint16_t(min(cmd->ClipRect.z, 65535.0f) - xx)
						, uint16_t(min(cmd->ClipRect.w, 65535.0f) - yy)
					);

					bgfx::setState(state);
					bgfx::setTexture(0, s_tex, th);
					bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
					bgfx::setIndexBuffer(&tib, offset, cmd->ElemCount);
					bgfx::submit(m_viewId, program);
				}

				offset += cmd->ElemCount;
			}
		}
	}


private:
	IAllocator& m_allocator;
	App& m_app;
	Engine* m_engine = nullptr;
	bool m_inputEnabled = false;

	//imgui
	ImGuiContext* m_imgui;
	HeapAllocator m_imguiAllocator;
	Input m_inputBuffer;
	Editor::WorldsWidget m_worldsWidget;
	Editor::EntitiesWidget m_entitiesWidget;
	//imgui data
	ImVec2 m_subWindowPosition = { 0.0f, 0.0f };
	ImVec2 m_subWindowSize = { 0.0f, 0.0f };

	//bgfx for imgui
	bgfx::VertexDecl m_decl;
	bgfx::ProgramHandle m_program;
	bgfx::TextureHandle m_texture;
	bgfx::UniformHandle s_tex;
	bgfx::ViewId m_viewId = 0;

	//bgfx stuff ////////////////////////////////////////////////
	BGFXAllocator m_bgfxAllocator;
	BGFXCallback m_bgfxCallback;
	//bgfx stuff for engine
	windowHandle m_subHwnd = nullptr;///////////////////////////////////
	bgfx::FrameBufferHandle m_fbh;

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
