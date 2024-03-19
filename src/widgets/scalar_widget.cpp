#include "scalar_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
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

void ScalarWidget::onEditingFinished() {
	float newValue = std::atof(valueLineEdit->text().toStdString().c_str());
	if (value != newValue) {
		value = newValue;
		emit valueChanged(value);
	}
}