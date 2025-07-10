#pragma once
#include "../resources/ntshengn_resources_audio.h"
#include "../resources/ntshengn_resources_graphics.h"
#include "../utils/ntshengn_utils_math.h"
#include <iostream>
#include <array>
#include <string>

#define NTSHENGN_ASSET_MANAGER_INFO(message) \
	do { \
		std::cout << "\33[1m\33[94mASSET MANAGER \33[34mINFO\33[39m\33[0m: " + std::string(message) << std::endl; \
	} while(0)

#define NTSHENGN_ASSET_MANAGER_WARNING(message) \
	do { \
		std::cout << "\33[1m\33[94mASSET MANAGER \33[93mWARNING\33[39m\33[0m: " + std::string(message) << std::endl; \
	} while(0)

#define NTSHENGN_ASSET_MANAGER_ERROR(message, code) \
	do { \
		std::cout << "\33[1m\33[94mASSET MANAGER \33[31mERROR\33[39m\33[0m: " + std::string(message) + " (" + std::string(#code) + ")" << std::endl; \
		exit(1); \
	} while(0)

namespace NtshEngn {

	class AssetManagerInterface {
	public:
		virtual Model* createModel(const std::string& modelName) = 0;
		virtual Model* loadModel(const std::string& filePath) = 0;

		virtual Material* createMaterial(const std::string& materialName) = 0;
		virtual Material* loadMaterial(const std::string& filePath) = 0;

		virtual Image* createImage(const std::string& imageName) = 0;
		virtual Image* loadImage(const std::string& filePath) = 0;

		virtual Font* createFont(const std::string& fontName) = 0;
		virtual Font* loadFontBitmap(const std::string& filePath, float fontHeight) = 0;
		virtual Font* loadFontSDF(const std::string& filePath) = 0;
		
		virtual Sound* createSound(const std::string& soundName) = 0;
		virtual Sound* loadSound(const std::string& filePath) = 0;

		virtual void destroyModel(const std::string& modelName) = 0;
		virtual void destroyMaterial(const std::string& materialName) = 0;
		virtual void destroyImage(const std::string& imageName) = 0;
		virtual void destroyFont(const std::string& fontName) = 0;
		virtual void destroySound(const std::string& soundName) = 0;

		virtual Model* findModelByName(const std::string& modelName) = 0;
		virtual std::string getModelName(const Model* model) = 0;

		virtual Material* findMaterialByName(const std::string& materialName) = 0;
		virtual std::string getMaterialName(const Material* material) = 0;

		virtual Image* findImageByName(const std::string& imageName) = 0;
		virtual std::string getImageName(const Image* image) = 0;

		virtual Font* findFontByName(const std::string& fontName) = 0;
		virtual std::string getFontName(const Font* font) = 0;

		virtual Sound* findSoundByName(const std::string& soundName) = 0;
		virtual std::string getSoundName(const Sound* sound) = 0;

		virtual void calculateTangents(Mesh& mesh) = 0;
		virtual std::array<Math::vec3, 2> calculateAABB(const Mesh& mesh) = 0;
	};

}