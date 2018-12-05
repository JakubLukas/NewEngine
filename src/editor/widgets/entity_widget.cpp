#include "entity_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/engine.h"
#include "core/world/world.h"
#include "core/math/matrix.h"

#include "renderer/renderer.h"///////////////
#include "renderer/camera.h"///////////////////
#include "renderer/model_manager.h"///////////


namespace Veng
{

class Engine;

namespace Editor
{


void EntityWidget::Init(IAllocator& allocator, Engine& engine)
{
	m_engine = &engine;
}


void EntityWidget::Deinit()
{}


void EntityWidget::Update(EventQueue& queue)
{
	for(size_t i = 0; i < queue.GetPullEventsSize(); ++i)
	{
		const Event* event = queue.PullEvents()[i];
		if(event->type == EventType::SelectWorld)
		{
			const EventSelectWorld* eventWorld = (EventSelectWorld*)event;
			m_world = m_engine->GetWorld(eventWorld->id);
		}
		else if(event->type == EventType::SelectEntity)
		{
			const EventSelectEntity* eventEntity = (EventSelectEntity*)event;
			m_entity = eventEntity->entity;
		}
	}
}


/*void RenderValue(ComponentInfo::Value value, void* data)
{
	switch (value.type)
	{
	ComponentInfo::ValueType::ResourceHandle:
		ImGui::Text("Resource handle: %s %d", value.name);
		break;
	default:
		break;
	}
}*/


void EntityWidget::RenderInternal(EventQueue& queue)
{
	if (m_world == nullptr)
	{
		ImGui::Text("No World selected");
		return;
	}
	if (m_entity == INVALID_ENTITY)
	{
		ImGui::Text("No Entity selected");
		return;
	}

	Transform& entityTrans = m_world->GetEntityTransform(m_entity);
	ImGui::Text("Transform");
	ImGui::InputFloat3("position", &entityTrans.position.x, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::InputFloat4("rotation", &entityTrans.rotation.x, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::Separator();
	
	RenderSystem* renderer = static_cast<RenderSystem*>(m_engine->GetSystem("renderer"));
	RenderScene* renderScene = static_cast<RenderScene*>(renderer->GetScene());

	for (int i = 0; i < renderScene->GetComponentCount(); ++i)
	{
		const ComponentInfo& info = renderScene->GetComponents()[i];
	}

	if(renderScene->HasComponent(componentHandle(1), m_entity, m_world->GetId()))
	{
		void* cameraData = renderScene->GetComponentData(componentHandle(1), m_entity, m_world->GetId());
		const Camera& camera = *(Camera*)cameraData;
		ImGui::Text("Camera");
		float near = camera.nearPlane;
		ImGui::InputFloat("near plane", &near, 0.1f, 1.0f, "%.3f");
		float far = camera.farPlane;
		ImGui::InputFloat("far plane", &far, 0.1f, 1.0f, "%.3f");
		float fov = camera.fov;
		ImGui::InputFloat("fov", &fov, 0.1f, 1.0f, "%.3f");
		ImGui::Separator();
	}

	if(renderScene->HasComponent(componentHandle(0), m_entity, m_world->GetId()))
	{
		void* modelData = renderScene->GetComponentData(componentHandle(0), m_entity, m_world->GetId());
		ImGui::Text("Model");
		ModelManager& modelManager = renderer->GetModelManager();
		MaterialManager& materialManager = renderer->GetMaterialManager();
		const Model* model = modelManager.GetResource(*(modelHandle*)modelData);

		for(int i = 0; i < model->meshes.GetSize(); ++i)
		{
			const Mesh& mesh = model->meshes[i];
			ImGui::Text("Mesh %d", i);
			const Material* material = materialManager.GetResource(mesh.material);
			ImGui::Text("Material %d", i);
		}
	}
}


REGISTER_WIDGET(entity)
{
	return NEW_OBJECT(allocator, EntityWidget)();
}


}

}
