#include "vector3_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <cstdlib>

Vector3Widget::Vector3Widget(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	QLocale useDotLocale = QLocale(QLocale::Language::English, QLocale::Country::UnitedStates);
	QDoubleValidator* doubleValidator = new QDoubleValidator();
	doubleValidator->setLocale(useDotLocale);
	doubleValidator->setNotation(QDoubleValidator::Notation::StandardNotation);

	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	layout()->addWidget(new QLabel("x:"));
	xLineEdit = new QLineEdit("0.0");
	xLineEdit->setValidator(doubleValidator);
	layout()->addWidget(xLineEdit);
	layout()->setAlignment(xLineEdit, Qt::AlignmentFlag::AlignRight);
	layout()->addWidget(new QLabel("y:"));
	yLineEdit = new QLineEdit("0.0");
	yLineEdit->setValidator(doubleValidator);
	layout()->addWidget(yLineEdit);
	layout()->setAlignment(yLineEdit, Qt::AlignmentFlag::AlignRight);
	layout()->addWidget(new QLabel("z:"));
	zLineEdit = new QLineEdit("0.0");
	zLineEdit->setValidator(doubleValidator);
	layout()->addWidget(zLineEdit);
	layout()->setAlignment(zLineEdit, Qt::AlignmentFlag::AlignRight);

	connect(xLineEdit, &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
	connect(yLineEdit, &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
	connect(zLineEdit, &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
}

void Vector3Widget::setValue(const nml::vec3& value) {
	m_value = value;
	xLineEdit->setText(QString::number(m_value.x, 'f', 3));
	yLineEdit->setText(QString::number(m_value.y, 'f', 3));
	zLineEdit->setText(QString::number(m_value.z, 'f', 3));
}

const nml::vec3& Vector3Widget::getValue() {
	return m_value;
}

void Vector3Widget::onEditingFinished() {
	nml::vec3 newValue = nml::vec3(std::atof(xLineEdit->text().toStdString().c_str()), std::atof(yLineEdit->text().toStdString().c_str()), std::atof(zLineEdit->text().toStdString().c_str()));
	if (m_value != newValue) {
		setValue(newValue);
		emit valueChanged(m_value);
	}
	static_cast<QWidget*>(sender())->clearFocus();
}