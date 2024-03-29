#pragma once

#include <Engine/Core/KeyCodes.h>
#include <Engine/Core/MouseCodes.h>

#include <glm/glm.hpp>

namespace dyxide
{
	class Input
	{
	public:
		static void Init();

		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMouseOffset();
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static void ShowCursor();
		static void HideCursor();
		static void UnlockCursor();
		static void LockCursor();
	};
}
