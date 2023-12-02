#include "MeshSystem.hpp"
#include "TextureSystem.hpp"

#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Quad.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Utility/File.hpp"
#include "Utility/Logger.hpp"
#include "Utility/Config.hpp"

namespace System
{
	MeshSystem::MeshSystem(TextureSystem& p_texture_system) noexcept
		: m_texture_system{p_texture_system}
		, m_mesh_manager{}
		, m_available_model_paths{}
		, m_cone{m_mesh_manager.insert(make_mesh(Geometry::ShapeType::Cone))}
		, m_cube{m_mesh_manager.insert(make_mesh(Geometry::ShapeType::Cuboid))}
		, m_cylinder{m_mesh_manager.insert(make_mesh(Geometry::ShapeType::Cylinder))}
		, m_plane{m_mesh_manager.insert(make_mesh(Geometry::ShapeType::Plane))}
		, m_sphere{m_mesh_manager.insert(make_mesh(Geometry::ShapeType::Sphere))}
		, m_quad{m_mesh_manager.insert(make_mesh(Geometry::ShapeType::Quad))}
	{
		Utility::File::foreach_file_recursive(Config::Model_Directory,[&](const std::filesystem::directory_entry& entry)
		{
			if (entry.is_regular_file() && entry.path().has_extension() && entry.path().extension() == ".obj")
				m_available_model_paths.push_back(entry.path());
		});
	}

	Data::Mesh MeshSystem::make_mesh(Geometry::ShapeType p_shape_type)
	{
		switch (p_shape_type)
		{
			case Geometry::ShapeType::Cone:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
				mb.add_cone(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1.f, 16);
				return mb.get_mesh();
			}
			case Geometry::ShapeType::Cuboid:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
				mb.add_cuboid(glm::vec3(0.f), glm::vec3(2.f, 2.f, 2.f));
				return mb.get_mesh();
			}
			case Geometry::ShapeType::Cylinder:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
				mb.add_cylinder(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1.f, 16);
				return mb.get_mesh();
			}
			case Geometry::ShapeType::Plane:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
				mb.add_quad(glm::vec3(-1000.f, 0.f, -1000.f), glm::vec3(1000.f, 0.f, -1000.f), glm::vec3(-1000.f, 0.f, 1000.f), glm::vec3(1000.f, 0.f, 1000.f));
				return mb.get_mesh();
			}
			case Geometry::ShapeType::Sphere:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
				mb.add_icosphere(glm::vec3(0.f, 0.f, 0.f), 1.f, 4);
				return mb.get_mesh();
			}
			case Geometry::ShapeType::Quad:
			{
				auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
				mb.add_quad(glm::vec3(-1.f, 0.f, -1.f), glm::vec3(1.f, 0.f, -1.f), glm::vec3(-1.f, 0.f, 1.f), glm::vec3(1.f, 0.f, 1.f));
				return mb.get_mesh();
			}
			default:
				throw std::runtime_error("Invalid shape type");
		}
	}
} // namespace System