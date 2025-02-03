#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <string>

class InfoLabelWidget : public QWidget {
	Q_OBJECT
public:
	InfoLabelWidget(GlobalInfo& globalInfo, const std::string& name);

	void setText(const std::string& text);
	const std::string& getText();

private:
	GlobalInfo& m_globalInfo;

	std::string m_text = "";

public:
	QLabel* nameLabel;
	QLabel* valueLabel;
};