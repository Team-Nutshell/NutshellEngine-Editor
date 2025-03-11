#include "entity_group_name_widget.h"
#include <QHBoxLayout>

EntityGroupNameWidget::EntityGroupNameWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	entityGroupNameLineEdit = new QLineEdit("");
	layout()->addWidget(entityGroupNameLineEdit);
	removeEntityGroupButton = new QPushButton("X");
	removeEntityGroupButton->setFixedWidth(20);
	layout()->addWidget(removeEntityGroupButton);

	connect(entityGroupNameLineEdit, &QLineEdit::editingFinished, this, &EntityGroupNameWidget::onEditingFinished);
	connect(removeEntityGroupButton, &QPushButton::clicked, this, &EntityGroupNameWidget::onRemoveEntityGroupClicked);
}

void EntityGroupNameWidget::setText(const std::string& text) {
	m_text = text;
	entityGroupNameLineEdit->setText(QString::fromStdString(m_text));
}

const std::string& EntityGroupNameWidget::getText() {
	return m_text;
}

void EntityGroupNameWidget::onRemoveEntityGroupClicked() {
	emit removeEntityGroupClicked();
}

void EntityGroupNameWidget::onEditingFinished() {
	std::string newValue = entityGroupNameLineEdit->text().toStdString();
	if (m_text != newValue) {
		setText(newValue);
		emit entityGroupNameChanged(m_text);
	}
	entityGroupNameLineEdit->clearFocus();
}