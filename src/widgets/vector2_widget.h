#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class Vector2Widget : public QWidget {
	Q_OBJECT
public:
	Vector2Widget(GlobalInfo& globalInfo, const std::string& name);

signals:
	void valueChanged(const nml::vec2&);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* nameLabel;
	QLineEdit* xLineEdit;
	QLineEdit* yLineEdit;

	nml::vec2 value = nml::vec2(0.0f);
};