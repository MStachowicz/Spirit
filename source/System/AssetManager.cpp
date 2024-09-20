#include "AssetManager.hpp"

#include "Geometry/Geometry.hpp"

#include "Utility/MeshBuilder.hpp"
#include "Utility/File.hpp"
#include "Utility/Config.hpp"

#include "glm/vec3.hpp"

namespace System
{
	enum class ShapeType : uint8_t { Cone, Cuboid, Cylinder, Plane, Sphere, Quad };
	static Data::Mesh make_mesh(ShapeType p_shape_type)
	{
		switch (p_shape_type)
		{
			case ShapeType::Cone:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_cone(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1.f, 16);
				return mb.get_mesh();
			}
			case ShapeType::Cuboid:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_cuboid(Geometry::Cuboid(glm::vec3(0.f)));
				return mb.get_mesh();
			}
			case ShapeType::Cylinder:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_cylinder(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1.f, 16);
				return mb.get_mesh();
			}
			case ShapeType::Sphere:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_icosphere(glm::vec3(0.f, 0.f, 0.f), 1.f, 4);
				return mb.get_mesh();
			}
			case ShapeType::Quad:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles, true>{};
				mb.add_quad(glm::vec3(-1.f, 0.f, -1.f), glm::vec3(1.f, 0.f, -1.f), glm::vec3(-1.f, 0.f, 1.f), glm::vec3(1.f, 0.f, 1.f));
				return mb.get_mesh();
			}
			default:
				throw std::runtime_error("Invalid shape type");
		}
	}

	AssetManager::AssetManager()
		: m_texture_manager{}
		, m_mesh_manager{}
		, m_available_textures{}
		, m_available_models{}
		, m_cone{m_mesh_manager.insert(make_mesh(ShapeType::Cone))}
		, m_cube{m_mesh_manager.insert(make_mesh(ShapeType::Cuboid))}
		, m_cylinder{m_mesh_manager.insert(make_mesh(ShapeType::Cylinder))}
		, m_sphere{m_mesh_manager.insert(make_mesh(ShapeType::Sphere))}
		, m_quad{m_mesh_manager.insert(make_mesh(ShapeType::Quad))}
	{
		Utility::File::foreach_file(Config::Texture_Directory, [&](auto& entry)
		{
			if (entry.is_regular_file())
				m_available_textures.push_back(entry.path());
		});
		Utility::File::foreach_file_recursive(Config::Model_Directory, [&](auto& entry)
		{
			if (entry.is_regular_file() && entry.path().has_extension() && entry.path().extension() == ".obj")
				m_available_models.push_back(entry.path());
		});
	}

	MeshRef AssetManager::insert(Data::Mesh&& p_mesh_data)
	{
		return m_mesh_manager.insert(std::move(p_mesh_data));
	}

	TextureRef AssetManager::get_texture(const std::filesystem::path& p_file_path)
	{
		return m_texture_manager.get_or_create([&p_file_path](const Data::Texture& p_texture)
		{
			return p_texture.filepath() == p_file_path;
		}, p_file_path);
	}
	TextureRef AssetManager::get_texture(const std::string_view p_file_name)
	{
		return get_texture(Config::Texture_Directory / p_file_name);
	}
} // namespace System