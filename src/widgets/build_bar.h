#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include <QWidget>
#include <QPushButton>

class BuildBar : public QWidget {
	Q_OBJECT
public:
	BuildBar(GlobalInfo& globalInfo);

private:
	void launchBuild();

	bool build();
	void run();

private slots:
	void onBuildAndRunStarted();
	void onBuildAndRunEnded();

private:
	GlobalInfo& m_globalInfo;
	
	bool m_isBuilding = false;

public:
	QPushButton* buildButton;
	ComboBoxWidget* buildTypeComboBox;
};