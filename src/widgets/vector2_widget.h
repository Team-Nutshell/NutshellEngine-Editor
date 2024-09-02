#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class Vector2Widget : public QWidget {
	Q_OBJECT
public:
	Vector2Widget(GlobalInfo& globalInfo, const std::string& name);

	void setValue(const nml::vec2& value);
	const nml::vec2& getValue();

signals:
	void valueChanged(const nml::vec2&);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

	nml::vec2 m_value = nml::vec2(0.0f);

public:
	QLabel* nameLabel;
	QLabel* xLabel;
	QLineEdit* xLineEdit;
	QLabel* yLabel;
	QLineEdit* yLineEdit;
};