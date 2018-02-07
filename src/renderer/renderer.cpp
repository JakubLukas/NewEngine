#include "irenderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/associative_array.h"
#include "core/logs.h"

#include "core/resource/dependency_manager.h"
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



void mtxRotateXY(float* _result, float _ax, float _ay);
struct Handness
{
	enum Enum
	{
		Left,
		Right,
	};
};
void mtxProj(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc);

void mtxLookAt(float* _result, const float* _eye, const float* _at, const float* _up = NULL);

inline void mtxQuat(float* _result, const float* _quat)
{
	const float x = _quat[0];
	const float y = _quat[1];
	const float z = _quat[2];
	const float w = _quat[3];

	const float x2 = x + x;
	const float y2 = y + y;
	const float z2 = z + z;
	const float x2x = x2 * x;
	const float x2y = x2 * y;
	const float x2z = x2 * z;
	const float x2w = x2 * w;
	const float y2y = y2 * y;
	const float y2z = y2 * z;
	const float y2w = y2 * w;
	const float z2z = z2 * z;
	const float z2w = z2 * w;

	_result[0] = 1.0f - (y2y + z2z);
	_result[1] = x2y - z2w;
	_result[2] = x2z + y2w;
	_result[3] = 0.0f;

	_result[4] = x2y + z2w;
	_result[5] = 1.0f - (x2x + z2z);
	_result[6] = y2z - x2w;
	_result[7] = 0.0f;

	_result[8] = x2z - y2w;
	_result[9] = y2z + x2w;
	_result[10] = 1.0f - (x2x + y2y);
	_result[11] = 0.0f;

	_result[12] = 0.0f;
	_result[13] = 0.0f;
	_result[14] = 0.0f;
	_result[15] = 1.0f;
}

inline void mtxQuatTranslation(float* _result, const float* _quat, const float* _translation)
{
	mtxQuat(_result, _quat);
	_result[12] = -(_result[0] * _translation[0] + _result[4] * _translation[1] + _result[8] * _translation[2]);
	_result[13] = -(_result[1] * _translation[0] + _result[5] * _translation[1] + _result[9] * _translation[2]);
	_result[14] = -(_result[2] * _translation[0] + _result[6] * _translation[1] + _result[10] * _translation[2]);
}

inline void mtxQuatTranslationHMD(float* _result, const float* _quat, const float* _translation)
{
	float quat[4];
	quat[0] = -_quat[0];
	quat[1] = -_quat[1];
	quat[2] = _quat[2];
	quat[3] = _quat[3];
	mtxQuatTranslation(_result, quat, _translation);
}













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


struct BgfxCallback : public bgfx::CallbackI
{
	BgfxCallback() {}
	~BgfxCallback() {}
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



class RenderSystemImpl : public RenderSystem
{
public:
	RenderSystemImpl(Engine& engine)
		: m_engine(engine)
		, m_allocator(engine.GetAllocator())
		, m_dependencyManager(m_allocator)
		, m_bgfxAllocator(m_allocator)
		, m_models(m_allocator)
	{
		///////////////
		bgfx::PlatformData d { 0 };
		d.nwh = m_engine.GetPlatformData().windowHndl;
		bgfx::setPlatformData(d);

		bgfx::init(bgfx::RendererType::Count, BGFX_PCI_ID_NONE, 0, &m_bgfxCallback, &m_bgfxAllocator);

		bgfx::setDebug(BGFX_DEBUG_NONE);

		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
		);

		m_shaderInternalManager = NEW_OBJECT(m_allocator, ShaderInternalManager)(m_allocator, *m_engine.GetFileSystem(), &m_dependencyManager);
		m_shaderManager = NEW_OBJECT(m_allocator, ShaderManager)(m_allocator, *m_engine.GetFileSystem(), &m_dependencyManager);
		m_materialManager = NEW_OBJECT(m_allocator, MaterialManager)(m_allocator, *m_engine.GetFileSystem(), &m_dependencyManager);
		m_modelManager = NEW_OBJECT(m_allocator, ModelManager)(m_allocator, *m_engine.GetFileSystem(), &m_dependencyManager);

		m_dependencyManager.RegisterManager(ResourceType::ShaderInternal, m_shaderInternalManager);
		m_dependencyManager.RegisterManager(ResourceType::Shader, m_shaderManager);
		m_dependencyManager.RegisterManager(ResourceType::Material, m_materialManager);
		m_dependencyManager.RegisterManager(ResourceType::Model, m_modelManager);
	}


	~RenderSystemImpl() override
	{
		for(auto& modelHandle : m_models)
		{
			m_modelManager->Unload(modelHandle);
		}

		DELETE_OBJECT(m_allocator, m_modelManager);
		DELETE_OBJECT(m_allocator, m_materialManager);
		DELETE_OBJECT(m_allocator, m_shaderManager);
		DELETE_OBJECT(m_allocator, m_shaderInternalManager);

		// Shutdown bgfx.
		bgfx::shutdown();
	}


	void Update(float deltaTime) override
	{
		static float time = 0;
		time += deltaTime * 0.001f;
		float at[3] =  { 0.0f, 0.0f,   0.0f };
		float eye[3] = { 0.0f, 0.0f, -35.0f };

		const bgfx::HMD* hmd = bgfx::getHMD();
		if (NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING))
		{
			float view[16];
			bx::mtxQuatTranslationHMD(view, hmd->eye[0].rotation, eye);
			bgfx::setViewTransform(0, view, hmd->eye[0].projection, BGFX_VIEW_STEREO, hmd->eye[1].projection);

			// Set view 0 default viewport.
			//
			// Use HMD's width/height since HMD's internal frame buffer size
			// might be much larger than window size.
			bgfx::setViewRect(0, 0, 0, hmd->width, hmd->height);
		}
		else
		{
			float view[16];
			bx::mtxLookAt(view, eye, at);

			float proj[16];
			bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
			bgfx::setViewTransform(0, view, proj);

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
		}

		bgfx::touch(0);//dummy draw call (clear view 0)

		// Submit 11x11 cubes.
		for (uint32_t yy = 0; yy < 11; ++yy)
		{
			for (uint32_t xx = 0; xx < 11; ++xx)
			{
				float mtx[16];
				bx::mtxRotateXY(mtx, time + xx*0.21f, time + yy*0.37f);
				mtx[12] = -15.0f + float(xx)*3.0f;
				mtx[13] = -15.0f + float(yy)*3.0f;
				mtx[14] = 0.0f;

				// Set model matrix for rendering.
				bgfx::setTransform(mtx);


				// Set vertex and index buffer.
				// DUMMY test
				for(auto& modelHandle : m_models)
				{
					const Model* model = m_modelManager->GetResource(modelHandle);
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


	void Resize(u32 width, u32 height) override
	{
		m_width = width;
		m_height = height;
		bgfx::reset(m_width, m_height, BGFX_RESET_VSYNC);
	}


	void AddModelComponent(Entity entity, worldId world, const Path& path) override
	{
		modelHandle h = m_modelManager->Load(path);
		m_models.Insert(entity, h);
	}

	void RemoveModelComponent(Entity entity, worldId world) override
	{
		m_models.Erase(entity);
	}

	bool HasModelComponent(Entity entity, worldId world) override
	{
		modelHandle* model;
		return m_models.Find(entity, model);
	}


	Engine& GetEngine() const override { return m_engine; }

private:
	HeapAllocator m_allocator;//must be first
	Engine& m_engine;

	DependencyManager m_dependencyManager;
	ShaderInternalManager* m_shaderInternalManager;
	ShaderManager* m_shaderManager;
	MaterialManager* m_materialManager;
	ModelManager* m_modelManager;


	AssociativeArray<Entity, modelHandle> m_models;

	/////////////////////
	u32 m_width = 0;
	u32 m_height = 0;
	BGFXAllocator m_bgfxAllocator;
	BgfxCallback m_bgfxCallback;
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