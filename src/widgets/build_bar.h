#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QPushButton>

class BuildBar : public QWidget {
	Q_OBJECT
public:
	BuildBar(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	QPushButton* buildButton;
};