#include "irenderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/associative_array.h"
#include "core/logs.h"
#include "core/file/path.h"

#include "core/resource/resource_management.h"
#include "shader_manager.h"
#include "material_manager.h"
#include "model_manager.h"
#include "model.h"

#include <bgfx/bgfx.h>///////////////

#include "core/math.h"
#include "core/memory.h"

namespace bx
{

/// Abstract allocator interface.
struct AllocatorI
{
	virtual ~AllocatorI() { }

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

bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName);

}


namespace Veng
{


struct BGFXAllocator : public bx::AllocatorI
{
	static const size_t NATURAL_ALIGNEMENT = 8;

	explicit BGFXAllocator(IAllocator& source)
		: m_source(source)
	{
	}

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
			LogError("Error: bgfx: DebugCheck: %s\n", _str);
			break;
		case bgfx::Fatal::InvalidShader:
			LogError("Error: bgfx: InvalidShader: %s\n", _str);
			break;
		case bgfx::Fatal::UnableToInitialize:
			LogError("Error: bgfx: UnableToInitialize: %s\n", _str);
			break;
		case bgfx::Fatal::UnableToCreateTexture:
			LogError("Error: bgfx: UnableToCreateTexture: %s\n", _str);
			break;
		case bgfx::Fatal::DeviceLost:
			LogError("Error: bgfx: DeviceLost: %s\n", _str);
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



class RenderSceneImpl : public RenderScene
{
public:
	RenderSceneImpl(IAllocator& allocator, RenderSystem& renderSystem)
		: m_allocator(allocator)
		, m_renderSystem(renderSystem)
		, m_models(m_allocator)
	{

	}

	~RenderSceneImpl() override
	{
		for (auto& modelHandle : m_models)
		{
			m_renderSystem.GetModelManager().Unload(modelHandle);
		}
	}

	void Update(float deltaTime) override
	{

	}

	void AddModelComponent(Entity entity, worldId world, const Path& path) override
	{
		modelHandle h = m_renderSystem.GetModelManager().Load(path);
		m_models.Insert(entity, h);
	}

	void RemoveModelComponent(Entity entity, worldId world) override
	{
		m_models.Erase(entity);
	}

	bool HasModelComponent(Entity entity, worldId world) const override
	{
		modelHandle* model;
		return m_models.Find(entity, model);
	}

	size_t GetModelsCount() const override
	{
		return m_models.GetSize();
	}

	modelHandle* GetModels() const override
	{
		return m_models.getValues();
	}

private:
	IAllocator& m_allocator;
	RenderSystem& m_renderSystem;
	AssociativeArray<Entity, modelHandle> m_models;
};


//----------------------------------------------------------------------------------


class RenderSystemImpl : public RenderSystem
{
public:
	RenderSystemImpl(Engine& engine)
		: m_allocator(engine.GetAllocator())
		, m_engine(engine)
		, m_bgfxAllocator(m_allocator)
	{
		///////////////
		bgfx::PlatformData d { 0 };
		d.nwh = m_engine.GetPlatformData().windowHndl;
		bgfx::setPlatformData(d);

		bgfx::init(bgfx::RendererType::Count, BGFX_PCI_ID_NONE, 0, &m_bgfxCallback, &m_bgfxAllocator);

		bgfx::setDebug(BGFX_DEBUG_NONE);//TODO

		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
		);

		ResourceManagement* resourceManagement = m_engine.GetResourceManagement();

		m_shaderInternalManager = NEW_OBJECT(m_allocator, ShaderInternalManager)(m_allocator, *m_engine.GetFileSystem(), resourceManagement);
		m_shaderManager = NEW_OBJECT(m_allocator, ShaderManager)(m_allocator, *m_engine.GetFileSystem(), resourceManagement);
		m_materialManager = NEW_OBJECT(m_allocator, MaterialManager)(m_allocator, *m_engine.GetFileSystem(), resourceManagement);
		m_modelManager = NEW_OBJECT(m_allocator, ModelManager)(m_allocator, *m_engine.GetFileSystem(), resourceManagement);

		resourceManagement->RegisterManager(ResourceType::ShaderInternal, m_shaderInternalManager);
		resourceManagement->RegisterManager(ResourceType::Shader, m_shaderManager);
		resourceManagement->RegisterManager(ResourceType::Material, m_materialManager);
		resourceManagement->RegisterManager(ResourceType::Model, m_modelManager);
	}


	~RenderSystemImpl() override
	{
		DELETE_OBJECT(m_allocator, m_scene);

		DELETE_OBJECT(m_allocator, m_modelManager);
		DELETE_OBJECT(m_allocator, m_materialManager);
		DELETE_OBJECT(m_allocator, m_shaderManager);
		DELETE_OBJECT(m_allocator, m_shaderInternalManager);

		// Shutdown bgfx.
		bgfx::shutdown();
	}


	void Init() override
	{
		m_scene = NEW_OBJECT(m_allocator, RenderSceneImpl)(m_allocator, *this);
	}


	void Update(float deltaTime) override
	{
		static float time = 0;
		time += deltaTime * 0.001f;
		static const Vector4 at = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		static const Vector4 eye = Vector4(0.0f, 0.0f, 35.0f, 1.0f);
		static const Vector4 up = Vector4(0.0f, 1.0f, 0.0f, 0.0f);

		static Matrix44 view;
		view.SetLookAt(eye, at, up);

		static Matrix44 proj;
		proj.SetPerspective(60.0_deg, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(0, &view.m11, &proj.m11);

		// Set view 0 default viewport.
		//bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

		bgfx::touch(0);//dummy draw call (clear view 0)

		// Submit 11x11 cubes.
		for (uint32_t yy = 0; yy < 11; ++yy)
		{
			for (uint32_t xx = 0; xx < 11; ++xx)
			{
				Matrix44 mtx = Matrix44::IDENTITY;
				mtx.RotateX(time + xx*0.21f);
				mtx.RotateY(time + yy*0.37f);
				mtx.m14 = -15.0f + float(xx)*3.0f;
				mtx.m24 = -15.0f + float(yy)*3.0f;
				mtx.m34 = 0.0f;
				mtx.Transpose();
				// Set model matrix for rendering.
				bgfx::setTransform(&mtx.m11);


				// Set vertex and index buffer.
				// DUMMY test
				modelHandle* handles = m_scene->GetModels();
				for(size_t i = 0; i < m_scene->GetModelsCount(); ++i)
				{
					const Model* model = m_modelManager->GetResource(handles[i]);
					if(model->GetState() == Resource::State::Ready)
					{
						for(const Mesh& mesh : model->meshes)
						{
							const Material* material = m_materialManager->GetResource(mesh.material);
							if(material->GetState() == Resource::State::Ready)
							{
								bgfx::setVertexBuffer(0, mesh.vertexBufferHandle);
								bgfx::setIndexBuffer(mesh.indexBufferHandle);

								// Set render states.
								bgfx::setState(BGFX_STATE_DEFAULT);

								// Submit primitive for rendering to view 0.
								const Shader* shader = m_shaderManager->GetResource(material->shader);
								bgfx::submit(0, shader->program.handle);
							}
						}
					}
				}


			}
		}

		bgfx::frame();//flip buffers
	};


	const char* GetName() const override { return "renderer"; }


	IScene* GetScene() const override
	{
		return m_scene;
	}


	MaterialManager& GetMaterialManager() const override { return *m_materialManager; }
	ShaderManager& GetShaderManager() const override { return *m_shaderManager; }
	ModelManager& GetModelManager() const override { return *m_modelManager; }


	void Resize(u32 width, u32 height) override
	{
		m_width = width;
		m_height = height;
		bgfx::reset(m_width, m_height, BGFX_RESET_VSYNC);
		bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height)); // Set view 0 default viewport.
	}


	Engine& GetEngine() const override { return m_engine; }

private:
	HeapAllocator m_allocator;//must be first
	Engine& m_engine;
	RenderSceneImpl* m_scene;

	ShaderInternalManager* m_shaderInternalManager;
	ShaderManager* m_shaderManager;
	MaterialManager* m_materialManager;
	ModelManager* m_modelManager;

	/////////////////////
	u32 m_width = 0;
	u32 m_height = 0;
	BGFXAllocator m_bgfxAllocator;
	BGFXCallback m_bgfxCallback;
	u32 m_bgfxResetFlags = BGFX_RESET_NONE;
	/////////////////////
};


RenderSystem* RenderSystem::Create(Engine& engine)
{
	return NEW_OBJECT(engine.GetAllocator(), RenderSystemImpl)(engine);
}

void RenderSystem::Destroy(RenderSystem* system)
{
	IAllocator& allocator = system->GetEngine().GetAllocator();
	DELETE_OBJECT(allocator, system);
}

}