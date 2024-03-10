#include "vector3_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <stdlib.h>

Vector3Widget::Vector3Widget(GlobalInfo& globalInfo, const std::string& name): m_globalInfo(globalInfo) {
	QLocale useDotLocale = QLocale(QLocale::Language::English, QLocale::Country::UnitedStates);
	QDoubleValidator* doubleValidator = new QDoubleValidator();
	doubleValidator->setLocale(useDotLocale);
	doubleValidator->setNotation(QDoubleValidator::Notation::StandardNotation);

	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = std::make_unique<QLabel>(QString::fromStdString(name));
	layout()->addWidget(nameLabel.get());
	layout()->addWidget(new QLabel("x:"));
	xLineEdit = std::make_unique<QLineEdit>("0.0");
	xLineEdit->setValidator(doubleValidator);
	layout()->addWidget(xLineEdit.get());
	layout()->setAlignment(xLineEdit.get(), Qt::AlignmentFlag::AlignRight);
	layout()->addWidget(new QLabel("y:"));
	yLineEdit = std::make_unique<QLineEdit>("0.0");
	yLineEdit->setValidator(doubleValidator);
	layout()->addWidget(yLineEdit.get());
	layout()->setAlignment(yLineEdit.get(), Qt::AlignmentFlag::AlignRight);
	layout()->addWidget(new QLabel("z:"));
	zLineEdit = std::make_unique<QLineEdit>("0.0");
	zLineEdit->setValidator(doubleValidator);
	layout()->addWidget(zLineEdit.get());
	layout()->setAlignment(zLineEdit.get(), Qt::AlignmentFlag::AlignRight);

	connect(xLineEdit.get(), &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
	connect(yLineEdit.get(), &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
	connect(zLineEdit.get(), &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
}

void Vector3Widget::onEditingFinished() {
	nml::vec3 newValue = nml::vec3(atof(xLineEdit->text().toStdString().c_str()), atof(yLineEdit->text().toStdString().c_str()), atof(zLineEdit->text().toStdString().c_str()));
	if (value != newValue) {
		value = newValue;
		emit valueChanged(value);
	}
}