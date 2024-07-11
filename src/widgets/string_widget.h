#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class StringWidget : public QWidget {
	Q_OBJECT
public:
	StringWidget(GlobalInfo& globalInfo, const std::string& name);

signals:
	void valueChanged(std::string);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* nameLabel;
	QLineEdit* valueLineEdit;

	std::string value = "";
};