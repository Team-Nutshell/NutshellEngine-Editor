#include "collapsable_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

CollapseBar::CollapseBar(const std::string& text) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	m_collapseButton = new QPushButton(">");
	m_collapseButton->setFixedWidth(20);
	m_collapseButton->setFlat(true);
	m_collapseButton->setStyleSheet("font-weight: bold");
	layout()->addWidget(m_collapseButton);
	m_collapseLabel = new QLabel(QString::fromStdString(text));
	layout()->addWidget(m_collapseLabel);

	connect(m_collapseButton, &QPushButton::clicked, this, &CollapseBar::onCollapseButtonPressed);
}

void CollapseBar::setText(const std::string& text) {
	m_collapseLabel->setText(QString::fromStdString(text));
}

void CollapseBar::onCollapseButtonPressed() {
	m_collapsed = !m_collapsed;
	if (m_collapsed) {
		m_collapseButton->setText(">");
	}
	else {
		m_collapseButton->setText("v");
	}

	emit collapseButtonPressed();
}

CollapsableWidget::CollapsableWidget(const std::string& text, QWidget* widget) : m_widget(widget) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	m_collapseBar = new CollapseBar(text);
	layout()->addWidget(m_collapseBar);
	m_widget->setVisible(false);
	layout()->addWidget(m_widget);

	connect(m_collapseBar, &CollapseBar::collapseButtonPressed, this, &CollapsableWidget::onCollapseButtonPressed);
}

void CollapsableWidget::setText(const std::string& text) {
	m_collapseBar->setText(text);
}

void CollapsableWidget::onCollapseButtonPressed() {
	m_widget->setVisible(!m_widget->isVisible());
}
