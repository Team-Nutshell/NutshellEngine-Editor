#pragma once
#include "../common/common.h"
#include <QLabel>
#include <string>

class LogBar : public QLabel {
	Q_OBJECT
public:
	LogBar(GlobalInfo& globalInfo);

	void updateLastLog(const Log& log);

private:
	GlobalInfo& m_globalInfo;
};