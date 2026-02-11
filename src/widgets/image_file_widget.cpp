#include "image_file_widget.h"
#include "main_window.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QImage>
#include <fstream>
#include <algorithm>

ImageFileWidget::ImageFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_image_file"))));
	imageLabel = new QLabel();
	layout()->addWidget(imageLabel);
	widthLabel = new InfoLabelWidget(globalInfo, m_globalInfo.localization.getString("assets_image_width"));
	layout()->addWidget(widthLabel);
	heightLabel = new InfoLabelWidget(globalInfo, m_globalInfo.localization.getString("assets_image_height"));
	layout()->addWidget(heightLabel);
}

void ImageFileWidget::setPath(const std::string& path) {
	QImage image;
	size_t lastDot = path.rfind('.');
	std::string extension = path.substr(lastDot + 1);
	if ((extension == "jpg") ||
		(extension == "jpeg") ||
		(extension == "png") ||
		(extension == "ico")) {
		image = QImage(QString::fromStdString(path));
	}
	else if (extension == "ntim") {
		int width = 1;
		int height = 1;

		std::fstream imageFile(path);
		if (imageFile.is_open()) {
			if (!nlohmann::json::accept(imageFile)) {
				m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_is_not_valid_json", { path }));
				return;
			}
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_cannot_be_opened", { path }));
			return;
		}

		imageFile = std::fstream(path, std::ios::in);

		nlohmann::json j = nlohmann::json::parse(imageFile);

		if (j.contains("width")) {
			width = static_cast<uint32_t>(j["width"]);
		}

		if (j.contains("height")) {
			height = static_cast<uint32_t>(j["height"]);
		}

		if (j.contains("data")) {
			for (size_t i = 0; i < j["data"].size(); i++) {
				m_pixelData.push_back(static_cast<uint8_t>(j["data"][i]));
			}
		}
		image = QImage(m_pixelData.data(), width, height, QImage::Format_RGBA8888);
	}

	m_pixmap = QPixmap();
	m_pixmap.convertFromImage(image);
  
	imageLabel->setPixmap(m_pixmap.scaledToWidth(m_globalInfo.mainWindow->infoPanel->width() - 20, Qt::TransformationMode::FastTransformation));
	widthLabel->setText(std::to_string(image.width()));
	heightLabel->setText(std::to_string(image.height()));
}

void ImageFileWidget::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);
	imageLabel->setPixmap(m_pixmap.scaledToWidth(m_globalInfo.mainWindow->infoPanel->width() - 20, Qt::TransformationMode::FastTransformation));
}