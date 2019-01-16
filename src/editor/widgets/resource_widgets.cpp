#include "resource_widgets.h"

#include "core/resource/resource_management.h"
#include "core/resource/resource_manager.h"
#include "renderer/resource_managers/texture.h"
#include "renderer/resource_managers/material.h"
#include "renderer/resource_managers/model.h"

#include "../external/imgui/imgui.h"


namespace Veng
{


namespace Editor
{


static bool RenderShader(resourceHandle& handle, ResourceManagement& resourceManagement)
{
	ResourceManager* manager = resourceManagement.GetManager(ResourceType::Shader);
	Shader* shader = (Shader*)manager->GetResource(handle);

	bool changed = false;
	char pathBuffer[Path::MAX_LENGTH + 1];
	memory::Copy(pathBuffer, shader->GetPath().GetPath(), Path::MAX_LENGTH + 1);
	ImGui::InputText("path", pathBuffer, Path::MAX_LENGTH + 1);
	for (size_t i = 0; i < manager->GetSupportedFileExtCount(); ++i)
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* data = ImGui::AcceptDragDropPayload(manager->GetSupportedFileExt()[i], ImGuiDragDropFlags_None);
			if (data != nullptr)
			{
				Path path((char*)data->Data);
				if (shader->GetPath() != path)
				{
					resourceHandle newResource = manager->Load(path);
					handle = newResource;
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (ImGui::TreeNode("varyings"))
	{
		for (int i = 0, c = sizeof(ShaderVaryingBits) * 8; i < c; ++i)
		{
			if (shader->varyings & 1 << i)
			{
				const char* name = GetShaderVaryingName((ShaderVaryingBits)(1 << i));
				ImGui::Text(name);
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("uniforms"))
	{
		for (int i = 0, c = sizeof(ShaderUniformBits) * 8; i < c; ++i)
		{
			if (shader->uniforms & 1 << i)
			{
				const char* name = GetShaderUniformName((ShaderUniformBits)(1 << i));
				ImGui::Text(name);
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("textures"))
	{
		for (int i = 0, c = sizeof(ShaderTextureBits) * 8; i < c; ++i)
		{
			if (shader->textures & 1 << i)
			{
				const char* name = GetShaderTextureName((ShaderTextureBits)(1 << i));
				ImGui::Text(name);
			}
		}
		ImGui::TreePop();
	}

	return changed;
}


static bool RenderTexture(resourceHandle& handle, ResourceManagement& resourceManagement)
{
	ResourceManager* manager = resourceManagement.GetManager(ResourceType::Texture);
	Texture* texture = (Texture*)manager->GetResource(handle);

	bool changed = false;
	char pathBuffer[Path::MAX_LENGTH + 1];
	memory::Copy(pathBuffer, texture->GetPath().GetPath(), Path::MAX_LENGTH + 1);
	ImGui::InputText("path", pathBuffer, Path::MAX_LENGTH + 1);
	for (size_t i = 0; i < manager->GetSupportedFileExtCount(); ++i)
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* data = ImGui::AcceptDragDropPayload(manager->GetSupportedFileExt()[i], ImGuiDragDropFlags_None);
			if (data != nullptr)
			{
				Path path((char*)data->Data);
				if (texture->GetPath() != path)
				{
					resourceHandle newResource = manager->Load(path);
					handle = newResource;
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::InputScalar("width", ImGuiDataType_U32, &texture->width, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
	ImGui::InputScalar("height", ImGuiDataType_U32, &texture->height, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
	ImGui::InputScalar("channels", ImGuiDataType_U32, &texture->channels, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);

	return changed;
}


static bool RenderMaterial(resourceHandle& handle, ResourceManagement& resourceManagement)
{
	ResourceManager* manager = resourceManagement.GetManager(ResourceType::Material);
	Material* material = (Material*)manager->GetResource(handle);

	bool changed = false;
	char pathBuffer[Path::MAX_LENGTH + 1];
	memory::Copy(pathBuffer, material->GetPath().GetPath(), Path::MAX_LENGTH + 1);
	ImGui::InputText("path", pathBuffer, Path::MAX_LENGTH + 1);
	for (size_t i = 0; i < manager->GetSupportedFileExtCount(); ++i)
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* data = ImGui::AcceptDragDropPayload(manager->GetSupportedFileExt()[i], ImGuiDragDropFlags_None);
			if (data != nullptr)
			{
				Path path((char*)data->Data);
				if (material->GetPath() != path)
				{
					resourceHandle newResource = manager->Load(path);
					handle = newResource;
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::Text("textures:");
	for (int i = 0; i < Material::MAX_TEXTURES; ++i)
	{
		const char* texSlotName = GetShaderTextureName((ShaderTextureBits)(1 << i));

		ImGui::PushID(i);
		if (ImGui::TreeNode("", texSlotName))
		{
			//if (material->textures & 1 << i)
			{
				ImGui::InputScalar("texture", ImGuiDataType_U64, &material->textureHandles[i], NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
				if(material->textureHandles[i] != INVALID_RESOURCE_HANDLE)
					RenderTexture(material->textureHandles[i], resourceManagement);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::InputScalar("shader", ImGuiDataType_U64, &material->shader, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
	RenderShader(material->shader, resourceManagement);
	ImGui::TreePop();

	return changed;
}


static bool RenderModel(resourceHandle& handle, ResourceManagement& resourceManagement)
{
	ResourceManager* manager = resourceManagement.GetManager(ResourceType::Model);
	Model* model = (Model*)manager->GetResource(handle);

	bool changed = false;
	char pathBuffer[Path::MAX_LENGTH + 1];
	memory::Copy(pathBuffer, model->GetPath().GetPath(), Path::MAX_LENGTH + 1);
	ImGui::InputText("path", pathBuffer, Path::MAX_LENGTH + 1);
	for (size_t i = 0; i < manager->GetSupportedFileExtCount(); ++i)
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* data = ImGui::AcceptDragDropPayload(manager->GetSupportedFileExt()[i], ImGuiDragDropFlags_None);
			if (data != nullptr)
			{
				Path path((char*)data->Data);
				if (model->GetPath() != path)
				{
					resourceHandle newResource = manager->Load(path);
					handle = newResource;
					changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::Text("meshes:");
	for (int i = 0; i < model->meshes.GetSize(); ++i)
	{
		ImGui::PushID(i);
		if (ImGui::TreeNode("", "[%d]", i))
		{
			Mesh& mesh = model->meshes[i];
			ImGui::InputScalar("material", ImGuiDataType_U64, &mesh.material, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
			RenderMaterial(mesh.material, resourceManagement);
		}

		ImGui::PopID();
	}

	return changed;
}


bool RenderResource(ResourceType type, resourceHandle& handle, ResourceManagement& resourceManagement)
{
	switch (type)
	{
	case Veng::ResourceType::ShaderInternal:
		return false;
	case Veng::ResourceType::Shader:
		return false;
	case Veng::ResourceType::Material:
		return RenderMaterial(handle, resourceManagement);
	case Veng::ResourceType::Model:
		return RenderModel(handle, resourceManagement);
	case Veng::ResourceType::Texture:
		return RenderTexture(handle, resourceManagement);
	default:
		ASSERT2(false, "Unrecognized type");
		return false;
	}
}


}


}