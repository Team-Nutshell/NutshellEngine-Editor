#pragma once
#include "../common/global_info.h"
#include <QTableWidget>

class LogsWidget : public QTableWidget {
	Q_OBJECT
public:
	LogsWidget(GlobalInfo& globalInfo);

public:
	void updateLogs();

private slots:
	void onLogAdded();

private:
	GlobalInfo& m_globalInfo;

	size_t currentLog = 0;
};