#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include <QWidget>
#include <QPushButton>

class BuildBar : public QWidget {
	Q_OBJECT
public:
	BuildBar(GlobalInfo& globalInfo);

	void launchBuild();

private:
	void launchExport();

	bool build();
	void run();
	void exportApplication(const std::string& exportDirectory);

	void addLog(std::string log);

	std::pair<std::string, std::string> parseVariableLineTokens(const std::vector<std::string>& tokens, bool usingNamespaceStd, bool usingNamespaceNtshEngnMath);
	void generateScriptManager();

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