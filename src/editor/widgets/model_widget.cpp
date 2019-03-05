#include "model_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/iallocator.h"
#include "core/engine.h"
#include "core/resource/resource_management.h"
#include "renderer/resource_managers/model.h"


namespace Veng
{


namespace Editor
{


ModelWidget::ModelWidget(IAllocator& allocator)
	: m_allocator(allocator)
{
}

ModelWidget::~ModelWidget()
{}


void ModelWidget::Init(Engine& engine, EditorInterface& editor)
{
	m_manager = engine.GetResourceManagement();
}

void ModelWidget::Deinit()
{
	m_model = INVALID_RESOURCE_HANDLE;
	m_manager = nullptr;
}


void ModelWidget::Update(EventQueue& queue)
{
	for (size_t i = 0; i < queue.GetPullEventsSize(); ++i)
	{
		const Event* event = queue.PullEvents()[i];
		if (event->type == EventType::SelectResource)
		{
			const EventSelectResource* eventResource = (EventSelectResource*)event;
			if (eventResource->type == ResourceType::Model)
			{
				m_model = eventResource->resource;
			}
		}
	}
}


void ModelWidget::RenderInternal(EventQueue& queue)
{
	if (m_model != INVALID_RESOURCE_HANDLE)
	{
		Model* model = (Model*)m_manager->GetResource(ResourceType::Model, m_model);

		bool changed = false;
		char pathBuffer[Path::MAX_LENGTH + 1];
		memory::Copy(pathBuffer, model->GetPath().GetPath(), Path::MAX_LENGTH + 1);
		ImGui::InputText("path", pathBuffer, Path::MAX_LENGTH + 1);

		ImGui::Text("meshes:");
		for (int i = 0; i < model->meshes.GetSize(); ++i)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "[%d]", i))
			{
				Mesh& mesh = model->meshes[i];
				ImGui::InputScalar("material", ImGuiDataType_U64, &mesh.material, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);

				Resource* material = m_manager->GetResource(ResourceType::Material, mesh.material);
				memory::Copy(pathBuffer, material->GetPath().GetPath(), Path::MAX_LENGTH + 1);
				ImGui::InputText("path", pathBuffer, Path::MAX_LENGTH + 1);
				if (ImGui::Button("Open in editor"))
				{
					EventSelectResource event;
					event.type = ResourceType::Material;
					event.resource = mesh.material;
					queue.PushEvent(event);
				}

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		//if (changed)
	}
}


REGISTER_WIDGET(material)
{
	return NEW_OBJECT(allocator, ModelWidget)(allocator);
}

}


}
