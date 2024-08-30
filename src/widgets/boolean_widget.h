#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <string>

class BooleanWidget : public QWidget {
	Q_OBJECT
public:
	BooleanWidget(GlobalInfo& globalInfo, const std::string& name);

	void setValue(bool value);
	bool getValue();

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