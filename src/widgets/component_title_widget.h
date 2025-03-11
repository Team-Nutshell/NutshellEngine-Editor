#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QPushButton>
#include <string>

class ComponentTitleWidget : public QWidget {
	Q_OBJECT
public:
	ComponentTitleWidget(GlobalInfo& globalInfo, const std::string& name);

private slots:
	void onRemoveWidgetClicked();

private:
	GlobalInfo& m_globalInfo;

	std::string m_name;

public:
	QPushButton* removeWidgetButton;
};