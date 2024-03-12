#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class ColorPickerWidget : public QWidget {
	Q_OBJECT
public:
	ColorPickerWidget(GlobalInfo& globalInfo, const std::string& name, const nml::vec4& defaultColor);

signals:
	void colorChanged(const nml::vec4&);

private slots:
	void onColorButtonClicked();

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* nameLabel;
	QPushButton* colorButton;

	nml::vec4 color;
};