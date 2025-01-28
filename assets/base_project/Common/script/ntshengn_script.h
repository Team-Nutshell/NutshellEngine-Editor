#pragma once
#include "../resources/ntshengn_resources_scripting.h"
#include "../modules/ntshengn_graphics_module_interface.h"
#include "../modules/ntshengn_physics_module_interface.h"
#include "../modules/ntshengn_window_module_interface.h"
#include "../modules/ntshengn_audio_module_interface.h"
#include "../ecs/ntshengn_ecs_interface.h"
#include "../asset_manager/ntshengn_asset_manager_interface.h"
#include "../job_system/ntshengn_job_system_interface.h"
#include "../profiler/ntshengn_profiler_interface.h"
#include "../frame_limiter/ntshengn_frame_limiter_interface.h"
#include "../networking/ntshengn_networking_interface.h"
#include "../scene_manager/ntshengn_scene_manager_interface.h"
#include "../script/ntshengn_script_manager_interface.h"

namespace NtshEngn {
	
	struct Script : public ScriptBase {
	public:
		virtual void init() = 0;
		virtual void update(float dt) = 0;
		virtual void destroy() = 0;

	protected:
		// Resources
		Model* loadModel(const std::string& filePath) {
			return assetManager->loadModel(filePath);
		}
		
		MeshID getMeshID(const Mesh& mesh) {
			if (!graphicsModule) {
				return NTSHENGN_MESH_UNKNOWN;
			}

			return graphicsModule->load(mesh);
		}

		Material* loadMaterial(const std::string& filePath) {
			return assetManager->loadMaterial(filePath);
		}

		Image* loadImage(const std::string& filePath) {
			return assetManager->loadImage(filePath);
		}

		ImageID getImageID(const Image& image) {
			if (!graphicsModule) {
				return NTSHENGN_IMAGE_UNKNOWN;
			}

			return graphicsModule->load(image);
		}

		Font* loadFont(const std::string& filePath, float fontHeight) {
			return assetManager->loadFont(filePath, fontHeight);
		}

		FontID getFontID(const Font& font) {
			if (!graphicsModule) {
				return NTSHENGN_FONT_UNKNOWN;
			}

			return graphicsModule->load(font);
		}

		Sound* loadSound(const std::string& filePath) {
			return assetManager->loadSound(filePath);
		}

		SoundID getSoundID(const Sound& sound) {
			if (!audioModule) {
				return NTSHENGN_SOUND_UNKNOWN;
			}

			return audioModule->load(sound);
		}

		// Scenes
		void goToScene(const std::string& filePath) {
			sceneManager->goToScene(filePath);
		}

		std::string getCurrentScenePath() {
			return sceneManager->getCurrentScenePath();
		}

		// ECS
		Entity createEntity(const std::string& name = "") {
			if (name != "") {
				return ecs->createEntity(name);
			}

			return ecs->createEntity();
		}

		void destroyEntity(Entity entity) {
			ecs->destroyEntity(entity);
		}

		void destroyAllEntities() {
			ecs->destroyAllEntities();
		}

		void destroyNonPersistentEntities() {
			ecs->destroyNonPersistentEntities();
		}

		bool entityExists(Entity entity) {
			return ecs->entityExists(entity);
		}

		void setEntityName(Entity entity, const std::string& name) {
			ecs->setEntityName(entity, name);
		}

		bool entityHasName(Entity entity) {
			return ecs->entityHasName(entity);
		}

		std::string getEntityName(Entity entity) {
			return ecs->getEntityName(entity);
		}

		Entity findEntityByName(const std::string& name) {
			return ecs->findEntityByName(name);
		}

		void setEntityPersistence(Entity entity, bool persistent) {
			return ecs->setEntityPersistence(entity, persistent);
		}

		bool isEntityPersistent(Entity entity) {
			return ecs->isEntityPersistent(entity);
		}

		template <typename T>
		void addEntityComponent(Entity entity, T component) {
			ecs->addComponent(entity, component);
		}
		template <typename T>
		void removeEntityComponent(Entity entity) {
			ecs->removeComponent<T>(entity);
		}
		template <typename T>
		bool hasEntityComponent(Entity entity) {
			return ecs->hasComponent<T>(entity);
		}
		template <typename T>
		T& getEntityComponent(Entity entity) {
			return ecs->getComponent<T>(entity);
		}

		// Input
		InputState getKeyState(InputKeyboardKey key, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return NtshEngn::InputState::None;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getKeyState(windowID, key);
		}

		InputState getMouseButtonState(InputMouseButton mouseButton, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return NtshEngn::InputState::None;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getMouseButtonState(windowID, mouseButton);
		}

		void setCursorPosition(int x, int y, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setCursorPosition(windowID, x, y);
		}

		int getCursorPositionX(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return 0;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getCursorPositionX(windowID);
		}

		int getCursorPositionY(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return 0;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getCursorPositionY(windowID);
		}

		float getMouseScrollOffsetX(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return 0.0f;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getMouseScrollOffsetX(windowID);
		}

		float getMouseScrollOffsetY(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return 0.0f;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getMouseScrollOffsetY(windowID);
		}

		void setCursorVisibility(bool visible, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setCursorVisibility(windowID, visible);
		}

		bool isCursorVisible(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return false;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->isCursorVisible(windowID);
		}

		std::vector<GamepadID> getConnectedGamepads() {
			if (!windowModule) {
				return {};
			}

			return windowModule->getConnectedGamepads();
		}

		InputState getGamepadButtonState(GamepadID gamepadID, InputGamepadButton button) {
			if (!windowModule) {
				return NtshEngn::InputState::None;
			}

			return windowModule->getGamepadButtonState(gamepadID, button);
		}

		float getGamepadStickAxisX(GamepadID gamepadID, InputGamepadStick stick) {
			if (!windowModule) {
				return 0.0f;
			}

			return windowModule->getGamepadStickAxisX(gamepadID, stick);
		}

		float getGamepadStickAxisY(GamepadID gamepadID, InputGamepadStick stick) {
			if (!windowModule) {
				return 0.0f;
			}

			return windowModule->getGamepadStickAxisY(gamepadID, stick);
		}

		float getGamepadLeftTrigger(GamepadID gamepadID) {
			if (!windowModule) {
				return 0.0f;
			}

			return windowModule->getGamepadLeftTrigger(gamepadID);
		}

		float getGamepadRightTrigger(GamepadID gamepadID) {
			if (!windowModule) {
				return 0.0f;
			}

			return windowModule->getGamepadRightTrigger(gamepadID);
		}

		std::string getGamepadName(GamepadID gamepadID) {
			if (!windowModule) {
				return "";
			}

			return windowModule->getGamepadName(gamepadID);
		}

		// Windows
		WindowID openWindow(int width, int height, const std::string& title) {
			if (!windowModule) {
				return NTSHENGN_WINDOW_UNKNOWN;
			}

			return windowModule->openWindow(width, height, title);
		}

		bool isWindowOpen(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return false;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->isWindowOpen(windowID);
		}

		void closeWindow(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->closeWindow(windowID);
		}

		WindowID getMainWindowID() {
			if (!windowModule) {
				return NTSHENGN_WINDOW_UNKNOWN;
			}

			return windowModule->getMainWindowID();
		}

		size_t windowCount() {
			if (!windowModule) {
				return 0;
			}

			return windowModule->windowCount();
		}

		void setWindowSize(int width, int height, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setWindowSize(windowID, width, height);
		}

		int getWindowWidth(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return 0;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getWindowWidth(windowID);
		}

		int getWindowHeight(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return 0;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getWindowHeight(windowID);
		}
		
		void setWindowPosition(int x, int y, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setWindowPosition(windowID, x, y);
		}

		int getWindowPositionX(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return 0;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getWindowPositionX(windowID);
		}

		int getWindowPositionY(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return 0;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getWindowPositionY(windowID);
		}

		void setWindowFullscreen(bool fullscreen, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setWindowFullscreen(windowID, fullscreen);
		}

		bool isWindowFullscreen(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return false;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->isWindowFullscreen(windowID);
		}

		void setWindowBorderless(bool borderless, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setWindowBorderless(windowID, borderless);
		}

		bool isWindowBorderless(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return false;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->isWindowBorderless(windowID);
		}

		void setWindowResizable(bool resizable, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setWindowResizable(windowID, resizable);
		}

		bool isWindowResizable(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return false;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->isWindowResizable(windowID);
		}

		void setWindowOpacity(float opacity, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setWindowOpacity(windowID, opacity);
		}

		float getWindowOpacity(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return false;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getWindowOpacity(windowID);
		}

		std::vector<std::string> getWindowDroppedFiles(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return std::vector<std::string>();
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getWindowDroppedFiles(windowID);
		}

		void setWindowTitle(const std::string& title, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setWindowTitle(windowID, title);
		}

		std::string getWindowTitle(WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return "";
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			return windowModule->getWindowTitle(windowID);
		}

		void setWindowIcon(const Image& image, WindowID windowID = NTSHENGN_WINDOW_UNKNOWN) {
			if (!windowModule) {
				return;
			}

			if (windowID == NTSHENGN_WINDOW_UNKNOWN) {
				windowID = windowModule->getMainWindowID();
			}

			windowModule->setWindowIcon(windowID, image);
		}

		int getMonitorWidth() {
			if (!windowModule) {
				return 0 ;
			}

			return windowModule->getMonitorWidth();
		}

		int getMonitorHeight() {
			if (!windowModule) {
				return 0;
			}

			return windowModule->getMonitorHeight();
		}

		int getMonitorRefreshRate() {
			if (!windowModule) {
				return 0;
			}

			return windowModule->getMonitorRefreshRate();
		}

		float getMonitorDisplayScaling() {
			if (!windowModule) {
				return 0.0f;
			}

			return windowModule->getMonitorDisplayScaling();
		}

		// Graphics
		void setBackgroundColor(const Math::vec4& backgroundColor) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->setBackgroundColor(backgroundColor);
		}

		void emitParticles(const ParticleEmitter& particleEmitter) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->emitParticles(particleEmitter);
		}

		// Physics
		IntersectionInformation intersect(const ColliderShape* shape1, const ColliderShape* shape2) {
			if (!physicsModule) {
				return IntersectionInformation();
			}

			return physicsModule->intersect(shape1, shape2);
		}
		
		RaycastInformation raycast(const Math::vec3& rayOrigin, const Math::vec3& rayDirection, const ColliderShape* shape, float tMin = 0.0001f, float tMax = 1000000.0f) {
			if (!physicsModule) {
				return RaycastInformation();
			}

			return physicsModule->raycast(rayOrigin, rayDirection, tMin, tMax, shape);
		}

		std::vector<std::pair<Entity, RaycastInformation>> raycastAll(const Math::vec3& rayOrigin, const Math::vec3& rayDirection, float tMin = 0.0001f, float tMax = 1000000.0f) {
			if (!physicsModule) {
				return {};
			}

			return physicsModule->raycastAll(rayOrigin, rayDirection, tMin, tMax);
		}

		void setConstantForces(const Math::vec3& constantForces) {
			if (!physicsModule) {
				return;
			}

			physicsModule->setConstantForces(constantForces);
		}

		Math::vec3 getConstantForces() {
			if (!physicsModule) {
				return Math::vec3(0.0f, 0.0f, 0.0f);
			}

			return physicsModule->getConstantForces();
		}

		// Audio
		SoundSourceID playSound(SoundID soundID, float gain = 1.0f, float pitch = 1.0f) {
			if (!audioModule) {
				return NTSHENGN_SOUND_SOURCE_UNKNOWN;
			}

			return audioModule->playSound(soundID, gain, pitch);
		}

		SoundSourceID playSoundAtPosition(SoundID soundID, const Math::vec3& position, float gain = 1.0f, float pitch = 1.0f) {
			if (!audioModule) {
				return NTSHENGN_SOUND_SOURCE_UNKNOWN;
			}

			return audioModule->playSoundAtPosition(soundID, position, gain, pitch);
		}

		void resumeSoundSource(SoundSourceID soundSourceID) {
			if (!audioModule) {
				return;
			}

			audioModule->resumeSoundSource(soundSourceID);
		}

		void pauseSoundSource(SoundSourceID soundSourceID) {
			if (!audioModule) {
				return;
			}

			audioModule->pauseSoundSource(soundSourceID);
		}

		void stopSoundSource(SoundSourceID soundSourceID) {
			if (!audioModule) {
				return;
			}

			audioModule->stopSoundSource(soundSourceID);
		}

		SoundSourceState getSoundSourceState(SoundSourceID soundSourceID) {
			if (!audioModule) {
				return SoundSourceState::Stopped;
			}

			return audioModule->getSoundSourceState(soundSourceID);
		}

		bool isSoundPlaying(SoundID soundID) {
			if (!audioModule) {
				return false;
			}

			return audioModule->isSoundPlaying(soundID);
		}

		void setSoundSourcePosition(SoundSourceID soundSourceID, const Math::vec3& newPosition) {
			if (!audioModule) {
				return;
			}

			audioModule->setSoundSourcePosition(soundSourceID, newPosition);
		}

		Math::vec3 getSoundSourcePosition(SoundSourceID soundSourceID) {
			if (!audioModule) {
				return Math::vec3(0.0f, 0.0f, 0.0f);
			}

			return audioModule->getSoundSourcePosition(soundSourceID);
		}

		void setSoundSourceGain(SoundSourceID soundSourceID, float newGain) {
			if (!audioModule) {
				return;
			}

			audioModule->setSoundSourceGain(soundSourceID, newGain);
		}

		float getSoundSourceGain(SoundSourceID soundSourceID) {
			if (!audioModule) {
				return 0.0f;
			}

			return audioModule->getSoundSourceGain(soundSourceID);
		}

		void setSoundSourcePitch(SoundSourceID soundSourceID, float newPitch) {
			if (!audioModule) {
				return;
			}

			audioModule->setSoundSourcePitch(soundSourceID, newPitch);
		}

		float getSoundSourcePitch(SoundSourceID soundSourceID) {
			if (!audioModule) {
				return 0.0f;
			}

			return audioModule->getSoundSourcePitch(soundSourceID);
		}

		// Animation
		void playAnimation(Entity entity, uint32_t animationIndex) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->playAnimation(entity, animationIndex);
		}

		void pauseAnimation(Entity entity) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->pauseAnimation(entity);
		}

		void stopAnimation(Entity entity) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->stopAnimation(entity);
		}

		bool isAnimationPlaying(Entity entity, uint32_t animationIndex) {
			if (!graphicsModule) {
				return false;
			}

			return graphicsModule->isAnimationPlaying(entity, animationIndex);
		}

		// UI
		void drawUIText(FontID fontID, const std::wstring& text, const Math::vec2& position, const Math::vec4& color = Math::vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->drawUIText(fontID, text, position, color);
		}

		void drawUIText(FontID fontID, const std::string& text, const Math::vec2& position, const Math::vec4& color = Math::vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
			if (!graphicsModule) {
				return;
			}

			std::wstring wideString(text.begin(), text.end());

			graphicsModule->drawUIText(fontID, wideString, position, color);
		}

		void drawUILine(const Math::vec2& start, const Math::vec2& end, const Math::vec4& color = Math::vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->drawUILine(start, end, color);
		}

		void drawUIRectangle(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color = Math::vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->drawUIRectangle(position, size, color);
		}

		void drawUIImage(ImageID imageID, ImageSamplerFilter imageSamplerFilter, const Math::vec2& position, float rotation = 0.0f, const Math::vec2& scale = Math::vec2(1.0f, 1.0f), const Math::vec4& color = Math::vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
			if (!graphicsModule) {
				return;
			}

			graphicsModule->drawUIImage(imageID, imageSamplerFilter, position, rotation, scale, color);
		}

		// Frame Limiter
		void setMaxFPS(uint32_t maxFPS) {
			frameLimiter->setMaxFPS(maxFPS);
		}

		uint32_t getMaxFPS() {
			return frameLimiter->getMaxFPS();
		}

		// Multithreading
		void executeJob(const std::function<void()>& job) {
			jobSystem->execute(job);
		}

		void dispatchJob(uint32_t jobCount, uint32_t jobsPerWorker, const std::function<void(JobDispatchArguments)>& job) {
			jobSystem->dispatch(jobCount, jobsPerWorker, job);
		}

		void waitAllThreads() {
			jobSystem->wait();
		}

		uint32_t getNumThreads() {
			return jobSystem->getNumThreads();
		}

		// Networking
		ServerSocketInterface* createServerSocket(uint16_t port, NetworkType networkType = NetworkType::UDP) {
			return networking->createServerSocket(port, networkType);
		}

		ClientSocketInterface* createClientSocket(NetworkType networkType = NetworkType::UDP) {
			return networking->createClientSocket(networkType);
		}

		void closeServerSocket(ServerSocketInterface* serverSocket) {
			networking->closeServerSocket(serverSocket);
		}

		void closeClientSocket(ClientSocketInterface* clientSocket) {
			networking->closeClientSocket(clientSocket);
		}

		// Scripting
		template <typename T>
		T* createScript() {
			return scriptManager->createScript<T>();
		}

		// Profiling
		void startProfiling(const std::string& profilingSessionName) {
			profiler->start(profilingSessionName);
		}

		ProfilerResultNode getProfilingResults() {
			return profiler->getResults();
		}

		ProfilerResultNode endProfiling() {
			return profiler->end();
		}

		bool isProfilerRunning() {
			return profiler->isRunning();
		}

		void startProfilingBlock(const std::string& profilingBlockName) {
			profiler->startBlock(profilingBlockName);
		}

		void endProfilingBlock() {
			profiler->endBlock();
		}
	
	public:
		void setEntityID(Entity passEntityID) { entityID = passEntityID; }
		void setSystemModules(GraphicsModuleInterface* passGraphicsModule, PhysicsModuleInterface* passPhysicsModule, WindowModuleInterface* passWindowModule, AudioModuleInterface* passAudioModule) {
			graphicsModule = passGraphicsModule;
			physicsModule = passPhysicsModule;
			windowModule = passWindowModule;
			audioModule = passAudioModule;
		}
		void setScriptManager(ScriptManagerInterface* passScriptManager) { scriptManager = passScriptManager; }
		void setECS(ECSInterface* passECS) { ecs = passECS; }
		void setAssetManager(AssetManagerInterface* passAssetManager) { assetManager = passAssetManager; }
		void setFrameLimiter(FrameLimiterInterface* passFrameLimiter) { frameLimiter = passFrameLimiter; }
		void setJobSystem(JobSystemInterface* passJobSystem) { jobSystem = passJobSystem; }
		void setProfiler(ProfilerInterface* passProfiler) { profiler = passProfiler; }
		void setNetworking(NetworkingInterface* passNetworking) { networking = passNetworking; }
		void setSceneManager(SceneManagerInterface* passSceneManager) { sceneManager = passSceneManager; }

	protected:
		Entity entityID = NTSHENGN_ENTITY_UNKNOWN;

		GraphicsModuleInterface* graphicsModule = nullptr;
		PhysicsModuleInterface* physicsModule = nullptr;
		WindowModuleInterface* windowModule = nullptr;
		AudioModuleInterface* audioModule = nullptr;

		ScriptManagerInterface* scriptManager = nullptr;

		ECSInterface* ecs = nullptr;

		AssetManagerInterface* assetManager = nullptr;

		FrameLimiterInterface* frameLimiter = nullptr;

		JobSystemInterface* jobSystem = nullptr;

		ProfilerInterface* profiler = nullptr;

		NetworkingInterface* networking = nullptr;

		SceneManagerInterface* sceneManager = nullptr;
	};

}