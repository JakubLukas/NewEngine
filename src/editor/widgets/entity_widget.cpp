#include "entity_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/engine.h"
#include "core/world/world.h"
#include "core/math/matrix.h"
#include "core/math/math.h"

#include "renderer/renderer.h"///////////////
#include "renderer/camera.h"///////////////////

#include "core/resource/resource_management.h"///////////
#include "core/resource/resource_manager.h"

#include "resource_widgets.h"


namespace Veng
{

class Engine;

namespace Editor
{


void EntityWidget::Init(Engine& engine, EditorInterface& editor)
{
	m_engine = &engine;
	m_editorInterface = &editor;
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
			m_entity = INVALID_ENTITY;
		}
		else if(event->type == EventType::SelectEntity)
		{
			const EventSelectEntity* eventEntity = (EventSelectEntity*)event;
			m_entity = eventEntity->entity;
		}
	}
}


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

	if (ImGui::TreeNodeEx("transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick))
	{
		Transform& entityTrans = m_world->GetEntityTransform(m_entity);
		ImGui::InputFloat3("position", &entityTrans.position.x, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::InputFloat4("rotation", &entityTrans.rotation.x, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::InputFloat("scale", &entityTrans.scale, 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::Separator();
		ImGui::TreePop();
	}

	ISystem* const* systems = m_engine->GetSystems();
	for (size_t i = 0; i < m_engine->GetSystemCount(); ++i)
	{
		ISystem* system = systems[i];
		IScene* scene = system->GetScene(m_world->GetId());

		if (scene == nullptr)
			continue;

		const ComponentInfo* componentInfos = scene->GetComponentInfos();
		for (int i = 0; i < scene->GetComponentCount(); ++i)
		{
			const ComponentInfo& info = componentInfos[i];

			if (!scene->HasComponent(info.handle, m_entity))
				continue;

			//char buffer[1024];
			//char* data = buffer;
			//ASSERT2(sizeof(buffer) >= info.dataSize, "Buffer is not large enough");
			//scene->GetComponentData(info.handle, m_entity, data);

			if (!ImGui::TreeNodeEx(info.name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick))
				continue;

			scene->EditComponent(m_editorInterface, info.handle, m_entity);

			//bool changed = false;
			/*for (const ComponentInfo::Value& value : info.values)
			{
				switch (value.type)
				{
				case ComponentInfo::ValueType::ResourceHandle:
				{
					ResourceType type = *(ResourceType*)data;
					resourceHandle* handle = (resourceHandle*)(data + sizeof(ResourceType));

					ImGui::InputScalar(value.name, ImGuiDataType_U64, handle, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);

					ResourceManager* manager = m_engine->GetResourceManager(type);
					Resource* resource = manager->GetResource(*handle);

					char pathBuffer[Path::MAX_LENGTH + 1];
					memory::Copy(pathBuffer, resource->GetPath().GetPath(), Path::MAX_LENGTH + 1);
					ImGui::InputText("path", pathBuffer, Path::MAX_LENGTH + 1);
					for (size_t i = 0; i < manager->GetSupportedFileExtCount(); ++i)
					{
						if (ImGui::BeginDragDropTarget())
						{
							const ImGuiPayload* data = ImGui::AcceptDragDropPayload(manager->GetSupportedFileExt()[i], ImGuiDragDropFlags_None);
							if (data != nullptr)
							{
								Path path((char*)data->Data);
								if (resource->GetPath() != path)
								{
									resourceHandle newResource = manager->Load(path);
									*handle = newResource;
									changed = true;
								}
							}
							ImGui::EndDragDropTarget();
						}
					}
					if (ImGui::Button("Open in editor"))
					{
						EventSelectResource event;
						event.type = type;
						event.resource = *handle;
						queue.PushEvent(event);
					}

					data = data + sizeof(ResourceType) + sizeof(resourceHandle);
					break;
				}
				case ComponentInfo::ValueType::Int:
				{
					int& val = *(int*)data;
					if (ImGui::InputInt(value.name, &val, 1, 10, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
						changed = true;
					data = data + sizeof(int);
					break;
				}
				case ComponentInfo::ValueType::Float:
				{
					float& val = *(float*)data;
					if (ImGui::InputFloat(value.name, &val, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
						changed = true;
					data = data + sizeof(float);
					break;
				}
				case ComponentInfo::ValueType::Angle:
				{
					float val = toDeg(*(float*)data);
					if (ImGui::InputFloat(value.name, &val, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
						changed = true;
					*(float*)data = toRad(val);
					data = data + sizeof(float);
					break;
				}
				case ComponentInfo::ValueType::String:
				{
					ASSERT2(false, "Not implemented yet");
				}
				case ComponentInfo::ValueType::Vector3:
				{
					Vector3& val = *(Vector3*)data;
					if (ImGui::InputFloat3(value.name, &val.x, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
						changed = true;
					data = data + sizeof(Vector3);
					break;
				}
				case ComponentInfo::ValueType::Color:
				{
					Color& val = *(Color*)data;
					if (ImGui::InputScalar(value.name, ImGuiDataType_U32, &val.abgr, NULL, NULL, NULL, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
						changed = true;
					data = data + sizeof(Color);
					break;
				}
				default:
					break;
				}
			}

			if(changed)
				scene->SetComponentData(info.handle, m_entity, buffer);*/

			ImGui::TreePop();
		}

	}
}


REGISTER_WIDGET(entity)
{
	return NEW_OBJECT(allocator, EntityWidget)();
}


}

}
