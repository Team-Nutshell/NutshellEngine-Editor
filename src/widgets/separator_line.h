#pragma once
#include "../common/global_info.h"
#include <QFrame>

class SeparatorLine : public QFrame {
	Q_OBJECT
public:
	SeparatorLine(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;
};