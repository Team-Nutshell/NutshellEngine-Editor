#pragma once
#include "../common/global_info.h"
#include "info_label_widget.h"
#include <QWindow>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <string>

class ImageFileWidget : public QWidget {
	Q_OBJECT
public:
	ImageFileWidget(GlobalInfo& globalInfo);

	void setPath(const std::string& path);

private slots:
	void resizeEvent(QResizeEvent* event);

private:
	GlobalInfo& m_globalInfo;

	QPixmap m_pixmap;

public:
	QLabel* imageLabel;
	InfoLabelWidget* widthLabel;
	InfoLabelWidget* heightLabel;
};