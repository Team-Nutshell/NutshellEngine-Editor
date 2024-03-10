#include "boolean_widget.h"
#include <QHBoxLayout>

BooleanWidget::BooleanWidget(GlobalInfo& globalInfo, const std::string& name): m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = std::make_unique<QLabel>(QString::fromStdString(name));
	layout()->addWidget(nameLabel.get());
	checkBox = std::make_unique<QCheckBox>();
	layout()->addWidget(checkBox.get());
	layout()->setAlignment(checkBox.get(), Qt::AlignmentFlag::AlignRight);
	
	connect(checkBox.get(), &QCheckBox::stateChanged, this, &BooleanWidget::onStateChanged);
}

void BooleanWidget::onStateChanged() {
	emit stateChanged(checkBox->isChecked());
}