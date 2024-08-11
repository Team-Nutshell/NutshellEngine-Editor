#include "scalar_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <algorithm>
#include <cstdlib>

ScalarWidget::ScalarWidget(GlobalInfo& globalInfo, const std::string& name): m_globalInfo(globalInfo) {
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
	layout()->addWidget(valueLineEdit);
	layout()->setAlignment(valueLineEdit, Qt::AlignmentFlag::AlignRight);

	connect(valueLineEdit, &QLineEdit::editingFinished, this, &ScalarWidget::onEditingFinished);
}

void ScalarWidget::setValue(float value) {
	value = std::clamp(value, m_min, m_max);
	if (m_value != value) {
		m_value = value;
		valueLineEdit->setText(QString::number(m_value, 'f', 3));
	}
}

float ScalarWidget::getValue() {
	return m_value;
}

void ScalarWidget::setMin(float min) {
	m_min = min;
	setValue(m_value);
}

void ScalarWidget::setMax(float max) {
	m_max = max;
	setValue(m_value);
}

void ScalarWidget::onEditingFinished() {
	float newValue = std::atof(valueLineEdit->text().toStdString().c_str());
	newValue = std::clamp(newValue, m_min, m_max);
	if (m_value != newValue) {
		setValue(newValue);
		emit valueChanged(m_value);
	}
}