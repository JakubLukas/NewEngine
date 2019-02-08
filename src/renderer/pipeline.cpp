#include "pipeline.h"

#include "core/allocators.h"
#include "core/file/path.h"
#include "core/engine.h"
#include "renderer.h"

#include "core/containers/hash_map.h"
#include "core/hashes.h"
#include "core/parsing/json.h"
#include "core/file/file.h"
#include "core/string.h"


namespace Veng
{

enum CommandType
{
	NewView,
	SetFramebuffer,
	Clear,
	RenderModels,
};

struct Command
{
	CommandType type;
	union
	{
		FramebufferHandle fbh;
		worldId world;
	};
};


class PipelineImpl : public Pipeline
{
public:
	PipelineImpl(IAllocator& allocator, Engine& engine, RenderSystem& renderer)
		: m_sourceAllocator(allocator)
		, m_allocator(allocator)
		, m_engine(engine)
		, m_renderer(renderer)
		, m_frameBuffers(m_allocator, &HashU32)
		, m_commands(m_allocator)
	{
		m_allocator.SetDebugName("Pipeline");
	}

	~PipelineImpl()
	{
	}

	void Load(const Path& path) override
	{
		const FileMode fileMode{
			FileMode::Access::Read,
			FileMode::ShareMode::ShareRead,
			FileMode::CreationDisposition::OpenExisting,
			FileMode::FlagNone
		};

		nativeFileHandle fHandle;
		ASSERT(FS::OpenFileSync(fHandle, path, fileMode));
		size_t fileSize = FS::GetFileSize(fHandle);
		u8* data = (u8*)m_allocator.Allocate(fileSize, alignof(u8));
		size_t fileSizeRead = 0;
		ASSERT(FS::ReadFileSync(fHandle, 0, data, fileSize, fileSizeRead));
		ASSERT(fileSize == fileSizeRead);
		ASSERT(FS::CloseFileSync(fHandle));

		char errorBuffer[64] = { 0 };
		JsonValue parsedJson;
		ASSERT(JsonParseError((char*)data, &m_allocator, &parsedJson, errorBuffer));
		ASSERT(JsonIsObject(&parsedJson));

		const JsonKeyValue* fbObj = JsonObjectCFind(&parsedJson, "framebuffers");
		ASSERT(fbObj != nullptr);
		ASSERT(JsonIsObject(&fbObj->value));

		size_t fbCount = JsonObjectCount(&fbObj->value);
		const JsonKeyValue* fb = JsonObjectCBegin(&fbObj->value);
		for (size_t i = 0; i < fbCount; ++i)
		{
			ASSERT(JsonIsObject(&fb->value));

			int width = 0;
			int height = 0;
			bool screenSize = false;

			const JsonKeyValue* fbWidth = JsonObjectCFind(&fb->value, "width");
			ASSERT(fbWidth != nullptr && JsonIsInt(&fbWidth->value));
			width = (int)JsonGetInt(&fbWidth->value);

			const JsonKeyValue* fbHeight = JsonObjectCFind(&fb->value, "height");
			ASSERT(fbHeight != nullptr && JsonIsInt(&fbHeight->value));
			height = (int)JsonGetInt(&fbHeight->value);

			const JsonKeyValue* fbScreenSize = JsonObjectCFind(&fb->value, "screen_size");
			if (fbScreenSize != nullptr)
			{
				ASSERT(JsonIsBool(&fbScreenSize->value));
				screenSize = JsonGetBool(&fbScreenSize->value);
			}

			FramebufferHandle fbh = m_renderer.CreateFrameBuffer(width, height, screenSize, FramebufferType_Color | FramebufferType_Depth);//TODO: cofigurable
			m_frameBuffers.Insert(crc32_string(JsonGetString(&fb->key)), fbh);

			fb++;
		}

		const JsonKeyValue* cmdArr = JsonObjectCFind(&parsedJson, "render");
		ASSERT(cmdArr != nullptr);
		ASSERT(JsonIsArray(&cmdArr->value));

		size_t cmdCount = JsonArrayCount(&cmdArr->value);
		const JsonValue* cmd = JsonArrayCBegin(&cmdArr->value);
		m_commands.Reserve(cmdCount);
		for (size_t i = 0; i < cmdCount; ++i)
		{
			ASSERT(JsonIsObject(cmd));

			ASSERT(JsonObjectCount(cmd) == 1);
			const JsonKeyValue* cmdInt = JsonObjectCBegin(cmd);
			const char* cmdIntName = JsonGetString(&cmdInt->key);
			if (string::Equal(cmdIntName, "new_view"))
			{
				Command& command = m_commands.PushBack();
				command.type = CommandType::NewView;
			}
			else if (string::Equal(cmdIntName, "set_frame_buffer"))
			{
				ASSERT(JsonIsObject(&cmdInt->value));
				ASSERT(JsonObjectCount(&cmdInt->value) == 1);
				const JsonKeyValue* fbName = JsonObjectCFind(&cmdInt->value, "name");
				ASSERT(fbName != nullptr && JsonIsString(&fbName->value));

				Command& command = m_commands.PushBack();
				command.type = CommandType::SetFramebuffer;
				u32 fbKey = crc32_string(JsonGetString(&fbName->value));
				FramebufferHandle* fbh;
				ASSERT(m_frameBuffers.Find(fbKey, fbh));
				command.fbh = *fbh;
			}
			else if (string::Equal(cmdIntName, "clear"))
			{
				Command& command = m_commands.PushBack();
				command.type = CommandType::Clear;
			}
			else if (string::Equal(cmdIntName, "render_models"))
			{
				ASSERT(JsonIsObject(&cmdInt->value));
				ASSERT(JsonObjectCount(&cmdInt->value) == 1);
				const JsonKeyValue* world = JsonObjectCFind(&cmdInt->value, "world");
				ASSERT(world != nullptr && JsonIsInt(&world->value));

				Command& command = m_commands.PushBack();
				command.type = CommandType::RenderModels;
				command.world = (worldId)JsonGetInt(&world->value);
			}
			else
			{
				ASSERT2(false, "Unrecognized command");
			}

			cmd++;
		}
		JsonDeinit(&parsedJson);

		m_allocator.Deallocate(data);
	}

	void Deinit() override
	{
		for (const HashMap<u32, FramebufferHandle>::HashNode& node : m_frameBuffers)
			m_renderer.DestroyFramebuffer(node.value);
	}

	void Render() override
	{
		for(const Command* cmd = m_commands.Begin(); cmd < m_commands.End(); cmd++)
		{
			switch(cmd->type)
			{
				case CommandType::NewView:
				{
					m_renderer.NewView();
					break;
				}
				case CommandType::SetFramebuffer:
				{
					m_renderer.SetFramebuffer(cmd->fbh);
					break;
				}
				case CommandType::Clear:
				{
					m_renderer.Clear();
					break;
				}
				case CommandType::RenderModels:
				{
					const RenderScene* scene = (RenderScene*)m_renderer.GetScene(cmd->world);
					World* world = m_engine.GetWorld(cmd->world);
					const RenderScene::ModelItem* modelItems = scene->GetModels();
					const RenderScene::CameraItem* cameraItem = scene->GetMainCamera();
					m_renderer.SetCamera(*world, cameraItem->entity);
					m_renderer.RenderModels(*world, modelItems, scene->GetModelsCount());
					break;
				}
				default:
					break;
			}
		}
	}

	void* GetMainFrameBuffer() override
	{
		FramebufferHandle* fbh;
		ASSERT(m_frameBuffers.Find(crc32_string("main"), fbh));
		return m_renderer.GetNativeFrameBufferHandle(*fbh);
	}

	IAllocator& GetSourceAllocator() { return m_sourceAllocator; }

private:
	IAllocator& m_sourceAllocator;
	ProxyAllocator m_allocator;
	Engine& m_engine;
	RenderSystem& m_renderer;

	HashMap<u32, FramebufferHandle> m_frameBuffers;
	Array<Command> m_commands;
};


Pipeline* Pipeline::Create(IAllocator& allocator, Engine& engine, RenderSystem& renderer)
{
	return NEW_OBJECT(allocator, PipelineImpl)(allocator, engine, renderer);
}

void Pipeline::Destroy(Pipeline* pipeline)
{
	PipelineImpl* pipe = static_cast<PipelineImpl*>(pipeline);
	IAllocator& allocator = pipe->GetSourceAllocator();
	DELETE_OBJECT(allocator, pipe);
}


}