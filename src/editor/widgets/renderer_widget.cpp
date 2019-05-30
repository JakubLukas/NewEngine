#include "renderer_widget.h"

#include "../widget_register.h"
#include "core/iallocator.h"
#include "core/engine.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"
#include "script/script.h"
#include "core/math/math.h"
#include "core/math/matrix.h"
#include "core/math/quaternion.h"

#include "core/input/input_system.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"

#include "core/logs.h"////////////////////////////////////////////////////


namespace Veng
{


static void ScriptCameraUpdate(void* data, Engine& engine, float deltaTime)
{
	deltaTime = 1.0f / deltaTime;

	static float angleYaw = 0.0f;
	static float anglePitch = 0.0f;
	static float speed = 1.0f;
	static bool forward = false;
	static bool backward = false;
	static bool left = false;
	static bool right = false;
	static bool up = false;
	static bool down = false;

	World* world = engine.GetWorld(worldId(0));
	RenderScene* renderScene = static_cast<RenderScene*>(engine.GetSystem("renderer")->GetScene(worldId(0)));
	const RenderScene::CameraItem* camera = renderScene->GetMainCamera();
	Transform& camTrans = world->GetEntityTransform(camera->entity);

	InputSystem* input = engine.GetInputSystem();
	const Array<InputEvent>& events = input->GetInputEventBuffer();
	for (const InputEvent event : events)
	{
		if (event.deviceCategory == InputDeviceCategory::Keyboard)
		{
			if (event.kbCode == KeyboardDevice::Button::W)
				forward = event.pressed;
			if (event.kbCode == KeyboardDevice::Button::S)
				backward = event.pressed;
			if (event.kbCode == KeyboardDevice::Button::A)
				left = event.pressed;
			if (event.kbCode == KeyboardDevice::Button::D)
				right = event.pressed;
			if (event.kbCode == KeyboardDevice::Button::Space)
				up = event.pressed;
			if (event.kbCode == KeyboardDevice::Button::ControlLeft)
				down = event.pressed;
		}
		if (event.deviceCategory == InputDeviceCategory::Mouse)
		{
			if (event.type == InputEvent::Type::AxisChanged)
			{
				if (event.maCode == MouseDevice::Axis::Movement)
				{
					angleYaw += event.axis.x * deltaTime * 0.1f;
					anglePitch += event.axis.y * deltaTime * 0.1f;

					Quaternion rotYaw(Vector3::AXIS_Y, angleYaw);
					Quaternion rotPitch(Vector3::AXIS_X, anglePitch);

					camTrans.rotation = rotYaw * rotPitch;
				}
				if (event.maCode == MouseDevice::Axis::Wheel)
					speed = Max(speed + event.axis.x * 0.1f, 0.0f);
			}
		}
	}

	if (forward)
		camTrans.position += Quaternion::Multiply(camTrans.rotation, Vector3::AXIS_Z) * speed;
	if (backward)
		camTrans.position -= Quaternion::Multiply(camTrans.rotation, Vector3::AXIS_Z) * speed;
	if (right)
		camTrans.position += Quaternion::Multiply(camTrans.rotation, Vector3::AXIS_X) * speed;
	if (left)
		camTrans.position -= Quaternion::Multiply(camTrans.rotation, Vector3::AXIS_X) * speed;
	if (up)
		camTrans.position += Vector3::AXIS_Y * speed;
	if (down)
		camTrans.position -= Vector3::AXIS_Y * speed;
}


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

		ScriptScene::ScriptClass camScript;
		camScript.updateFunction = ScriptCameraUpdate;

		ScriptSystem* scriptSystem = static_cast<ScriptSystem*>(m_engine->GetSystem("script"));
		ScriptScene* scriptScene = static_cast<ScriptScene*>(scriptSystem->GetScene(world.GetId()));
		scriptScene->AddComponent(ScriptScene::GetComponentHandle(ScriptScene::Component::Script), m_camera);
		scriptScene->SetComponentData(ScriptScene::GetComponentHandle(ScriptScene::Component::Script), m_camera, &camScript);
		scriptScene->SetScriptActive(m_camera, false);
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

	if (ImGui::IsKeyPressed((int)KeyboardDevice::Button::Escape, false))
	{
		m_focused = false;
		m_engine->GetInputSystem()->LockCursor(false);
		//m_engine->GetInputSystem()->HideCursor(false);
		((ScriptScene*)(m_engine->GetSystem("script")->GetScene((worldId)0)))->SetScriptActive(m_camera, false);
	}

	ImVec2 cursorPos = ImGui::GetCursorPos();

	m_pipeline->Render();
	ImGui::Image(m_pipeline->GetMainFrameBuffer(), windowSize);

	ImGui::SetCursorPos(cursorPos);
	if (ImGui::InvisibleButton("##raycast_trigger", windowSize))
	{
		if (!m_focused)
		{
			m_focused = true;
			m_engine->GetInputSystem()->LockCursor(true);
			//m_engine->GetInputSystem()->HideCursor(true);
			((ScriptScene*)(m_engine->GetSystem("script")->GetScene((worldId)0)))->SetScriptActive(m_camera, true);
		}
		else
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

			Vector3 camRight = Quaternion::Multiply(cameraTransform.rotation, Vector3::AXIS_X);
			Vector3 camUp = Quaternion::Multiply(cameraTransform.rotation, Vector3::AXIS_Y);
			Vector3 camDir = Quaternion::Multiply(cameraTransform.rotation, Vector3::AXIS_Z);

			Vector3 screenCenter = cameraTransform.position + camDir * cam->nearPlane;
			Vector3 screenRightDir = camRight * tanf(cam->fov * 0.5f) * cam->nearPlane;
			Vector3 screenUpDir = camUp * tanf(cam->fov * 0.5f) * cam->nearPlane / cam->aspect;
			Vector3 screenPoint = screenCenter + screenRightDir * mx + screenUpDir * my;
			ray.direction = screenPoint - cameraTransform.position;
			ray.direction.Normalize();

			RenderScene::ModelItem hitModel;
			if (renderScene->RaycastModels(ray, &hitModel))
			{
				//ASSERT(false);
				Log(LogType::Info, "Model hit\n");
				//m_renderer->AddDebugLine(Vector3(0, 0, 25), Vector3(0, 0, 45), Color(0, 255, 0), 0);
			}

			m_renderer->AddDebugLine(ray.origin, ray.origin + ray.direction * 100, Color(0, 255, 0), 0.05f, 10.0f);
			//ImGui::SetCursorPos(mousePosRel);
			//ImGui::Button("test");
		}
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