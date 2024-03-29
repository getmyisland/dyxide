#include <dxpch.h>
#include <Engine/Core/Scene.h>

#include <Engine/Core/Entity.h>
#include <Engine/Core/Components.h>
#include <Engine/Renderer/Renderer.h>
#include <Engine/Renderer/RenderCommand.h>
#include <Engine/Renderer/RendererUI.h>

#include <glm/glm.hpp>

#include <tuple>

namespace dyxide
{
	Scene::Scene() { }
	Scene::~Scene() { }

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
		for (auto srcEntity : view)
		{
			entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

			auto& srcComponent = src.get<Component>(srcEntity);
			dst.emplace_or_replace<Component>(dstEntity, srcComponent);
		}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityMap[uuid] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdate(Timestep ts)
	{
		// Update game state and variables
		OnLogicUpdate(ts);

		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			// Update Physics
		}

		// Try finding the main camera
		CameraComponent* primaryCamera = nullptr;
		TransformComponent* primaryCameraTransform = nullptr;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					primaryCamera = &camera;
					primaryCameraTransform = &transform;
					break;
				}
			}
		}

		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });
		RenderCommand::Clear();

		if (primaryCamera && primaryCameraTransform)
		{
			Renderer::BeginScene(*primaryCamera, *primaryCameraTransform);

			// Draw models
			{
				auto group = m_Registry.group<TransformComponent>(entt::get<ModelRendererComponent>);
				for (auto entity : group)
				{
					auto [transform, model] = group.get<TransformComponent, ModelRendererComponent>(entity);

					if (!transform.IsActive) continue;

					Renderer::DrawModel(transform.GetTransform(), model);
				}
			}

			Renderer::EndScene();
		}
		else
		{
			DYXIDE_WARN("No Primary Camera found");
		}

		RendererUI::BeginScene();

		// Draw sprites
		{
			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

				if (!transform.IsActive) continue;

				// NOTE: Sprites are manually upscaled by 100 or else it would be almost invisible at the default scale.
				RendererUI::DrawSprite(transform.GetTransform() * glm::scale(glm::mat4(1.0f), glm::vec3{ 100.0f }), sprite);
			}
		}

		// Draw text
		{
			auto view = m_Registry.view<TransformComponent, TextComponent>();
			for (auto entity : view)
			{
				auto [transform, text] = view.get<TransformComponent, TextComponent>(entity);

				if (!transform.IsActive) continue;

				// NOTE: Text is manually upscaled by 100 or else it would be almost invisible at the default scale.
				RendererUI::DrawString(text.TextString, transform.GetTransform() * glm::scale(glm::mat4(1.0f), glm::vec3{ 100.0f }), text);
			}
		}

		RendererUI::EndScene();
	}

	void Scene::OnWindowResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		OnViewportResize(width, height);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	void Scene::Step(int frames)
	{
		m_StepFrames = frames;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap.at(uuid), this };

		return {};
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<ModelRendererComponent>(Entity entity, ModelRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TextComponent>(Entity entity, TextComponent& component)
	{
	}
}
