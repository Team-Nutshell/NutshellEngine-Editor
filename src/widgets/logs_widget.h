#pragma once
#include "../common/global_info.h"
#include <QTableWidget>

class LogsWidget : public QTableWidget {
	Q_OBJECT
public:
	LogsWidget(GlobalInfo& globalInfo);
	~LogsWidget();

public:
	void updateLogs();

private:
	GlobalInfo& m_globalInfo;

	size_t currentLog = 0;
};