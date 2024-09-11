#include "vector4_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <cstdlib>

Vector4Widget::Vector4Widget(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	QLocale useDotLocale = QLocale(QLocale::Language::English, QLocale::Country::UnitedStates);
	QDoubleValidator* doubleValidator = new QDoubleValidator();
	doubleValidator->setLocale(useDotLocale);
	doubleValidator->setNotation(QDoubleValidator::Notation::StandardNotation);

	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	xLabel = new QLabel("x:");
	layout()->addWidget(xLabel);
	layout()->setAlignment(xLabel, Qt::AlignmentFlag::AlignRight);
	xLineEdit = new QLineEdit("0.0");
	xLineEdit->setValidator(doubleValidator);
	layout()->addWidget(xLineEdit);
	layout()->setAlignment(xLineEdit, Qt::AlignmentFlag::AlignRight);
	yLabel = new QLabel("y:");
	layout()->addWidget(yLabel);
	layout()->setAlignment(yLabel, Qt::AlignmentFlag::AlignRight);
	yLineEdit = new QLineEdit("0.0");
	yLineEdit->setValidator(doubleValidator);
	layout()->addWidget(yLineEdit);
	layout()->setAlignment(yLineEdit, Qt::AlignmentFlag::AlignRight);
	zLabel = new QLabel("z:");
	layout()->addWidget(zLabel);
	layout()->setAlignment(zLabel, Qt::AlignmentFlag::AlignRight);
	zLineEdit = new QLineEdit("0.0");
	zLineEdit->setValidator(doubleValidator);
	layout()->addWidget(zLineEdit);
	layout()->setAlignment(zLineEdit, Qt::AlignmentFlag::AlignRight);
	wLabel = new QLabel("w:");
	layout()->addWidget(wLabel);
	layout()->setAlignment(wLabel, Qt::AlignmentFlag::AlignRight);
	wLineEdit = new QLineEdit("0.0");
	wLineEdit->setValidator(doubleValidator);
	layout()->addWidget(wLineEdit);
	layout()->setAlignment(wLineEdit, Qt::AlignmentFlag::AlignRight);

	connect(xLineEdit, &QLineEdit::editingFinished, this, &Vector4Widget::onEditingFinished);
	connect(yLineEdit, &QLineEdit::editingFinished, this, &Vector4Widget::onEditingFinished);
	connect(zLineEdit, &QLineEdit::editingFinished, this, &Vector4Widget::onEditingFinished);
	connect(wLineEdit, &QLineEdit::editingFinished, this, &Vector4Widget::onEditingFinished);
}

void Vector4Widget::setValue(const nml::vec4& value) {
	m_value = value;
	xLineEdit->setText(QString::number(m_value.x, 'f', 3));
	yLineEdit->setText(QString::number(m_value.y, 'f', 3));
	zLineEdit->setText(QString::number(m_value.z, 'f', 3));
	wLineEdit->setText(QString::number(m_value.w, 'f', 3));
}

const nml::vec4& Vector4Widget::getValue() {
	return m_value;
}

void Vector4Widget::onEditingFinished() {
	nml::vec4 newValue = nml::vec4(std::atof(xLineEdit->text().toStdString().c_str()), std::atof(yLineEdit->text().toStdString().c_str()), std::atof(zLineEdit->text().toStdString().c_str()), std::atof(wLineEdit->text().toStdString().c_str()));
	if (m_value != newValue) {
		setValue(newValue);
		emit valueChanged(m_value);
	}
	static_cast<QWidget*>(sender())->clearFocus();
}