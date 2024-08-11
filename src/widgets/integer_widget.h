#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>
#include <limits>

class IntegerWidget : public QWidget {
	Q_OBJECT
public:
	IntegerWidget(GlobalInfo& globalInfo, const std::string& name);

	void setValue(int value);
	int getValue();

	void setMin(int min);
	void setMax(int max);

signals:
	void valueChanged(int);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

	int m_value = 0;

	int m_min = std::numeric_limits<int>::min();
	int m_max = std::numeric_limits<int>::max();

public:
	QLabel* nameLabel;
	QLineEdit* valueLineEdit;
};