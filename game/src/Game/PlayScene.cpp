#include <Game/PlayScene.h>

#include <Engine/Core/Application.h>
#include <Engine/Core/Entity.h>
#include <Engine/Core/Input.h>
#include <Engine/Core/ResourceLoader.h>

#include <Game/PlayerController.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <sstream>

namespace dyxide
{
	Entity g_Player;
	PlayerController g_PlayerController;

	Entity g_Humanoid;
	Entity g_Debug;

	void PlayScene::OnLoad()
	{
		Input::LockCursor();

		g_Player = CreateEntity("Camera");
		g_Player.AddComponent<CameraComponent>().Primary = true;

		g_Humanoid = CreateEntity("Humanoid");
		g_Humanoid.AddComponent<ModelRendererComponent>().ModelAsset = ResourceLoader::LoadModel("/models/Humanoid.fbx");
		auto& transform = g_Humanoid.GetComponent<TransformComponent>();
		transform.Translation = { 5.0f, 0.0f, 0.0f };

		g_Debug = CreateEntity("Debug");
		g_Debug.AddComponent<TextComponent>("");
		g_Debug.GetComponent<TransformComponent>().Scale = glm::vec3{ 0.2f };

		auto& window = ::dyxide::Application::Get().GetWindow();
		OnViewportResize(window.GetWidth(), window.GetHeight());
	}

	void PlayScene::OnUnload()
	{
	}

	void PlayScene::OnEvent(Event& e)
	{
	}

	void PlayScene::OnLogicUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(Key::Escape))
		{
			Application::Get().Close();
			return;
		}

		auto& transform = g_Player.GetComponent<TransformComponent>();
		g_PlayerController.CalculateMovement(ts, transform);

		std::stringstream oss;
		oss << "position: " << glm::to_string(transform.Translation) << "\n";
		oss << "rotation: " << glm::to_string(transform.Rotation);

		g_Debug.GetComponent<TextComponent>().TextString = oss.str();
	}

	void PlayScene::OnViewportResize(uint32_t width, uint32_t height)
	{
		g_Debug.GetComponent<TransformComponent>().Translation = { glm::vec3(10, height - 20, 0) };
	}
}