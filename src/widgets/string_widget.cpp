#include "string_widget.h"
#include <QHBoxLayout>
#include <stdlib.h>

StringWidget::StringWidget(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	valueLineEdit = new QLineEdit("");
	layout()->addWidget(valueLineEdit);
	layout()->setAlignment(valueLineEdit, Qt::AlignmentFlag::AlignRight);

	connect(valueLineEdit, &QLineEdit::editingFinished, this, &StringWidget::onEditingFinished);
}

void StringWidget::setText(const std::string& text) {
	m_text = text;
	valueLineEdit->setText(QString::fromStdString(m_text));
}

const std::string& StringWidget::getText() {
	return m_text;
}

void StringWidget::onEditingFinished() {
	std::string newValue = valueLineEdit->text().toStdString();
	if (m_text != newValue) {
		setText(newValue);
		emit valueChanged(m_text);
	}
}