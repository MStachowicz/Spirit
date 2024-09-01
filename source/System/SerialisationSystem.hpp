#pragma once

#include "Utility/Logger.hpp"
#include "Utility/Config.hpp"

#include <filesystem>
#include <fstream>
#include <cstdio>

namespace System
{
	class Scene;

	class SerialisationSystem
	{
	public:
		static void serialise(const Scene& p_scene, const std::filesystem::path& p_path)
		{
			std::ofstream ostrm;

			// create directories if they don't exist
			std::filesystem::create_directories(p_path.parent_path());

			// ensure ofstream objects can throw exceptions
			ostrm.exceptions(std::ofstream::failbit | std::ofstream::badbit);

			try
			{
				ostrm.open(p_path, std::ios::binary);

				uint16_t version = Config::Save_Version;
				ostrm.write(reinterpret_cast<const char*>(&version), sizeof(uint16_t));

				ECS::Storage::serialise(ostrm, version, p_scene.m_entities);

				ostrm.close();
			}
			catch (std::ofstream::failure& e)
			{
				ASSERT_THROW(false, "File not successfully written, exception thrown: {}", e.what());
			}
		}
		static void deserialise(Scene& p_scene, const std::filesystem::path& p_path)
		{
			ASSERT_THROW(std::filesystem::exists(p_path),          "File with path {} doesnt exist", p_path.string());
			ASSERT_THROW(std::filesystem::is_regular_file(p_path), "Path is not a file");
			ASSERT_THROW(p_path.extension() == ".ss",              "File is not a scene file");

			std::ifstream istrm;
			// ensure ifstream objects can throw exceptions
			istrm.exceptions(std::ifstream::failbit | std::ifstream::badbit);

			try
			{
				istrm.open(p_path, std::ios::binary);

				uint16_t version = 0;
				istrm.read(reinterpret_cast<char*>(&version), sizeof(uint16_t));

				p_scene.m_entities = ECS::Storage::deserialise(istrm, version);

				istrm.close();
			}
			catch (std::ifstream::failure& e)
			{
				ASSERT_THROW(false, "File not successfully read, exception thrown: {}", e.what());
			}
		}
	};
} // namespace System