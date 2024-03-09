#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>
#include <memory>

class Vector3Widget : public QWidget {
	Q_OBJECT
public:
	Vector3Widget(GlobalInfo& globalInfo, const std::string& name);

signals:
	void valueChanged(const nml::vec3&);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

public:
	std::unique_ptr<QLabel> nameLabel;
	std::unique_ptr<QLineEdit> xLineEdit;
	std::unique_ptr<QLineEdit> yLineEdit;
	std::unique_ptr<QLineEdit> zLineEdit;

	nml::vec3 value = nml::vec3(0.0f);
};