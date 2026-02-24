#include "color_picker_widget.h"
#include <QHBoxLayout>
#include <QColorDialog>
#include <QPalette>
#include <algorithm>

ColorPickerWidget::ColorPickerWidget(GlobalInfo& globalInfo, const std::string& name, const nml::vec3& defaultColor) : m_globalInfo(globalInfo) {
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

void ColorPickerWidget::setColor(const nml::vec3& color) {
	m_color = color;
	float luminance = nml::dot(nml::vec3(m_color), nml::vec3(0.2126f, 0.7152f, 0.0722f));
	float buttonTextColor = 0.0f;
	if (luminance < 0.5f) {
		buttonTextColor = 1.0f;
	}
	colorButton->setText("(" + QString::number(m_color.x, 'g', 2) + ", " + QString::number(m_color.y, 'g', 2) + ", " + QString::number(m_color.z, 'g', 2) + ")");
	QFont buttonFont = colorButton->font();
	buttonFont.setItalic(false);
	colorButton->setFont(buttonFont);
	QPalette colorButtonPalette = colorButton->palette();
	colorButtonPalette.setColor(QPalette::ColorRole::Button, QColor::fromRgbF(m_color.x, m_color.y, m_color.z));
	colorButtonPalette.setColor(QPalette::ColorRole::ButtonText, QColor::fromRgbF(buttonTextColor, buttonTextColor, buttonTextColor));
	colorButton->setAutoFillBackground(true);
	colorButton->setPalette(colorButtonPalette);
	colorButton->update();
}

const nml::vec3& ColorPickerWidget::getColor() {
	return m_color;
}

void ColorPickerWidget::setTemporaryColor(const nml::vec3& color) {
	m_temporaryColor = color;
	float luminance = nml::dot(nml::vec3(m_temporaryColor), nml::vec3(0.2126f, 0.7152f, 0.0722f));
	float buttonTextColor = 0.0f;
	if (luminance < 0.5f) {
		buttonTextColor = 1.0f;
	}
	colorButton->setText("(" + QString::number(m_temporaryColor.x, 'g', 2) + ", " + QString::number(m_temporaryColor.y, 'g', 2) + ", " + QString::number(m_temporaryColor.z, 'g', 2) + ")");
	QFont buttonFont = colorButton->font();
	buttonFont.setItalic(true);
	colorButton->setFont(buttonFont);
	QPalette colorButtonPalette = colorButton->palette();
	colorButtonPalette.setColor(QPalette::ColorRole::Button, QColor::fromRgbF(m_temporaryColor.x, m_temporaryColor.y, m_temporaryColor.z));
	colorButtonPalette.setColor(QPalette::ColorRole::ButtonText, QColor::fromRgbF(buttonTextColor, buttonTextColor, buttonTextColor));
	colorButton->setAutoFillBackground(true);
	colorButton->setPalette(colorButtonPalette);
	colorButton->update();
}

const nml::vec3& ColorPickerWidget::getTemporaryColor() {
	return m_temporaryColor;
}

void ColorPickerWidget::onCurrentColorChanged(const QColor& color) {
	nml::vec3 tempColor = nml::vec3(color.redF(), color.greenF(), color.blueF());
	setTemporaryColor(tempColor);
	emit temporaryColorChanged(tempColor);
}

void ColorPickerWidget::onColorButtonClicked() {
	QColorDialog colorDialog;
	colorDialog.setCurrentColor(QColor::fromRgbF(m_color.x, m_color.y, m_color.z));
	colorDialog.setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("select_color")));
	colorDialog.setWindowIcon(QIcon("assets/icon.png"));
	connect(&colorDialog, &QColorDialog::currentColorChanged, this, &ColorPickerWidget::onCurrentColorChanged);
	if (colorDialog.exec()) {
		QColor newColor = colorDialog.currentColor();
		nml::vec3 newColorToVec3 = nml::vec3(newColor.redF(), newColor.greenF(), newColor.blueF());
		if (m_color != newColorToVec3) {
			setColor(newColorToVec3);
			emit colorChanged(m_color);
		}
	}
	else {
		setColor(m_color);
		emit colorChanged(m_color);
	}
}