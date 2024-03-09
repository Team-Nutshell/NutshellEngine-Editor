#pragma once
#include "../common/common.h"
#include <QLineEdit>
#include <string>

class EntityInfoNameWidget : public QLineEdit {
	Q_OBJECT
public:
	EntityInfoNameWidget(GlobalInfo& globalInfo);

private slots:
	void onSelectEntity();
	void onEditingFinished();
	void onChangeEntityName(EntityID entityID, const std::string& name);

private:
	GlobalInfo& m_globalInfo;

	std::string m_previousName;
};