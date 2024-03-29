#include <dxpch.h>
#include<glad/glad.h>
#include <Engine/Renderer/Texture.h>

#include <Engine/Core/Assert.h>

#include <stb_image.h>

namespace dyxide
{
	std::unordered_map<std::string, Ref<Texture2D>> s_TextureCache;

	namespace utils
	{
		static GLenum ImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB;
			case ImageFormat::RGBA8: return GL_RGBA;
			}

			DYXIDE_ASSERT(false, "ImageFormat not found");
			return 0;
		}

		static GLenum ImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB8;
			case ImageFormat::RGBA8: return GL_RGBA8;
			}

			DYXIDE_ASSERT(false, "ImageFormat not found");
			return 0;
		}
	}

	Texture2D::Texture2D(const TextureSpecification& specification)
		: m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
	{
		m_InternalFormat = utils::ImageFormatToGLInternalFormat(m_Specification.Format);
		m_DataFormat = utils::ImageFormatToGLDataFormat(m_Specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	Texture2D::Texture2D(const std::string& path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}

		if (data)
		{
			m_IsLoaded = true;

			m_Width = width;
			m_Height = height;

			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (channels == 3)
			{
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}

			m_InternalFormat = internalFormat;
			m_DataFormat = dataFormat;

			DYXIDE_ASSERT((internalFormat & dataFormat), "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			DYXIDE_ERROR("Texture could not be loaded at path: " << path);
		}
	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void Texture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		DYXIDE_ASSERT((size == m_Width * m_Height * bpp), "Data must be entire texture. Expected: " << size << " Provided: " << m_Width * m_Height * bpp);
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void Texture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification)
	{
		return CreateRef<Texture2D>(specification);
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		if (s_TextureCache.find(path) != s_TextureCache.end())
		{
			return s_TextureCache[path];
		}

		auto texture = CreateRef<Texture2D>(path);
		s_TextureCache[path] = texture;
		return texture;
	}
}
