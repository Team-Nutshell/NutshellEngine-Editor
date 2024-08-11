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

	void setText(const std::string& text);
	const std::string& getText();

signals:
	void valueChanged(const std::string&);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

	std::string m_text = "";

public:
	QLabel* nameLabel;
	QLineEdit* valueLineEdit;
};