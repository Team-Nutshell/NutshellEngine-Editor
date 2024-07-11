#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class ScalarWidget : public QWidget {
	Q_OBJECT
public:
	ScalarWidget(GlobalInfo& globalInfo, const std::string& name);

signals:
	void valueChanged(float);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* nameLabel;
	QLineEdit* valueLineEdit;

	float value = 0.0f;
};