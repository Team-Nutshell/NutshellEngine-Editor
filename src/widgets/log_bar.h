#pragma once
#include "../common/common.h"
#include <QLabel>
#include <QMouseEvent>
#include <string>

class LogBar : public QLabel {
	Q_OBJECT
public:
	LogBar(GlobalInfo& globalInfo);

	void updateLastLog();

private slots:
	void mousePressEvent(QMouseEvent* event);

private:
	GlobalInfo& m_globalInfo;
};