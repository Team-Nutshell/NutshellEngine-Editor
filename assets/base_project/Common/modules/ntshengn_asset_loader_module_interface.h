#pragma once
#include "ntshengn_module_interface.h"
#include "../resources/ntshengn_resources_audio.h"
#include "../resources/ntshengn_resources_graphics.h"
#include "../utils/ntshengn_utils_math.h"
#include <string>
#include <array>

namespace NtshEngn {

	class AssetManagerInterface;

	class AssetLoaderModuleInterface : public ModuleInterface {
	public:
		AssetLoaderModuleInterface() {}
		AssetLoaderModuleInterface(const std::string& name) : ModuleInterface(ModuleType::AssetLoader, name) {}
		virtual ~AssetLoaderModuleInterface() {}
		
		// Loads the sound in file at path filePath
		virtual bool loadSound(const std::string& filePath, Sound& sound) = 0;
		// Loads the image in file at path filePath
		virtual bool loadImage(const std::string& filePath, Image& image) = 0;
		// Loads the model in file at path filePath
		virtual bool loadModel(const std::string& filePath, Model& model) = 0;
		// Loads the material in file at path filePath
		virtual bool loadMaterial(const std::string& filePath, Material& material) = 0;
		// Loads the bitmap font in file at path filePath
		virtual bool loadFontBitmap(const std::string& filePath, float fontHeight, Font& font) = 0;
		// Loads the SDF font in file at path filePath
		virtual bool loadFontSDF(const std::string& filePath, Font& font) = 0;

	public:
		void setAssetManager(AssetManagerInterface* passAssetManager) {
			assetManager = passAssetManager;
		}

	protected:
		AssetManagerInterface* assetManager = nullptr;
	};

}