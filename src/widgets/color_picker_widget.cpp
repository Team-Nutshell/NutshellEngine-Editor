#include "color_picker_widget.h"
#include <QHBoxLayout>
#include <QColorDialog>
#include <QPalette>
#include <algorithm>

ColorPickerWidget::ColorPickerWidget(GlobalInfo& globalInfo, const std::string& name, const nml::vec4& defaultColor) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	colorButton = new QPushButton();
	layout()->addWidget(colorButton);
	layout()->setAlignment(colorButton, Qt::AlignmentFlag::AlignRight);

	setColor(defaultColor);

	connect(colorButton, &QPushButton::clicked, this, &ColorPickerWidget::onColorButtonClicked);
}

void ColorPickerWidget::setColor(const nml::vec4& color) {
	m_color = color;
	float luminance = nml::dot(nml::vec3(m_color), nml::vec3(0.2126f, 0.7152f, 0.0722f));
	float buttonTextColor = 0.0f;
	if (luminance < 0.5f) {
		buttonTextColor = 1.0f;
	}
	colorButton->setText("(" + QString::number(m_color.x, 'f', 2) + ", " + QString::number(m_color.y, 'f', 2) + ", " + QString::number(m_color.z, 'f', 2) + ", 1.00)");
	QPalette colorButtonPalette = colorButton->palette();
	colorButtonPalette.setColor(QPalette::ColorRole::Button, QColor::fromRgbF(m_color.x, m_color.y, m_color.z));
	colorButtonPalette.setColor(QPalette::ColorRole::ButtonText, QColor::fromRgbF(buttonTextColor, buttonTextColor, buttonTextColor));
	colorButton->setAutoFillBackground(true);
	colorButton->setPalette(colorButtonPalette);
	colorButton->update();
}

const nml::vec4& ColorPickerWidget::getColor() {
	return m_color;
}

void ColorPickerWidget::onColorButtonClicked() {
	QColorDialog colorDialog;
	colorDialog.setCurrentColor(QColor::fromRgbF(m_color.x, m_color.y, m_color.z, m_color.w));
	colorDialog.setWindowTitle("NutshellEngine - Select a color");
	colorDialog.setWindowIcon(QIcon("assets/icon.png"));
	if (colorDialog.exec()) {
		QColor newColor = colorDialog.currentColor();
		nml::vec4 newColorToVec4 = nml::vec4(newColor.redF(), newColor.greenF(), newColor.blueF(), newColor.alphaF());
		if (m_color != newColorToVec4) {
			setColor(newColorToVec4);
			emit colorChanged(m_color);
		}
	}
}