#include "integer_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QIntValidator>
#include <algorithm>
#include <cstdlib>

IntegerWidget::IntegerWidget(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	QIntValidator* intValidator = new QIntValidator();

	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	valueLineEdit = new QLineEdit("0");
	valueLineEdit->setValidator(intValidator);
	layout()->addWidget(valueLineEdit);
	layout()->setAlignment(valueLineEdit, Qt::AlignmentFlag::AlignRight);

	connect(valueLineEdit, &QLineEdit::editingFinished, this, &IntegerWidget::onEditingFinished);
}

void IntegerWidget::setValue(int value) {
	value = std::clamp(value, m_min, m_max);
	if (m_value != value) {
		m_value = value;
		valueLineEdit->setText(QString::number(m_value));
	}
}

int IntegerWidget::getValue() {
	return m_value;
}

void IntegerWidget::setMin(int min) {
	m_min = min;
	setValue(m_value);
}

void IntegerWidget::setMax(int max) {
	m_max = max;
	setValue(m_value);
}

void IntegerWidget::onEditingFinished() {
	int newValue = std::atoi(valueLineEdit->text().toStdString().c_str());
	newValue = std::clamp(newValue, m_min, m_max);
	if (m_value != newValue) {
		setValue(newValue);
		emit valueChanged(m_value);
	}
}