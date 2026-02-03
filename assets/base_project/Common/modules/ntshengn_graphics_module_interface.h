#pragma once
#include "ntshengn_system_module_interface.h"
#include "../resources/ntshengn_resources_graphics.h"

namespace NtshEngn {

	class GraphicsModuleInterface : public SystemModuleInterface {
	public:
		GraphicsModuleInterface() {}
		GraphicsModuleInterface(const std::string& name) : SystemModuleInterface(ModuleType::Graphics, name) {}
		virtual ~GraphicsModuleInterface() {}

		// Loads the mesh described in the mesh parameter in the internal format and returns a unique identifier
		virtual MeshID load(const Mesh& mesh) = 0;
		// Loads the image described in the image parameter in the internal format and returns a unique identifier
		virtual ImageID load(const Image& image) = 0;
		// Loads the font described in the font parameter in the internal format and returns a unique identifier
		virtual FontID load(const Font& font) = 0;

		// Sets the background color
		virtual void setBackgroundColor(const Math::vec4& backgroundColor) = 0;

		// Plays an animation for an entity, indexed in the entity's model animation list
		virtual void playAnimation(Entity entity, uint32_t animationIndex) = 0;
		// Pauses an animation played by an entity
		virtual void pauseAnimation(Entity entity) = 0;
		// Stops an animation played by an entity
		virtual void stopAnimation(Entity entity) = 0;
		// Sets the current playing time of an animation played by an entity
		virtual void setAnimationCurrentTime(Entity entity, float time) = 0;

		// Returns true if the entity is currently playing the animation with index animationIndex, else, returns false
		virtual bool isAnimationPlaying(Entity entity, uint32_t animationIndex) = 0;

		// Emits particles described by particleEmitter
		virtual void emitParticles(const ParticleEmitter& particleEmitter) = 0;
		// Destroys all particles
		virtual void destroyParticles() = 0;

		// Draws a text on the UI with the font in the fontID parameter using the position on screen, scale and color
		virtual void drawUIText(FontID fontID, const std::wstring& text, AnchorPoint anchorPoint, CoordinateType coordinateType, const Math::vec2& position, const Math::vec2& scale, const Math::vec4& color) = 0;
		// Draws a line on the UI according to its start and end points and its color
		virtual void drawUILine(CoordinateType coordinateType, const Math::vec2& start, const Math::vec2& end, const Math::vec4& color) = 0;
		// Draws a rectangle on the UI according to its position, its size (width and height) and its color
		virtual void drawUIRectangle(CoordinateType coordinateType, const Math::vec2& position, const Math::vec2& size, const Math::vec4& color) = 0;
		// Draws an image on the UI according to its sampler filter, position, rotation, scale and color to multiply the image with
		virtual void drawUIImage(ImageID imageID, ImageSamplerFilter imageSamplerFilter, AnchorPoint anchorPoint, CoordinateType coordinateType, const Math::vec2& position, float rotation, const Math::vec2& scale, const Math::vec4& color) = 0;
	}; 

}