#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include <QWidget>
#include <QPushButton>
#if defined(NTSHENGN_OS_WINDOWS)
#include <windows.h>
#undef far
#undef near
#elif defined(NTSHENGN_OS_LINUX) || defined(NTSHENGN_OS_FREEBSD)
#include <stdio.h>
#include <sys/types.h>
#endif

class BuildBar : public QWidget {
	Q_OBJECT
public:
	BuildBar(GlobalInfo& globalInfo);

	void launchBuild();

private:
	void launchExport();

	bool build();
	void run();
	void stopRun();
	void exportApplication(const std::string& exportDirectory);

	void addLog(std::string log);

	std::pair<std::string, std::string> parseVariableLineTokens(const std::vector<std::string>& tokens, bool usingNamespaceStd, bool usingNamespaceNtshEngnMath);
	void generateScriptManager();

#if defined(NTSHENGN_OS_LINUX) || defined(NTSHENGN_OS_FREEBSD)
	pid_t popen2(const char** command, int* inFp, int* outFp);
#endif

private slots:
	void onBuildRunExportStarted();
	void onBuildRunExportEnded();
	void onRunStarted();

signals:
	void runStarted();

private:
	GlobalInfo& m_globalInfo;

#if defined(NTSHENGN_OS_WINDOWS)
	HANDLE m_process = 0;
#elif defined(NTSHENGN_OS_LINUX) || defined(NTSHENGN_OS_FREEBSD)
	pid_t m_process = 0;
#endif

public:
	QPushButton* buildAndRunButton;
	QPushButton* exportButton;
	ComboBoxWidget* buildTypeComboBox;
	QPushButton* stopRunButton;
};
