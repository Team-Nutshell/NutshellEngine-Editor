#include "integer_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QIntValidator>
#include <cstdlib>

IntegerWidget::IntegerWidget(GlobalInfo& globalInfo, const std::string& name): m_globalInfo(globalInfo) {
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

void IntegerWidget::onEditingFinished() {
	int newValue = std::atoi(valueLineEdit->text().toStdString().c_str());
	if (value != newValue) {
		value = newValue;
		emit valueChanged(value);
	}
}