#pragma once
#include "../common/common.h"
#include "add_component_menu.h"
#include <QPushButton>

class AddComponentButton : public QPushButton {
	Q_OBJECT
public:
	AddComponentButton(GlobalInfo& globalInfo);

private slots:
	void onClick();

private:
	GlobalInfo& m_globalInfo;

	AddComponentMenu* menu;
};