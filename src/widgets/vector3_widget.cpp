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

	connect(xLineEdit, &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
	connect(yLineEdit, &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
	connect(zLineEdit, &QLineEdit::editingFinished, this, &Vector3Widget::onEditingFinished);
}

void Vector3Widget::setValue(const nml::vec3& value) {
	m_value = value;
	xLineEdit->setText(QString::number(m_value.x, 'g', 7));
	yLineEdit->setText(QString::number(m_value.y, 'g', 7));
	zLineEdit->setText(QString::number(m_value.z, 'g', 7));
}

const nml::vec3& Vector3Widget::getValue() {
	return m_value;
}

void Vector3Widget::onEditingFinished() {
	nml::vec3 newValue = nml::vec3(std::atof(xLineEdit->text().toStdString().c_str()), std::atof(yLineEdit->text().toStdString().c_str()), std::atof(zLineEdit->text().toStdString().c_str()));
	QObject* senderWidget = sender();
	if (senderWidget == xLineEdit) {
		if (m_value.x != newValue.x) {
			emit xChanged(newValue.x);
		}
	}
	else if (senderWidget == yLineEdit) {
		if (m_value.y != newValue.y) {
			emit yChanged(newValue.y);
		}
	}
	else if (senderWidget == zLineEdit) {
		if (m_value.z != newValue.z) {
			emit zChanged(newValue.z);
		}
	}
	if (m_value != newValue) {
		setValue(newValue);
		emit valueChanged(m_value);
	}
	static_cast<QWidget*>(senderWidget)->clearFocus();
}