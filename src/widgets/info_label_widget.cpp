#include "info_label_widget.h"
#include <QHBoxLayout>

InfoLabelWidget::InfoLabelWidget(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 5, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	valueLabel = new QLabel();
	layout()->addWidget(valueLabel);
	layout()->setAlignment(valueLabel, Qt::AlignmentFlag::AlignRight);
}

void InfoLabelWidget::setText(const std::string& text) {
	m_text = text;
	valueLabel->setText(QString::fromStdString(m_text));
}

const std::string& InfoLabelWidget::getText() {
	return m_text;
}