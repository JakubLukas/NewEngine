#include "material_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/allocator.h"
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


MaterialWidget::MaterialWidget(Allocator& allocator)
	: m_allocator(allocator)
{
}

MaterialWidget::~MaterialWidget()
{}


void MaterialWidget::Init(Engine& engine, EditorInterface& editor)
{
	m_manager = engine.GetResourceManager(Material::RESOURCE_TYPE);

	RenderSystem* renderSystem = static_cast<RenderSystem*>(engine.GetSystem("renderer"));
	m_pipeline = Pipeline::Create(m_allocator, engine, *renderSystem);
	m_pipeline->Load(Path("pipelines/material_widget.pipeline"));

	worldId wId = engine.AddWorld();
	World* world = engine.GetWorld(wId);
	RenderScene* renderScene = static_cast<RenderScene*>(renderSystem->GetScene(wId));

	Entity entity = world->CreateEntity();
	Transform& trans = world->GetEntityTransform(entity);
	renderScene->AddModel(entity);

	renderScene->SetModelData(entity, { Path("models/cube.model") });

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
	renderScene->AddCamera(camera);
	renderScene->SetCameraData(camera, { camera, cam });
	renderScene->SetActiveCamera(camera);
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
			if (eventResource->type == Material::RESOURCE_TYPE)
			{
				m_material = eventResource->resource;
			}
		}
	}
}


void MaterialWidget::Render(EventQueue& queue)
{
	if (m_material != INVALID_RESOURCE_HANDLE)
	{
		Material* material = (Material*)m_manager->GetResource(m_material);

		bool changed = false;
		char pathBuffer[Path::BUFFER_LENGTH];
		memory::Copy(pathBuffer, material->GetPath().GetPath(), Path::BUFFER_LENGTH);
		ImGui::InputText("path", pathBuffer, Path::BUFFER_LENGTH);

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
