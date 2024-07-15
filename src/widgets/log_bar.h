#pragma once
#include "../common/global_info.h"
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
	void mousePressEvent(QMouseEvent* event);

protected:
	void paintEvent(QPaintEvent* event);

private:
	GlobalInfo& m_globalInfo;
};