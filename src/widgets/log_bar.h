#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <string>

class LogBar : public QWidget {
	Q_OBJECT
public:
	LogBar(GlobalInfo& globalInfo);

	void updateLastLog(const Log& log);

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* lastLogLabel;
};