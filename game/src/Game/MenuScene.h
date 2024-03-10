#pragma once

#include <Engine/Core/Common.h>
#include <Engine/Core/Scene.h>

namespace dyxide
{
	class MenuScene : public Scene
	{
	public:
		virtual void OnLoad() override;
		virtual void OnUnload() override;
		virtual void OnEvent(Event& e) override;

	protected:
		virtual void OnUpdateEntities(Timestep ts) override;
	};
}