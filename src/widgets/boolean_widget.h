#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <string>

class BooleanWidget : public QWidget {
	Q_OBJECT
public:
	BooleanWidget(GlobalInfo& globalInfo, const std::string& name);

signals:
	void stateChanged(bool);

private slots:
	void onStateChanged();

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* nameLabel;
	QCheckBox* checkBox;
};