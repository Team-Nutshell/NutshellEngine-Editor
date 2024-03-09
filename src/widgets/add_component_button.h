#pragma once
#include "../common/common.h"
#include "add_component_menu.h"
#include <QPushButton>
#include <memory>

class AddComponentButton : public QPushButton {
	Q_OBJECT
public:
	AddComponentButton(GlobalInfo& globalInfo);

private slots:
	void onClick();

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<AddComponentMenu> menu;
};