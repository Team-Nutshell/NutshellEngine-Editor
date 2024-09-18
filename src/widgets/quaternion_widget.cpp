#include "quaternion_widget.h"
#include <QHBoxLayout>
#include <QLocale>
#include <QDoubleValidator>
#include <cstdlib>

QuaternionWidget::QuaternionWidget(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	QLocale useDotLocale = QLocale(QLocale::Language::English, QLocale::Country::UnitedStates);
	QDoubleValidator* doubleValidator = new QDoubleValidator();
	doubleValidator->setLocale(useDotLocale);
	doubleValidator->setNotation(QDoubleValidator::Notation::StandardNotation);

	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	aLabel = new QLabel("a:");
	layout()->addWidget(aLabel);
	layout()->setAlignment(aLabel, Qt::AlignmentFlag::AlignRight);
	aLineEdit = new QLineEdit("0.0");
	aLineEdit->setValidator(doubleValidator);
	layout()->addWidget(aLineEdit);
	layout()->setAlignment(aLineEdit, Qt::AlignmentFlag::AlignRight);
	bLabel = new QLabel("b:");
	layout()->addWidget(bLabel);
	layout()->setAlignment(bLabel, Qt::AlignmentFlag::AlignRight);
	bLineEdit = new QLineEdit("0.0");
	bLineEdit->setValidator(doubleValidator);
	layout()->addWidget(bLineEdit);
	layout()->setAlignment(bLineEdit, Qt::AlignmentFlag::AlignRight);
	cLabel = new QLabel("c:");
	layout()->addWidget(cLabel);
	layout()->setAlignment(cLabel, Qt::AlignmentFlag::AlignRight);
	cLineEdit = new QLineEdit("0.0");
	cLineEdit->setValidator(doubleValidator);
	layout()->addWidget(cLineEdit);
	layout()->setAlignment(cLineEdit, Qt::AlignmentFlag::AlignRight);
	dLabel = new QLabel("d:");
	layout()->addWidget(dLabel);
	layout()->setAlignment(dLabel, Qt::AlignmentFlag::AlignRight);
	dLineEdit = new QLineEdit("0.0");
	dLineEdit->setValidator(doubleValidator);
	layout()->addWidget(dLineEdit);
	layout()->setAlignment(dLineEdit, Qt::AlignmentFlag::AlignRight);

	connect(aLineEdit, &QLineEdit::editingFinished, this, &QuaternionWidget::onEditingFinished);
	connect(bLineEdit, &QLineEdit::editingFinished, this, &QuaternionWidget::onEditingFinished);
	connect(cLineEdit, &QLineEdit::editingFinished, this, &QuaternionWidget::onEditingFinished);
	connect(dLineEdit, &QLineEdit::editingFinished, this, &QuaternionWidget::onEditingFinished);
}

void QuaternionWidget::setValue(const nml::quat& value) {
	m_value = value;
	aLineEdit->setText(QString::number(m_value.a, 'f', 3));
	bLineEdit->setText(QString::number(m_value.b, 'f', 3));
	cLineEdit->setText(QString::number(m_value.c, 'f', 3));
	dLineEdit->setText(QString::number(m_value.d, 'f', 3));
}

const nml::quat& QuaternionWidget::getValue() {
	return m_value;
}

void QuaternionWidget::onEditingFinished() {
	nml::quat newValue = nml::quat(std::atof(aLineEdit->text().toStdString().c_str()), std::atof(bLineEdit->text().toStdString().c_str()), std::atof(cLineEdit->text().toStdString().c_str()), std::atof(dLineEdit->text().toStdString().c_str()));
	if (m_value != newValue) {
		setValue(newValue);
		emit valueChanged(m_value);
	}
	static_cast<QWidget*>(sender())->clearFocus();
}