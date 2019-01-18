#include "material_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/iallocator.h"
#include "core/engine.h"
#include "core/resource/resource_manager.h"
#include "renderer/resource_managers/material.h"


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
}

void MaterialWidget::Deinit()
{
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
			const char* texSlotName = GetShaderTextureName((ShaderTextureBits)(1 << i));

			ImGui::PushID(i);
			if (ImGui::TreeNode("", texSlotName))
			{
				ImGui::InputScalar("texture", ImGuiDataType_U64, &material->textureHandles[i], NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::InputScalar("shader", ImGuiDataType_U64, &material->shader, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);

		//if (changed)
	}
}


REGISTER_WIDGET(material)
{
	return NEW_OBJECT(allocator, MaterialWidget)(allocator);
}

}


}
