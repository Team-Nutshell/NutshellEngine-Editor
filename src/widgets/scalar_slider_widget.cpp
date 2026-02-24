#include "scalar_slider_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <QSignalBlocker>
#include <algorithm>
#include <cstdlib>

ScalarSliderWidget::ScalarSliderWidget(GlobalInfo& globalInfo, const std::string& name, float min, float max) : m_globalInfo(globalInfo) {
	QLocale useDotLocale = QLocale(QLocale::Language::English, QLocale::Country::UnitedStates);
	QDoubleValidator* doubleValidator = new QDoubleValidator();
	doubleValidator->setLocale(useDotLocale);
	doubleValidator->setNotation(QDoubleValidator::Notation::StandardNotation);

	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	valueLineEdit = new QLineEdit("0.0");
	valueLineEdit->setValidator(doubleValidator);
	valueLineEdit->setFixedWidth(50);
	layout()->addWidget(valueLineEdit);
	layout()->setAlignment(valueLineEdit, Qt::AlignmentFlag::AlignRight);
	slider = new QSlider();
	slider->setOrientation(Qt::Orientation::Horizontal);
	slider->setMinimum(static_cast<int>(min * 100.0f));
	slider->setMaximum(static_cast<int>(max * 100.0f));
	slider->setFixedWidth(200);
	layout()->addWidget(slider);

	connect(valueLineEdit, &QLineEdit::editingFinished, this, &ScalarSliderWidget::onEditingFinished);
	connect(slider, &QSlider::sliderReleased, this, &ScalarSliderWidget::onSliderReleased);
	connect(slider, &QSlider::sliderMoved, this, &ScalarSliderWidget::onSliderMoved);
	connect(slider, &QSlider::valueChanged, this, &ScalarSliderWidget::onSliderValueChanged);

	setMin(min);
	setMax(max);
}

void ScalarSliderWidget::setValue(float value) {
	value = std::clamp(value, m_min, m_max);
	if (m_value != value) {
		m_value = value;
		valueLineEdit->setText(QString::number(m_value, 'g', 7));
		QFont font = valueLineEdit->font();
		font.setItalic(false);
		valueLineEdit->setFont(font);
		{
			const QSignalBlocker signalBlocker(slider);
			slider->setValue(static_cast<int>(value * 100.0f));
		}
	}
}

float ScalarSliderWidget::getValue() {
	return m_value;
}

void ScalarSliderWidget::setTemporaryValue(float value) {
	value = std::clamp(value, m_min, m_max);
	if (m_temporaryValue != value) {
		m_temporaryValue = value;
		valueLineEdit->setText(QString::number(m_temporaryValue, 'g', 7));
		QFont font = valueLineEdit->font();
		font.setItalic(true);
		valueLineEdit->setFont(font);
	}
}

float ScalarSliderWidget::getTemporaryValue() {
	return m_temporaryValue;
}

void ScalarSliderWidget::setMin(float min) {
	m_min = min;
	slider->setMinimum(static_cast<int>(min * 100.0f));
	setValue(m_value);
}

void ScalarSliderWidget::setMax(float max) {
	m_max = max;
	slider->setMaximum(static_cast<int>(max * 100.0f));
	setValue(m_value);
}

void ScalarSliderWidget::onEditingFinished() {
	float newValue = std::atof(valueLineEdit->text().toStdString().c_str());
	newValue = std::clamp(newValue, m_min, m_max);
	if (m_value != newValue) {
		setValue(newValue);
		emit valueChanged(m_value);
	}
	else {
		valueLineEdit->setText(QString::number(m_value, 'g', 7));
		QFont font = valueLineEdit->font();
		font.setItalic(false);
		valueLineEdit->setFont(font);
	}
	{
		const QSignalBlocker signalBlocker(valueLineEdit);
		valueLineEdit->clearFocus();
	}
}

void ScalarSliderWidget::onSliderReleased() {
	float newValue = static_cast<float>(slider->sliderPosition()) / 100.0f;
	if (m_value != newValue) {
		setValue(newValue);
		emit valueChanged(m_value);
	}
	else {
		valueLineEdit->setText(QString::number(m_value, 'g', 7));
		QFont font = valueLineEdit->font();
		font.setItalic(false);
		valueLineEdit->setFont(font);
	}
}

void ScalarSliderWidget::onSliderMoved() {
	float newValue = static_cast<float>(slider->sliderPosition()) / 100.0f;
	newValue = std::clamp(newValue, m_min, m_max);
	if (m_temporaryValue != newValue) {
		setTemporaryValue(newValue);
		emit temporaryValueChanged(m_temporaryValue);
	}
}

void ScalarSliderWidget::onSliderValueChanged() {
	if (!slider->isSliderDown()) {
		float newValue = static_cast<float>(slider->sliderPosition()) / 100.0f;
		if (m_value != newValue) {
			setValue(newValue);
			emit valueChanged(m_value);
		}
	}
}
