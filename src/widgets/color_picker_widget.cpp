#include "color_picker_widget.h"
#include <QHBoxLayout>
#include <QColorDialog>
#include <QPalette>
#include <algorithm>

ColorPickerWidget::ColorPickerWidget(GlobalInfo& globalInfo, const std::string& name, const nml::vec4& defaultColor) : m_globalInfo(globalInfo), m_color(defaultColor) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = std::make_unique<QLabel>(QString::fromStdString(name));
	layout()->addWidget(nameLabel.get());
	colorButton = std::make_unique<QPushButton>();
	colorButton->setText(QString::fromStdString(nml::to_string(m_color)));
	QPalette colorButtonPalette = colorButton->palette();
	colorButtonPalette.setColor(QPalette::ColorRole::Button, QColor::fromRgbF(m_color.x, m_color.y, m_color.z));
	colorButtonPalette.setColor(QPalette::ColorRole::ButtonText, QColor::fromRgbF(1.0f - std::clamp(m_color.x, 0.0f, 1.0f), 1.0f - std::clamp(m_color.y, 0.0f, 1.0f), 1.0f - std::clamp(m_color.z, 0.0f, 1.0f)));
	colorButton->setAutoFillBackground(true);
	colorButton->setPalette(colorButtonPalette);
	colorButton->update();
	layout()->addWidget(colorButton.get());

	connect(colorButton.get(), &QPushButton::clicked, this, &ColorPickerWidget::onColorButtonClicked);
}

void ColorPickerWidget::onColorButtonClicked() {
	QColor newColor = QColorDialog::getColor(QColor::fromRgbF(m_color.x, m_color.y, m_color.z, m_color.w), nullptr, "Select a color");
	nml::vec4 newColorToVec4 = nml::vec4(newColor.redF(), newColor.greenF(), newColor.blueF(), newColor.alphaF());
	if (newColorToVec4 != m_color) {
		m_color = newColorToVec4;
		emit colorChanged(m_color);
	}
}