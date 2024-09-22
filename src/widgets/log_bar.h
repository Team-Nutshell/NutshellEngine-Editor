#pragma once
#include "../common/global_info.h"
#include "log_bar_menu.h"
#include "logs_widget.h"
#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include <string>

class LogBar : public QLabel {
	Q_OBJECT
public:
	LogBar(GlobalInfo& globalInfo);

private slots:
	void onLogAdded();
	void onLogsCleared();
	void showMenu(const QPoint& pos);
	void mousePressEvent(QMouseEvent* event);

	void onLogsWidgetClose();

protected:
	void paintEvent(QPaintEvent* event);

private:
	GlobalInfo& m_globalInfo;

	LogsWidget* m_logsWidget = nullptr;

public:
	LogBarMenu* menu;
};