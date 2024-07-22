#include "key_select_widget.h"
#include <QKeySequence>
#include <QHBoxLayout>

KeySelectWidget::KeySelectWidget(GlobalInfo& globalInfo, const std::string& name, const std::string& key) : m_globalInfo(globalInfo) {
	setFocusPolicy(Qt::ClickFocus);
	
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	button = new QPushButton(QString::fromStdString(key));
	layout()->addWidget(button);
	layout()->setAlignment(button, Qt::AlignmentFlag::AlignRight);

	connect(button, &QPushButton::clicked, this, &KeySelectWidget::onButtonClicked);
}

void KeySelectWidget::keyPressEvent(QKeyEvent* event) {
	if (!m_changeKey) {
		return;
	}

	QKeySequence keySequence = QKeySequence(event->key());
	button->setText(keySequence.toString());

	m_changeKey = false;

	button->setStyleSheet("");

	emit keyChanged(keySequence.toString().toStdString());
}

void KeySelectWidget::focusOutEvent(QFocusEvent* event) {
	(void)event;

	m_changeKey = false;

	button->setStyleSheet("");
}

void KeySelectWidget::onButtonClicked() {
	setFocus();

	m_changeKey = true;

	button->setStyleSheet("font: italic; color: #ffa200;");
}