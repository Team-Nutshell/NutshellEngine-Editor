#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QScrollArea>
#include <QListWidget>

class LogsWidget : public QListWidget {
	Q_OBJECT
public:
	LogsWidget(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;
};