#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class Vector4Widget : public QWidget {
	Q_OBJECT
public:
	Vector4Widget(GlobalInfo& globalInfo, const std::string& name);

	void setValue(const nml::vec4& value);
	const nml::vec4& getValue();

signals:
	void valueChanged(const nml::vec4&);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

	nml::vec4 m_value = nml::vec4(0.0f);

public:
	QLabel* nameLabel;
	QLabel* xLabel;
	QLineEdit* xLineEdit;
	QLabel* yLabel;
	QLineEdit* yLineEdit;
	QLabel* zLabel;
	QLineEdit* zLineEdit;
	QLabel* wLabel;
	QLineEdit* wLineEdit;
};