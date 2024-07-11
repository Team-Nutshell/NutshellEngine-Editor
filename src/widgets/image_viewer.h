#pragma once
#include "../common/global_info.h"
#include <QWindow>
#include <QLabel>
#include <QPixmap>
#include <string>

class ImageViewer : public QWidget {
	Q_OBJECT
public:
	ImageViewer(GlobalInfo& globalInfo, const std::string& imagePath, const QImage& image);

private:
	GlobalInfo& m_globalInfo;

	QPixmap m_pixmap;

public:
	QLabel* imageLabel;
};