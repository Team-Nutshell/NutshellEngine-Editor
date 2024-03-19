#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class IntegerWidget : public QWidget {
	Q_OBJECT
public:
	IntegerWidget(GlobalInfo& globalInfo, const std::string& name);

signals:
	void valueChanged(int);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* nameLabel;
	QLineEdit* valueLineEdit;

	int value = 0;
};