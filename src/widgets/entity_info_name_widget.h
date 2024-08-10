#pragma once
#include "../common/global_info.h"
#include <QLineEdit>
#include <string>

class EntityInfoNameWidget : public QLineEdit {
	Q_OBJECT
public:
	EntityInfoNameWidget(GlobalInfo& globalInfo);

private slots:
	void onEntitySelected();
	void onEditingFinished();
	void onEntityNameChanged(EntityID entityID, const std::string& name);

private:
	GlobalInfo& m_globalInfo;

	std::string m_previousName;
};