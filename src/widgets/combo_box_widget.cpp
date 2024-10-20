#include "combo_box_widget.h"
#include <QHBoxLayout>

ComboBoxWidget::ComboBoxWidget(GlobalInfo& globalInfo, const std::string& name, std::vector<std::string>& elements) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	comboBox = new ComboBoxNoScrollWidget();
	for (const std::string& element : elements) {
		comboBox->addItem(QString::fromStdString(element));
	}
	layout()->addWidget(comboBox);
	layout()->setAlignment(comboBox, Qt::AlignmentFlag::AlignRight);
	
	connect(comboBox, &QComboBox::currentTextChanged, this, &ComboBoxWidget::onElementSelected);
}

void ComboBoxWidget::setElementByText(const std::string& text) {
	int index = comboBox->findText(QString::fromStdString(text));
	if (index != -1) {
		{
			const QSignalBlocker signalBlocker(comboBox);
			comboBox->setCurrentIndex(index);
		}
	}
}

std::string ComboBoxWidget::getElementText() {
	return comboBox->currentText().toStdString();
}

void ComboBoxWidget::onElementSelected(const QString& element) {
	emit elementSelected(element.toStdString());
}