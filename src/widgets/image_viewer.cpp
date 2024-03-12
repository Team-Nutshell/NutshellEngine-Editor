#include "image_viewer.h"
#include <QVBoxLayout>

ImageViewer::ImageViewer(GlobalInfo& globalInfo, const std::string& imagePath) : m_globalInfo(globalInfo) {
	setWindowTitle("NutshellEngine - Image Viewer - " + QString::fromStdString(imagePath));
	setWindowIcon(QIcon("assets/icon.png"));

	m_image = QPixmap(QString::fromStdString(imagePath));
	setFixedSize(m_image.width() + 20, m_image.height() + 20);

	setLayout(new QVBoxLayout());
	imageLabel = new QLabel();
	imageLabel->setPixmap(m_image);
	layout()->addWidget(imageLabel);
}
