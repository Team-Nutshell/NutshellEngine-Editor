#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QScrollArea>
#include <QListWidget>

class LogsWidget : public QListWidget {
	Q_OBJECT
public:
	LogsWidget(GlobalInfo& globalInfo);
	~LogsWidget();

public:
	void updateLogs();

private:
	GlobalInfo& m_globalInfo;
};