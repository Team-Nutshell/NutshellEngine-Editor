#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class ColorPickerWidget : public QWidget {
	Q_OBJECT
public:
	ColorPickerWidget(GlobalInfo& globalInfo, const std::string& name, const nml::vec3& defaultColor);

	void setColor(const nml::vec3& color);
	const nml::vec3& getColor();

signals:
	void colorChanged(const nml::vec3&);

private slots:
	void onColorButtonClicked();

private:
	GlobalInfo& m_globalInfo;

	nml::vec3 m_color;

public:
	QLabel* nameLabel;
	QPushButton* colorButton;
};