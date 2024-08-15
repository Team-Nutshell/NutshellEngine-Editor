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
	void launchExport();

	bool build();
	void run();
	void exportApplication();

private slots:
	void onBuildRunExportStarted();
	void onBuildRunExportEnded();

private:
	GlobalInfo& m_globalInfo;

public:
	QPushButton* buildAndRunButton;
	QPushButton* exportButton;
	ComboBoxWidget* buildTypeComboBox;
};