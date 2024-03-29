#include <dxpch.h>
#include <Engine/Renderer/Model.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace dyxide
{
	std::unordered_map<std::string, Ref<Model>> s_ModelCache;

	Model::Model(std::vector<Mesh> meshes) : m_Meshes(meshes)
	{

	}

	Model::~Model()
	{

	}

	Ref<Texture2D> LoadMaterialTexture(aiMaterial* mat, aiTextureType type, std::string directory)
	{
		if (!mat->GetTextureCount(type))
		{
			return nullptr;
		}

		aiString aiStr;
		mat->GetTexture(type, 0, &aiStr);
		std::string str = std::string(aiStr.C_Str());

		for (size_t i = 0; i < str.length(); ++i) {
			if (str[i] == ' ') {
				str[i] = '_';
			}
		}

		size_t pos = str.find_last_of("/\\");
		if (pos != std::string::npos) {
			str = str.substr(pos + 1);
		}

		return Texture2D::Create(directory + "/" + str);
	}

	Mesh ProcessMesh(aiMesh* aiMesh, const aiScene* aiScene, std::string directory)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		Ref<Texture2D> diffuseTexture = nullptr;

		// Process vertex positions, normals and texture coordinates
		for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
		{
			Vertex vertex = {};

			glm::vec3 vec;
			vec.x = aiMesh->mVertices[i].x;
			vec.y = aiMesh->mVertices[i].y;
			vec.z = aiMesh->mVertices[i].z;
			vertex.Position = vec;

			vec.x = aiMesh->mNormals[i].x;
			vec.y = aiMesh->mNormals[i].y;
			vec.z = aiMesh->mNormals[i].z;
			vertex.Normal = vec;

			if (aiMesh->mTextureCoords[0]) // Does the aiMesh contain texture coordinates?
			{
				glm::vec2 vect;
				vect.x = aiMesh->mTextureCoords[0][i].x;
				vect.y = aiMesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vect;
			}
			else
			{
				vertex.TexCoords = { 0.0f, 0.0f };
			}

			vertices.push_back(vertex);
		}

		// Process indices
		for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
		{
			aiFace face = aiMesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		// Load textures
		if (aiMesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = aiScene->mMaterials[aiMesh->mMaterialIndex];
			diffuseTexture = LoadMaterialTexture(material, aiTextureType_DIFFUSE, directory);
		}

		return Mesh(vertices, indices, diffuseTexture);
	}

	std::vector<Mesh> ProcessNode(aiNode* aiNode, const aiScene* aiScene, std::string directory)
	{
		std::vector<Mesh> meshes = std::vector<Mesh>();

		// Process all the aiNode's meshes (if any)
		for (unsigned int i = 0; i < aiNode->mNumMeshes; i++)
		{
			aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
			meshes.push_back(ProcessMesh(aiMesh, aiScene, directory));
		}

		// Then do the same for each of its children
		for (unsigned int i = 0; i < aiNode->mNumChildren; i++)
		{
			std::vector<Mesh> childMeshes = ProcessNode(aiNode->mChildren[i], aiScene, directory);
			meshes.insert(meshes.end(), childMeshes.begin(), childMeshes.end());
		}

		return meshes;
	}

	Ref<Model> Model::Create(const std::string& path)
	{
		if (s_ModelCache.find(path) != s_ModelCache.end())
		{
			return s_ModelCache[path];
		}

		Assimp::Importer importer;
		// NOTE: Using aiProcess_FlipUVs causes the texture to be applied incorrectly
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);// | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			auto error = importer.GetErrorString();
			DYXIDE_ERROR("Failed to load model with Assimp\n" << error);
			return nullptr;
		}

		std::vector<Mesh> meshes = ProcessNode(scene->mRootNode, scene, path.substr(0, path.find_last_of('/')));

		auto model = CreateRef<Model>(meshes);
		s_ModelCache[path] = model;
		return model;
	}
}