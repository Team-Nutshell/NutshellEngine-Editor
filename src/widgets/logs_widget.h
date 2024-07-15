#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>

class LogsWidget : public QWidget {
	Q_OBJECT
public:
	LogsWidget(GlobalInfo& globalInfo);

public:
	void updateLogs();

private slots:
	void onClearLogsButtonClicked();
	void onLogAdded();
	void onLogsCleared();

private:
	GlobalInfo& m_globalInfo;

	size_t m_currentLog = 0;

public:
	QTableWidget* logsTable;
	QPushButton* clearLogsButton;
};