#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>

class ImageSizeWidget : public QWidget {
	Q_OBJECT
public:
	ImageSizeWidget(GlobalInfo& globalInfo, const std::string& name);

	void setWidth(uint32_t width);
	uint32_t getWidth();

	void setHeight(uint32_t height);
	uint32_t getHeight();

signals:
	void valueChanged(uint32_t width, uint32_t height);
	void widthChanged(uint32_t);
	void heightChanged(uint32_t);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;

	uint32_t m_width = 1;
	uint32_t m_height = 1;

public:
	QLabel* nameLabel;
	QLineEdit* widthLineEdit;
	QLabel* xLabel;
	QLineEdit* heightLineEdit;
};