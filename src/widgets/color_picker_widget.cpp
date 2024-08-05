#include "color_picker_widget.h"
#include <QHBoxLayout>
#include <QColorDialog>
#include <QPalette>
#include <algorithm>

ColorPickerWidget::ColorPickerWidget(GlobalInfo& globalInfo, const std::string& name, const nml::vec4& defaultColor) : m_globalInfo(globalInfo), color(defaultColor) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	colorButton = new QPushButton();
	colorButton->setText("(" + QString::number(color.x, 'f', 2) + ", " + QString::number(color.y, 'f', 2) + ", " + QString::number(color.z, 'f', 2) + ", 1.00)");
	QPalette colorButtonPalette = colorButton->palette();
	colorButtonPalette.setColor(QPalette::ColorRole::Button, QColor::fromRgbF(color.x, color.y, color.z));
	colorButtonPalette.setColor(QPalette::ColorRole::ButtonText, QColor::fromRgbF(1.0f - std::clamp(color.x, 0.0f, 1.0f), 1.0f - std::clamp(color.y, 0.0f, 1.0f), 1.0f - std::clamp(color.z, 0.0f, 1.0f)));
	colorButton->setAutoFillBackground(true);
	colorButton->setPalette(colorButtonPalette);
	colorButton->update();
	layout()->addWidget(colorButton);
	layout()->setAlignment(colorButton, Qt::AlignmentFlag::AlignRight);

	connect(colorButton, &QPushButton::clicked, this, &ColorPickerWidget::onColorButtonClicked);
}

void ColorPickerWidget::updateColor(const nml::vec4& newColor) {
	color = newColor;

	colorButton->setText("(" + QString::number(color.x, 'f', 2) + ", " + QString::number(color.y, 'f', 2) + ", " + QString::number(color.z, 'f', 2) + ", 1.00)");
	QPalette colorButtonPalette = colorButton->palette();
	colorButtonPalette.setColor(QPalette::ColorRole::Button, QColor::fromRgbF(color.x, color.y, color.z));
	colorButtonPalette.setColor(QPalette::ColorRole::ButtonText, QColor::fromRgbF(1.0f - std::clamp(color.x, 0.0f, 1.0f), 1.0f - std::clamp(color.y, 0.0f, 1.0f), 1.0f - std::clamp(color.z, 0.0f, 1.0f)));
	colorButton->setAutoFillBackground(true);
	colorButton->setPalette(colorButtonPalette);
	colorButton->update();
}

void ColorPickerWidget::onColorButtonClicked() {
	QColorDialog colorDialog;
	colorDialog.setCurrentColor(QColor::fromRgbF(color.x, color.y, color.z, color.w));
	colorDialog.setWindowTitle("NutshellEngine - Select a color");
	colorDialog.setWindowIcon(QIcon("assets/icon.png"));
	if (colorDialog.exec()) {
		QColor newColor = colorDialog.currentColor();
		nml::vec4 newColorToVec4 = nml::vec4(newColor.redF(), newColor.greenF(), newColor.blueF(), newColor.alphaF());
		if (color != newColorToVec4) {
			updateColor(newColorToVec4);
			emit colorChanged(color);
		}
	}
}