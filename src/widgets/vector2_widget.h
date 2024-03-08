#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>
#include <memory>

class Vector2Widget : public QWidget {
	Q_OBJECT
public:
	Vector2Widget(GlobalInfo& globalInfo, const std::string& name);

signals:
	void editingFinished(const nml::vec2&);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;
	
	nml::vec2 m_value = nml::vec2(0.0f);

public:
	std::unique_ptr<QLabel> nameLabel;
	std::unique_ptr<QLineEdit> xLineEdit;
	std::unique_ptr<QLineEdit> yLineEdit;
};