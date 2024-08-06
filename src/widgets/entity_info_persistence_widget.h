#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QCheckBox>

class EntityInfoPersistenceWidget : public QWidget {
	Q_OBJECT
public:
	EntityInfoPersistenceWidget(GlobalInfo& globalInfo);

private slots:
	void onEntitySelected();
	void onStateChanged(int state);
	void onChangeEntityPersistence(EntityID entityID, bool isPersistent);

private:
	GlobalInfo& m_globalInfo;

	QCheckBox* m_persistenceCheckBox;
};