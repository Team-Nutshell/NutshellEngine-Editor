#pragma once
#include "components/camera.h"
#include "components/collidable.h"
#include "components/light.h"
#include "components/renderable.h"
#include "components/rigidbody.h"
#include "components/scriptable.h"
#include "components/sound_listener.h"
#include "components/transform.h"
#include "../../external/nml/include/nml.h"
#include <QObject>
#include <string>

typedef uint32_t EntityID;

class SignalEmitter : public QObject {
	Q_OBJECT
signals:
	void createEntitySignal(EntityID);
	void destroyEntitySignal(EntityID);
	void selectEntitySignal();
	void changeEntityNameSignal(EntityID, const std::string&);
	void changeEntityPersistenceSignal(EntityID, bool);
	void changeEntityTransformSignal(EntityID, const Transform&);
	void addEntityCameraSignal(EntityID);
	void removeEntityCameraSignal(EntityID);
	void changeEntityCameraSignal(EntityID, const Camera&);
	void addEntityLightSignal(EntityID);
	void removeEntityLightSignal(EntityID);
	void changeEntityLightSignal(EntityID, const Light&);
	void addEntityRenderableSignal(EntityID);
	void removeEntityRenderableSignal(EntityID);
	void changeEntityRenderableSignal(EntityID, const Renderable&);
	void addEntityRigidbodySignal(EntityID);
	void removeEntityRigidbodySignal(EntityID);
	void changeEntityRigidbodySignal(EntityID, const Rigidbody&);
	void addEntityCollidableSignal(EntityID);
	void removeEntityCollidableSignal(EntityID);
	void changeEntityCollidableSignal(EntityID, const Collidable&);
	void addEntitySoundListenerSignal(EntityID);
	void removeEntitySoundListenerSignal(EntityID);
	void changeEntitySoundListenerSignal(EntityID, const SoundListener&);
	void addEntityScriptableSignal(EntityID);
	void removeEntityScriptableSignal(EntityID);
	void changeEntityScriptableSignal(EntityID, const Scriptable&);

	void toggleEntityVisibilitySignal(EntityID, bool);
	void switchCameraProjectionSignal(bool);
	void resetCameraSignal();
	void orthographicCameraToAxisSignal(const nml::vec3&);

	void selectAssetSignal(const std::string&);
	void renameFileSignal(const std::string&, const std::string&);

	void startBuildAndRunSignal();
	void endBuildAndRunSignal();
	void startExportSignal();
	void endExportSignal();
};