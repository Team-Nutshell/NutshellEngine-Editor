#pragma once
#include "../common/common.h"
#include "combo_box_widget.h"
#include <QWidget>
#include <QPushButton>

class BuildBar : public QWidget {
	Q_OBJECT
public:
	BuildBar(GlobalInfo& globalInfo);

private:
	void launchBuild();

	void build();
	void run();

private:
	GlobalInfo& m_globalInfo;

public:
	QPushButton* buildButton;
	ComboBoxWidget* buildTypeComboBox;
};