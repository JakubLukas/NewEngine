#include "renderer_widget.h"

#include "../widget_register.h"
#include "core/allocator.h"
#include "core/engine.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"
#include "script/script.h"
#include "core/math/math.h"
#include "core/math/matrix.h"
#include "core/math/quaternion.h"
#include "core/string.h"

#include "core/input/input_system.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"

#include "core/logs.h"////////////////////////////////////////////////////


namespace Veng
{


struct CameraUpdateScript : ScriptClassBase
{
	void Init(class Engine& engine) override
	{
		m_engine = &engine;
	}

	void Deinit() override {}

	void Update(float deltaTime) override
	{
		deltaTime = 1.0f / deltaTime;

		World* world = m_engine->GetWorld(worldId(0));
		RenderScene* renderScene = static_cast<RenderScene*>(m_engine->GetSystem("renderer")->GetScene(worldId(0)));
		const RenderScene::CameraItem* camera = renderScene->GetActiveCamera();
		Transform& camTrans = world->GetEntityTransform(camera->entity);

		InputSystem* input = m_engine->GetInputSystem();
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
						angleYaw += event.axis.x * deltaTime * 0.01f;
						anglePitch += event.axis.y * deltaTime * 0.01f;

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


	Engine* m_engine;
	float angleYaw = 0.0f;
	float anglePitch = 0.0f;
	float speed = 1.0f;
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
};

REGISTER_SCRIPT(RenderWidget_CameraUpdateScript) {
	return NEW_OBJECT(allocator, CameraUpdateScript)();
}


namespace Editor
{


RendererWidget::RendererWidget(Allocator& allocator)
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


	World* world = m_engine->GetWorld(m_world);
	RenderScene* renderScene = static_cast<RenderScene*>(m_renderer->GetScene(m_world));
	m_camera = world->CreateEntity();
	Transform& camTrans = world->GetEntityTransform(m_camera);
	camTrans.position = Vector3(0, 0, 0);
	Camera cam
	{
		Camera::Type::Perspective,
		40.0f,
		40.0f,
		0.1f,
		1000.0f,
		90.0_deg,
	};
	renderScene->AddCamera(m_camera);
	renderScene->SetCameraData(m_camera, { m_camera, cam });
	renderScene->SetActiveCamera(m_camera);

	ScriptScene::ScriptData camScript;
	string::Copy(camScript.className, "RenderWidget_CameraUpdateScript");

	ScriptSystem* scriptSystem = static_cast<ScriptSystem*>(m_engine->GetSystem("script"));
	ScriptScene* scriptScene = static_cast<ScriptScene*>(scriptSystem->GetScene(m_world));
	scriptScene->AddScript(m_camera);
	scriptScene->SetScriptData(m_camera, camScript);
	scriptScene->SetScriptActive(m_camera, false);
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


void RendererWidget::Render(EventQueue& queue)
{
	if (nullptr == m_renderer)
	{
		ImGui::Text("No renderer selected");
		return;
	}

	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	if (windowSize.x <= 0) windowSize.x = 1.0f;
	if (windowSize.y <= 0) windowSize.y = 1.0f;
	if (windowSize != m_size)
	{
		m_size = windowSize;
		OnResize();
	}

	ImVec2 cursorPos = ImGui::GetCursorPos();

	m_pipeline->Render();
	ImGui::Image(m_pipeline->GetMainFrameBuffer(), windowSize);

	ImGui::SetCursorPos(cursorPos);
	if (ImGui::InvisibleButton("##raycast_trigger", windowSize) && !m_cameraControl)
	{
		ImVec2 mousePosAbs = ImGui::GetMousePos();
		ImVec2 mousePosRel = mousePosAbs - ImGui::GetWindowPos() - cursorPos;

		RenderScene* renderScene = (RenderScene*)m_renderer->GetScene(m_world);
		const RenderScene::CameraItem* camItem = renderScene->GetCameraData(m_camera);
		const Transform& cameraTransform = m_engine->GetWorld(m_world)->GetEntityTransform(m_camera);

		Ray ray = ray_from_screen_coords(cameraTransform, camItem->camera, Vector2(mousePosRel.x, mousePosRel.y));

		RayHit hit;
		if (renderScene->RaycastModels(ray, &hit))
		{
			queue.PushEvent(EventSelectEntity(m_world, hit.entity));
		}
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		m_engine->GetInputSystem()->LockCursor(true);
		m_engine->GetInputSystem()->ShowCursor(false);
		((ScriptScene*)(m_engine->GetSystem("script")->GetScene((worldId)0)))->SetScriptActive(m_camera, true);
		m_cameraControl = true;
	}
	else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
		m_engine->GetInputSystem()->LockCursor(false);
		m_engine->GetInputSystem()->ShowCursor(true);
		((ScriptScene*)(m_engine->GetSystem("script")->GetScene((worldId)0)))->SetScriptActive(m_camera, false);
		m_cameraControl = false;
	}
}


void RendererWidget::OnResize()
{
	m_renderer->Resize((i32)m_size.x, (i32)m_size.y);
	RenderScene* renderScene = static_cast<RenderScene*>(m_renderer->GetScene(m_world));
	RenderScene::CameraItem cam = *renderScene->GetCameraData(m_camera);
	cam.camera.screenWidth = m_size.x;
	cam.camera.screenHeight = m_size.y;
	cam.camera.aspect = cam.camera.screenWidth / cam.camera.screenHeight;
	renderScene->SetCameraData(m_camera, cam);
}


REGISTER_WIDGET(renderer)
{
	return NEW_OBJECT(allocator, RendererWidget)(allocator);
}


}

}