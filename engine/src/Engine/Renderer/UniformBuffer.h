#pragma once

#include <Engine/Core/Common.h>

namespace dyxide
{
	class UniformBuffer
	{
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		virtual ~UniformBuffer();
		
		void SetData(const void* data, uint32_t size, uint32_t offset = 0);

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);

	private:
		uint32_t m_RendererID = 0;
	};
}
