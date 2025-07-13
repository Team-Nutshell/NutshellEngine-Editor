#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class Vector3Widget : public QWidget {
	Q_OBJECT
public:
	Vector3Widget(GlobalInfo& globalInfo, const std::string& name);

	void setValue(const nml::vec3& value);
	const nml::vec3& getValue();

signals:
	void valueChanged(const nml::vec3&);
	void xChanged(float);
	void yChanged(float);
	void zChanged(float);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

	nml::vec3 m_value = nml::vec3(0.0f);

public:
	QLabel* nameLabel;
	QLabel* xLabel;
	QLineEdit* xLineEdit;
	QLabel* yLabel;
	QLineEdit* yLineEdit;
	QLabel* zLabel;
	QLineEdit* zLineEdit;
};