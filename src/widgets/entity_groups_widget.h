#pragma once
#include "../common/global_info.h"
#include "entity_group_name_widget.h"
#include <QWidget>
#include <QPushButton>
#include <string>

class EntityGroupsWidget : public QWidget {
	Q_OBJECT
public:
	EntityGroupsWidget(GlobalInfo& globalInfo);

private:
	void updateEntityGroups();

	EntityGroupNameWidget* addEntityGroupNameWidget();

private slots:
	void onEntitySelected();
	void onEntityGroupNameChanged();
	void onRemoveEntityGroupClicked();
	void onAddEntityGroupClicked();

private:
	GlobalInfo& m_globalInfo;

public:
	QPushButton* addEntityGroupButton;
};