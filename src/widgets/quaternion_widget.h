#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class QuaternionWidget : public QWidget {
	Q_OBJECT
public:
	QuaternionWidget(GlobalInfo& globalInfo, const std::string& name);

	void setValue(const nml::quat& value);
	const nml::quat& getValue();

signals:
	void valueChanged(const nml::quat&);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

	nml::quat m_value = nml::quat(0.0f, 0.0f, 0.0f, 0.0f);

public:
	QLabel* nameLabel;
	QLabel* aLabel;
	QLineEdit* aLineEdit;
	QLabel* bLabel;
	QLineEdit* bLineEdit;
	QLabel* cLabel;
	QLineEdit* cLineEdit;
	QLabel* dLabel;
	QLineEdit* dLineEdit;
};