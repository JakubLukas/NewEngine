#include "entity_widget.h"

#include "../external/imgui/imgui.h"
#include "core/engine.h"
#include "core/matrix.h"

#include "renderer/renderer.h"///////////////
#include "renderer/camera.h"///////////////////
#include "renderer/model_manager.h"///////////


namespace Veng
{

class Engine;

namespace Editor
{


void EntityWidget::SetEngine(Engine* engine) { m_engine = engine; }

void EntityWidget::SetEntity(Entity entity) { m_entity = entity; }


void EntityWidget::RenderInternal()
{
	if(m_entity == INVALID_ENTITY)
		return;

	worldId world = (worldId)0;///////////////////////////////////

	Transform& entityTrans = m_engine->GetWorld(world)->GetEntityTransform(m_entity);
	ImGui::Text("Transform");
	ImGui::InputFloat3("position", &entityTrans.position.x, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::InputFloat4("rotation", &entityTrans.rotation.x, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::Separator();
	
	RenderSystem* renderer = static_cast<RenderSystem*>(m_engine->GetSystem("renderer"));
	RenderScene* renderScene = static_cast<RenderScene*>(renderer->GetScene());
	if(renderScene->HasCameraComponent(m_entity, world))
	{
		const RenderScene::CameraItem* cameraItem = renderScene->GetCameraComponent(m_entity, world);
		const Camera& camera = cameraItem->camera;
		ImGui::Text("Camera");
		float near = camera.nearPlane;
		ImGui::InputFloat("near plane", &near, 0.1f, 1.0f, "%.3f");
		float far = camera.farPlane;
		ImGui::InputFloat("far plane", &far, 0.1f, 1.0f, "%.3f");
		float fov = camera.fov;
		ImGui::InputFloat("fov", &fov, 0.1f, 1.0f, "%.3f");
		ImGui::Separator();
	}

	if(renderScene->HasModelComponent(m_entity, world))
	{
		const RenderScene::ModelItem* modelItem = renderScene->GetModelComponent(m_entity, world);
		ImGui::Text("Model");
		ModelManager& modelManager = renderer->GetModelManager();
		MaterialManager& materialManager = renderer->GetMaterialManager();
		const Model* model = modelManager.GetResource(modelItem->handle);

		for(int i = 0; i < model->meshes.GetSize(); ++i)
		{
			const Mesh& mesh = model->meshes[i];
			ImGui::Text("Mesh %d", i);
			const Material* material = materialManager.GetResource(mesh.material);
			ImGui::Text("Material %d", i);
		}
	}
}


}

}