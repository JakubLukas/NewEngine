#include "renderer_widget.h"

#include "../widget_register.h"
#include "core/iallocator.h"
#include "core/engine.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"
#include "core/math/math.h"
#include "core/math/matrix.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"


namespace Veng
{


namespace Editor
{


RendererWidget::RendererWidget(IAllocator& allocator)
	: m_allocator(allocator)
{

}

RendererWidget::~RendererWidget()
{}


void RendererWidget::Init(Engine& engine, EditorInterface& editor)
{
	m_engine = &engine;
	m_renderer = static_cast<RenderSystem*>(engine.GetSystem("renderer"));
	m_pipeline = Pipeline::Create(m_allocator, *m_engine, *m_renderer);
	m_pipeline->Load(Path("pipelines/main.pipeline"));

	for (size_t i = 0; i < /*m_engine->GetWorldCount()*/1; ++i)
	{
		World& world = m_engine->GetWorlds()[i];
		RenderScene* renderScene = static_cast<RenderScene*>(m_renderer->GetScene(world.GetId()));
		m_camera = world.CreateEntity();
		Transform& camTrans = world.GetEntityTransform(m_camera);
		camTrans.position = Vector3(0, 0, 0);
		Camera cam
		{
			Camera::Type::Perspective,
			40.0f,
			40.0f,
			0.1f,
			1000.0f,
			60.0_deg,
		};
		renderScene->AddComponent(RenderScene::GetComponentHandle(RenderScene::Component::Camera), m_camera);
		renderScene->SetComponentData(RenderScene::GetComponentHandle(RenderScene::Component::Camera), m_camera, &cam);
		renderScene->SetMainCamera(m_camera);
	}
}


void RendererWidget::Deinit()
{
	Pipeline::Destroy(m_pipeline);
}


void RendererWidget::Update(EventQueue& queue)
{
	for (size_t i = 0; i < queue.GetPullEventsSize(); ++i)
	{
		const Event* event = queue.PullEvents()[i];
		if (event->type == EventType::SelectCamera)
		{
			const EventSelectCamera* eventCamera = (EventSelectCamera*)event;
			m_camera = eventCamera->camera;
		}
	}
}


void RendererWidget::RenderInternal(EventQueue& queue)
{
	if (nullptr == m_renderer)
	{
		ImGui::Text("No renderer selected");
		return;
	}

	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	if (windowSize != m_size)
	{
		m_size = windowSize;
		OnResize();
		m_changedSize = true;
	}
	else
	{
		m_changedSize = false;
	}

	ImVec2 cursorPos = ImGui::GetCursorPos();

	m_pipeline->Render();
	ImGui::Image(m_pipeline->GetMainFrameBuffer(), windowSize);

	ImGui::SetCursorPos(cursorPos);
	if (ImGui::InvisibleButton("##raycast_trigger", windowSize))
	{
		ImVec2 mousePosAbs = ImGui::GetMousePos();
		ImVec2 mousePosRel = mousePosAbs - ImGui::GetWindowPos() - cursorPos;

		RenderScene* renderScene = (RenderScene*)m_renderer->GetScene((worldId)0);//TODO FIX WORLD ID ///////////////////
		const componentHandle cameraComponentHandle = renderScene->GetComponentHandle("camera"); ////////////////////////////
		Camera* cam = (Camera*)renderScene->GetComponentData(cameraComponentHandle, m_camera);
		const Transform& cameraTransform = m_engine->GetWorld((worldId)0)->GetEntityTransform(m_camera);//////////////////////////////

		Ray ray;
		ray.origin = cameraTransform.position;
		//compute correct ray
		float mx = (mousePosRel.x - cam->screenWidth * 0.5f) * (1.0f / cam->screenWidth) * 2.0f;
		float my = -(mousePosRel.y - cam->screenHeight * 0.5f) * (1.0f / cam->screenHeight) * 2.0f;
		Matrix44 camRot;
		camRot.SetRotation(cameraTransform.rotation);
		Vector4 camRight = (camRot * Vector4::AXIS_X);
		Vector4 camUp = (camRot * Vector4::AXIS_Y);
		Vector4 camDir = (camRot * Vector4::AXIS_Z);

		Vector4 screenCenter = Vector4(cameraTransform.position, 1.0f) + camDir * cam->nearPlane;
		Vector4 screenRightDir = camRight * tanf(cam->fov * 0.5f) * cam->nearPlane;
		Vector4 screenUpDir = camUp * tanf(cam->fov * 0.5f) * cam->nearPlane / cam->aspect;
		Vector4 screenPoint = screenCenter + screenRightDir * mx + screenUpDir * my;
		ray.direction = Vector3(screenPoint.x, screenPoint.y, screenPoint.z) - cameraTransform.position;
		ray.direction.Normalize();

		RenderScene::ModelItem hitModel;
		if (renderScene->RaycastModels(ray, &hitModel))
		{
			ASSERT(false);
		}

		//ImGui::SetCursorPos(mousePosRel);
		//ImGui::Button("test");
	}
}


void RendererWidget::OnResize()
{
	m_renderer->Resize((i32)m_size.x, (i32)m_size.y);
	RenderScene* renderScene = static_cast<RenderScene*>(m_renderer->GetScene(worldId(0)));///////////////////////////////////////
	const componentHandle cameraComponentHandle = renderScene->GetComponentHandle("camera");///////////////////////////////////////
	Camera* cam = (Camera*)renderScene->GetComponentData(cameraComponentHandle, m_camera);
	cam->screenWidth = m_size.x;
	cam->screenHeight = m_size.y;
	cam->aspect = cam->screenWidth / cam->screenHeight;
	//renderScene->SetComponentData(componentHandle(1), m_camera, &cam);/////////////////////////////////////
}


REGISTER_WIDGET(renderer)
{
	return NEW_OBJECT(allocator, RendererWidget)(allocator);
}


}

}