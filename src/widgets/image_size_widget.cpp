#include "image_size_widget.h"
#include <QHBoxLayout>
#include <QIntValidator>
#include <cstdlib>

ImageSizeWidget::ImageSizeWidget(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	QIntValidator* intValidator = new QIntValidator();

	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	nameLabel = new QLabel(QString::fromStdString(name));
	layout()->addWidget(nameLabel);
	static_cast<QHBoxLayout*>(layout())->addStretch();
	widthLineEdit = new QLineEdit("1");
	widthLineEdit->setValidator(intValidator);
	layout()->addWidget(widthLineEdit);
	layout()->setAlignment(widthLineEdit, Qt::AlignmentFlag::AlignRight);
	xLabel = new QLabel("x");
	layout()->addWidget(xLabel);
	layout()->setAlignment(xLabel, Qt::AlignmentFlag::AlignRight);
	heightLineEdit = new QLineEdit("1");
	heightLineEdit->setValidator(intValidator);
	layout()->addWidget(heightLineEdit);
	layout()->setAlignment(heightLineEdit, Qt::AlignmentFlag::AlignRight);

	connect(widthLineEdit, &QLineEdit::editingFinished, this, &ImageSizeWidget::onEditingFinished);
	connect(heightLineEdit, &QLineEdit::editingFinished, this, &ImageSizeWidget::onEditingFinished);
}

void ImageSizeWidget::setWidth(uint32_t width) {
	width = std::max(static_cast<uint32_t>(1), width);
	if (m_width != width) {
		m_width = width;
		widthLineEdit->setText(QString::number(m_width));
	}
}

uint32_t ImageSizeWidget::getWidth() {
	return m_width;
}

void ImageSizeWidget::setHeight(uint32_t height) {
	height = std::max(static_cast<uint32_t>(1), height);
	if (m_height != height) {
		m_height = height;
		heightLineEdit->setText(QString::number(m_height));
	}
}

uint32_t ImageSizeWidget::getHeight() {
	return m_height;
}

void ImageSizeWidget::onEditingFinished() {
	int newWidth = std::atoi(widthLineEdit->text().toStdString().c_str());
	newWidth = std::max(1, newWidth);
	int newHeight = std::atoi(heightLineEdit->text().toStdString().c_str());
	newHeight = std::max(1, newHeight);
	QObject* senderWidget = sender();
	if (senderWidget == widthLineEdit) {
		if (m_width != static_cast<uint32_t>(newWidth)) {
			emit widthChanged(static_cast<uint32_t>(newWidth));
		}
		else {
			widthLineEdit->setText(QString::number(newWidth));
		}
	}
	else if (senderWidget == heightLineEdit) {
		if (m_height != static_cast<uint32_t>(newHeight)) {
			emit heightChanged(static_cast<uint32_t>(newHeight));
		}
		else {
			heightLineEdit->setText(QString::number(newHeight));
		}
	}
	if ((m_width != static_cast<uint32_t>(newWidth)) || (m_height != static_cast<uint32_t>(newHeight))) {
		setWidth(static_cast<uint32_t>(newWidth));
		setHeight(static_cast<uint32_t>(newHeight));
		emit valueChanged(m_width, m_height);
	}
	static_cast<QWidget*>(senderWidget)->clearFocus();
}