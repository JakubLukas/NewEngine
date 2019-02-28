#include "material_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/iallocator.h"
#include "core/engine.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"
#include "core/resource/resource_manager.h"
#include "renderer/resource_managers/material.h"
#include "renderer/resource_managers/model_manager.h"
#include "core/math/matrix.h"
#include "core/math/math.h"


namespace Veng
{


namespace Editor
{


struct MaterialWidgetData
{
};


MaterialWidget::MaterialWidget(IAllocator& allocator)
	: m_allocator(allocator)
{
}

MaterialWidget::~MaterialWidget()
{}


void MaterialWidget::Init(Engine& engine)
{
	m_manager = engine.GetResourceManager(ResourceType::Material);

	RenderSystem* renderSystem = static_cast<RenderSystem*>(engine.GetSystem("renderer"));
	m_pipeline = Pipeline::Create(m_allocator, engine, *renderSystem);
	m_pipeline->Load(Path("pipelines/material_widget.pipeline"));

	char* buffer[sizeof(ResourceType) + sizeof(resourceHandle)];
	void* data = buffer;
	*(ResourceType*)data = ResourceType::Model;
	data = (ResourceType*)data + 1;

	worldId wId = engine.AddWorld();
	World* world = engine.GetWorld(wId);
	RenderScene* renderScene = static_cast<RenderScene*>(renderSystem->GetScene(wId));

	Entity entity = world->CreateEntity();
	Transform& trans = world->GetEntityTransform(entity);
	renderScene->AddComponent(RenderScene::GetComponentHandle(RenderScene::Component::Model), entity);

	resourceHandle modelHandle = renderSystem->GetModelManager().Load(Path("models/cube.model"));
	*(resourceHandle*)data = modelHandle;
	renderScene->SetComponentData(RenderScene::GetComponentHandle(RenderScene::Component::Model), entity, buffer);

	Entity camera = world->CreateEntity();
	Transform& camTrans = world->GetEntityTransform(camera);
	camTrans.position = Vector3(0, 0, 4);
	Camera cam
	{
		Camera::Type::Perspective,
		100.0f,
		100.0f,
		0.01f,
		10.0f,
		60.0_deg,
	};
	renderScene->AddComponent(RenderScene::GetComponentHandle(RenderScene::Component::Camera), camera);
	renderScene->SetComponentData(RenderScene::GetComponentHandle(RenderScene::Component::Camera), camera, &cam);
	renderScene->SetMainCamera(camera);
}

void MaterialWidget::Deinit()
{
	Pipeline::Destroy(m_pipeline);

	m_material = INVALID_RESOURCE_HANDLE;
	m_manager = nullptr;
}


void MaterialWidget::Update(EventQueue& queue)
{
	for (size_t i = 0; i < queue.GetPullEventsSize(); ++i)
	{
		const Event* event = queue.PullEvents()[i];
		if (event->type == EventType::SelectResource)
		{
			const EventSelectResource* eventResource = (EventSelectResource*)event;
			if (eventResource->type == ResourceType::Material)
			{
				m_material = eventResource->resource;
			}
		}
	}
}


void MaterialWidget::RenderInternal(EventQueue& queue)
{
	if (m_material != INVALID_RESOURCE_HANDLE)
	{
		Material* material = (Material*)m_manager->GetResource(m_material);

		bool changed = false;
		char pathBuffer[Path::MAX_LENGTH + 1];
		memory::Copy(pathBuffer, material->GetPath().GetPath(), Path::MAX_LENGTH + 1);
		ImGui::InputText("path", pathBuffer, Path::MAX_LENGTH + 1);

		ImGui::Text("textures:");
		for (int i = 0; i < Material::MAX_TEXTURES; ++i)
		{
			const char* texSlotName = material->outputTextures[i].name;

			ImGui::PushID(i);
			if (ImGui::TreeNode("", texSlotName))
			{
				ImGui::InputScalar("texture", ImGuiDataType_U64, &material->textures[i], NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::InputScalar("shader", ImGuiDataType_U64, &material->shader, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);

		//if (changed)
		m_pipeline->Render();
		ImGui::Image(m_pipeline->GetMainFrameBuffer(), ImVec2(100, 100));
	}
}


REGISTER_WIDGET(material)
{
	return NEW_OBJECT(allocator, MaterialWidget)(allocator);
}

}


}
