#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>
#include <limits>

class ScalarWidget : public QWidget {
	Q_OBJECT
public:
	ScalarWidget(GlobalInfo& globalInfo, const std::string& name);

	void setValue(float value);
	float getValue();

	void setMin(float min);
	void setMax(float max);

signals:
	void valueChanged(float);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

	float m_value = 0.0f;

	float m_min = std::numeric_limits<float>::lowest();
	float m_max = std::numeric_limits<float>::max();

public:
	QLabel* nameLabel;
	QLineEdit* valueLineEdit;
};