#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QCheckBox>
#include <memory>

class EntityInfoPersistenceWidget : public QWidget {
	Q_OBJECT
public:
	EntityInfoPersistenceWidget(GlobalInfo& globalInfo);

private slots:
	void onSelectEntity();
	void onStateChanged(int state);
	void onChangeEntityPersistence(EntityID entityID, bool isPersistent);

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<QCheckBox> m_persistenceCheckBox;
};