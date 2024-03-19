#include "vector2_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <cstdlib>

Vector2Widget::Vector2Widget(GlobalInfo& globalInfo, const std::string& name): m_globalInfo(globalInfo) {
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

	connect(xLineEdit, &QLineEdit::editingFinished, this, &Vector2Widget::onEditingFinished);
	connect(yLineEdit, &QLineEdit::editingFinished, this, &Vector2Widget::onEditingFinished);
}

void Vector2Widget::onEditingFinished() {
	nml::vec2 newValue = nml::vec2(std::atof(xLineEdit->text().toStdString().c_str()), std::atof(yLineEdit->text().toStdString().c_str()));
	if (value != newValue) {
		value = newValue;
		emit valueChanged(value);
	}
}