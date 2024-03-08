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
	void editingFinished(const nml::vec3&);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;
	
	nml::vec3 m_value = nml::vec3(0.0f);

public:
	std::unique_ptr<QLabel> nameLabel;
	std::unique_ptr<QLineEdit> xLineEdit;
	std::unique_ptr<QLineEdit> yLineEdit;
	std::unique_ptr<QLineEdit> zLineEdit;
};