#include "combo_box_widget.h"
#include <QHBoxLayout>

ComboBoxWidget::ComboBoxWidget(GlobalInfo& globalInfo, const std::string& name, std::vector<std::string>& elements) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	comboBox = new QComboBox();
	for (const std::string& element : elements) {
		comboBox->addItem(QString::fromStdString(element));
	}
	layout()->addWidget(comboBox);
	layout()->setAlignment(comboBox, Qt::AlignmentFlag::AlignRight);
	
	connect(comboBox, &QComboBox::currentTextChanged, this, &ComboBoxWidget::onElementSelected);
}

void ComboBoxWidget::onElementSelected(const QString& element) {
	emit elementSelected(element.toStdString());
}