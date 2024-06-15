#pragma once
#include "../resources/ntshengn_resources_audio.h"
#include "../resources/ntshengn_resources_graphics.h"
#include "../utils/ntshengn_utils_math.h"
#include <array>
#include <string>

#if defined(NTSHENGN_DEBUG)
#define NTSHENGN_ASSET_MANAGER_INFO(message) \
	do { \
		std::cerr << "\33[1m\33[94mASSET MANAGER \33[34mINFO\33[39m\33[0m: " << message << std::endl; \
	} while(0)
#else
#define NTSHENGN_ASSET_MANAGER_INFO(message) \
	do { \
	} while(0)
#endif

#if defined(NTSHENGN_DEBUG)
#define NTSHENGN_ASSET_MANAGER_WARNING(message) \
	do { \
		std::cerr << "\33[1m\33[94mASSET MANAGER \33[93mWARNING\33[39m\33[0m: " << message << std::endl; \
	} while(0)
#else
#define NTSHENGN_ASSET_MANAGER_WARNING(message) \
	do { \
	} while(0)
#endif

#if defined(NTSHENGN_DEBUG)
#define NTSHENGN_ASSET_MANAGER_ERROR(message, code) \
	do { \
		std::cerr << "\33[1m\33[94mASSET MANAGER \33[31mERROR\33[39m\33[0m: " << message << " (" << #code << ")" << std::endl; \
		exit(1); \
	} while(0)
#else
#define NTSHENGN_ASSET_MANAGER_ERROR(message, code) \
	do { \
		exit(1); \
	} while(0)
#endif

namespace NtshEngn {

	class AssetManagerInterface {
	public:
		virtual Sound* createSound() = 0;
		virtual Sound* loadSound(const std::string& filePath) = 0;

		virtual Model* createModel() = 0;
		virtual Model* loadModel(const std::string& filePath) = 0;

		virtual Image* createImage() = 0;
		virtual Image* loadImage(const std::string& filePath) = 0;

		virtual Font* createFont() = 0;
		virtual Font* loadFont(const std::string& filePath, float fontHeight) = 0;

		virtual void destroySound(Sound* sound) = 0;
		virtual void destroyModel(Model* model) = 0;
		virtual void destroyImage(Image* image) = 0;
		virtual void destroyFont(Font* font) = 0;

		virtual void calculateTangents(Mesh& mesh) = 0;
		virtual std::array<Math::vec3, 2> calculateAABB(const Mesh& mesh) = 0;
	};

}