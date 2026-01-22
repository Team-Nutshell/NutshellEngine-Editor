#pragma once
#include <string>
#include <algorithm>
#include <filesystem>

struct AssetHelper {
	static std::string absoluteToRelative(std::string path, const std::string& projectDirectory) {
		if (!path.empty()) {
			std::replace(path.begin(), path.end(), '\\', '/');
			if (!projectDirectory.empty()) {
				if (std::filesystem::path(path).is_absolute()) {
					if (path.substr(0, projectDirectory.size()) == projectDirectory) {
						path = path.substr(projectDirectory.size() + 1);
					}
				}
			}
		}

		return path;
	}

	static std::string relativeToAbsolute(const std::string& path, const std::string& projectDirectory) {
		if (std::filesystem::path(path).is_absolute()) {
			return path;
		}

		return projectDirectory + "/" + path;
	}

	enum class FileType {
		Image,
		Icon,
		Font,
		Mesh,
		ImageSampler,
		Material,
		Model,
		Sound,
		Options,
		Scene,
		Json,
		Text,
		Unknown
	};

	static FileType fileType(const std::string& path) {
		size_t lastDot = path.rfind('.');
		if (lastDot != std::string::npos) {
			std::string extension = path.substr(lastDot + 1);
			if ((extension == "ntim") ||
				(extension == "jpg") ||
				(extension == "jpeg") ||
				(extension == "png") ||
				(extension == "tga") ||
				(extension == "bmp") ||
				(extension == "gif")) {
				return FileType::Image;
			}
			else if (extension == "ico") {
				return FileType::Icon;
			}
			else if (extension == "ttf") {
				return FileType::Font;
			}
			else if (extension == "ntmh") {
				return FileType::Mesh;
			}
			else if (extension == "ntsp") {
				return FileType::ImageSampler;
			}
			else if (extension == "ntml") {
				return FileType::Material;
			}
			else if ((extension == "ntmd") ||
				(extension == "gltf") ||
				(extension == "glb") ||
				(extension == "obj")) {
				return FileType::Model;
			}
			else if ((extension == "ntsd") ||
				(extension == "wav") ||
				(extension == "ogg")) {
				return FileType::Sound;
			}
			else if (extension == "ntop") {
				return FileType::Options;
			}
			else if (extension == "ntsn") {
				return FileType::Scene;
			}
			else if (extension == "json") {
				return FileType::Json;
			}
			else if (extension == "txt") {
				return FileType::Text;
			}
		}

		return FileType::Unknown;
	}
};