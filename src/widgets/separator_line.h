#pragma once
#include "../common/common.h"
#include <QFrame>

class SeparatorLine : public QFrame {
	Q_OBJECT
public:
	SeparatorLine(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;
};