#include "new_project_name_widget.h"
#include <QHBoxLayout>
#include <QLabel>

NewProjectNameWidget::NewProjectNameWidget(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new QLabel("New project name:"));
	m_projectNameLineEdit = new QLineEdit();
	layout()->addWidget(m_projectNameLineEdit);

	connect(m_projectNameLineEdit, &QLineEdit::textChanged, this, &NewProjectNameWidget::onTextChanged);
}

void NewProjectNameWidget::onTextChanged() {
	emit textChanged(m_projectNameLineEdit->text().toStdString());
}