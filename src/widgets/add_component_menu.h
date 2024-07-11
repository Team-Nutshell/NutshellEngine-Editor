#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class AddComponentMenu : public QMenu {
	Q_OBJECT
public:
	AddComponentMenu(GlobalInfo& globalInfo);

private slots:
	void addCameraComponent();
	void addLightComponent();
	void addRenderableComponent();
	void addRigidbodyComponent();
	void addCollidableComponent();
	void addScriptableComponent();

private:
	GlobalInfo& m_globalInfo;

public:
	QAction* addCameraComponentAction;
	QAction* addLightComponentAction;
	QAction* addRenderableComponentAction;
	QAction* addRigidbodyComponentAction;
	QAction* addCollidableComponentAction;
	QAction* addScriptableComponentAction;
};