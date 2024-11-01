#pragma once
#include "../common/global_info.h"
#include "vector3_widget.h"
#include <QWidget>

class SoundListenerComponentWidget : public QWidget {
	Q_OBJECT
public:
	SoundListenerComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const SoundListener& soundListener);
	void updateComponents(const std::vector<EntityID>& entityIDs, std::vector<SoundListener>& soundListeners);

private slots:
	void onEntitySelected();
	void onEntitySoundListenerAdded(EntityID entityID);
	void onEntitySoundListenerRemoved(EntityID entityID);
	void onEntitySoundListenerChanged(EntityID entityID, const SoundListener& soundListener);
	void onVec3Changed(const nml::vec3& value);

private:
	GlobalInfo& m_globalInfo;

public:
	Vector3Widget* forwardWidget;
	Vector3Widget* upWidget;
};