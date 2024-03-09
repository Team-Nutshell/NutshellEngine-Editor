#include "scalar_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <stdlib.h>

ScalarWidget::ScalarWidget(GlobalInfo& globalInfo, const std::string& name): m_globalInfo(globalInfo) {
	QLocale useDotLocale = QLocale(QLocale::Language::English, QLocale::Country::UnitedStates);
	QDoubleValidator* doubleValidator = new QDoubleValidator();
	doubleValidator->setLocale(useDotLocale);
	doubleValidator->setNotation(QDoubleValidator::Notation::StandardNotation);

	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = std::make_unique<QLabel>(QString::fromStdString(name));
	layout()->addWidget(nameLabel.get());
	valueLineEdit = std::make_unique<QLineEdit>("0.0");
	valueLineEdit->setValidator(doubleValidator);
	layout()->addWidget(valueLineEdit.get());

	connect(valueLineEdit.get(), &QLineEdit::editingFinished, this, &ScalarWidget::onEditingFinished);
}

void ScalarWidget::onEditingFinished() {
	float newValue = atof(valueLineEdit->text().toStdString().c_str());
	if (value != newValue) {
		value = newValue;
		emit valueChanged(value);
	}
}