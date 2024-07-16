#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class LogBarMenu : public QMenu {
	Q_OBJECT
public:
	LogBarMenu(GlobalInfo& globalInfo);

private slots:
	void clearLogs();

private:
	GlobalInfo& m_globalInfo;

public:
	QAction* clearLogsAction;
};