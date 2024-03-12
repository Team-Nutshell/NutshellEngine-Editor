#pragma once
#include "../common/common.h"
#include <QWindow>
#include <QLabel>
#include <QPixmap>
#include <string>

class ImageViewer : public QWidget {
	Q_OBJECT
public:
	ImageViewer(GlobalInfo& globalInfo, const std::string& imagePath);

private:
	GlobalInfo& m_globalInfo;

	QPixmap m_image;

public:
	QLabel* imageLabel;
};