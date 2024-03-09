#include "vector2_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <stdlib.h>

Vector2Widget::Vector2Widget(GlobalInfo& globalInfo, const std::string& name): m_globalInfo(globalInfo) {
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
	layout()->addWidget(new QLabel("y:"));
	yLineEdit = std::make_unique<QLineEdit>("0.0");
	yLineEdit->setValidator(doubleValidator);
	layout()->addWidget(yLineEdit.get());

	connect(xLineEdit.get(), &QLineEdit::editingFinished, this, &Vector2Widget::onEditingFinished);
	connect(yLineEdit.get(), &QLineEdit::editingFinished, this, &Vector2Widget::onEditingFinished);
}

void Vector2Widget::onEditingFinished() {
	nml::vec2 newValue = nml::vec2(atof(xLineEdit->text().toStdString().c_str()), atof(yLineEdit->text().toStdString().c_str()));
	if (value != newValue) {
		value = newValue;
		emit valueChanged(value);
	}
}