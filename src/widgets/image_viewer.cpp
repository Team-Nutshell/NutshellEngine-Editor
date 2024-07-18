#include "image_viewer.h"
#include <QVBoxLayout>
#include <QImage>
#include <fstream>
#include <algorithm>

ImageViewer::ImageViewer(GlobalInfo& globalInfo, const std::string& imagePath, const QImage& image) : m_globalInfo(globalInfo) {
	setWindowTitle("NutshellEngine - Image Viewer - " + QString::fromStdString(imagePath));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);
	
	m_pixmap = QPixmap();
	m_pixmap.convertFromImage(image);
	setFixedSize(std::max(m_pixmap.width() + 20, 100), std::max(m_pixmap.height() + 20, 100));

	setLayout(new QVBoxLayout());
	imageLabel = new QLabel();
	imageLabel->setPixmap(m_pixmap);
	layout()->addWidget(imageLabel);
}
