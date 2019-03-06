#include "model_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/iallocator.h"
#include "core/engine.h"
#include "core/resource/resource_management.h"
#include "core/resource/resource_manager.h"
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
	if (m_modelHandle != INVALID_RESOURCE_HANDLE)
		m_manager->UnloadResource(ResourceType::Model, m_modelHandle);

	m_modelHandle = INVALID_RESOURCE_HANDLE;
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
				m_modelHandle = eventResource->resource;
			}
		}
	}
}


void ModelWidget::RenderInternal(EventQueue& queue)
{
	char pathBuffer[Path::BUFFER_LENGTH] = {0};
	Model* model = nullptr;
	ResourceManager* modelManager = m_manager->GetManager(ResourceType::Model);

	if (m_modelHandle != INVALID_RESOURCE_HANDLE)
	{
		model = (Model*)modelManager->GetResource(m_modelHandle);
		memory::Copy(pathBuffer, model->GetPath().GetPath(), Path::BUFFER_LENGTH);
	}

	ImGui::InputText("path", pathBuffer, Path::BUFFER_LENGTH);
	for (int i = 0; i < modelManager->GetSupportedFileExtCount(); ++i)
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* data = ImGui::AcceptDragDropPayload(modelManager->GetSupportedFileExt()[i], ImGuiDragDropFlags_None);
			if (data != nullptr)
			{
				if (m_modelHandle != INVALID_RESOURCE_HANDLE)
					modelManager->Unload(m_modelHandle);
				Path modelPath((char*)data->Data);
				m_modelHandle = modelManager->Load(modelPath);
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (model != nullptr && model->GetState() == Resource::State::Ready)
	{
		ImGui::Text("meshes:");
		for (int i = 0; i < model->meshes.GetSize(); ++i)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "[%d]", i))
			{
				Mesh& mesh = model->meshes[i];
				ImGui::InputScalar("material", ImGuiDataType_U64, &mesh.material, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);

				Resource* material = m_manager->GetResource(ResourceType::Material, mesh.material);
				memory::Copy(pathBuffer, material->GetPath().GetPath(), Path::BUFFER_LENGTH);
				ImGui::InputText("path", pathBuffer, Path::BUFFER_LENGTH);
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
	}
}


REGISTER_WIDGET(material)
{
	return NEW_OBJECT(allocator, ModelWidget)(allocator);
}

}


}
