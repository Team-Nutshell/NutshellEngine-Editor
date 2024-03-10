#include "combo_box_widget.h"
#include <QHBoxLayout>

ComboBoxWidget::ComboBoxWidget(GlobalInfo& globalInfo, const std::string& name, std::vector<std::string>& elements): m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = std::make_unique<QLabel>(QString::fromStdString(name));
	layout()->addWidget(nameLabel.get());
	comboBox = std::make_unique<QComboBox>();
	for (const std::string& element : elements) {
		comboBox->addItem(QString::fromStdString(element));
	}
	layout()->addWidget(comboBox.get());
	layout()->setAlignment(comboBox.get(), Qt::AlignmentFlag::AlignRight);
	
	connect(comboBox.get(), &QComboBox::currentTextChanged, this, &ComboBoxWidget::onElementSelected);
}

void ComboBoxWidget::onElementSelected(const QString& element) {
	emit elementSelected(element.toStdString());
}