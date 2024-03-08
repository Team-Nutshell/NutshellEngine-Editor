#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QPushButton>
#include <string>
#include <memory>

class ComponentTitleWidget : public QWidget {
	Q_OBJECT
public:
	ComponentTitleWidget(GlobalInfo& globalInfo, const std::string& name);

private slots:
	void onClick();

private:
	GlobalInfo& m_globalInfo;

	std::string m_name;

public:
	std::unique_ptr<QPushButton> removeWidgetButton;
};