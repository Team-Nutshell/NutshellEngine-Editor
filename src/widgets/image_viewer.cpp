#include "image_viewer.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QImage>
#include <fstream>
#include <algorithm>

ImageViewer::ImageViewer(GlobalInfo& globalInfo, const std::string& imagePath) : m_globalInfo(globalInfo) {
	setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("assets_image_viewer")) + " - " + QString::fromStdString(imagePath));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	QImage image;
	size_t lastDot = imagePath.rfind('.');
	std::string extension = imagePath.substr(lastDot + 1);
	if ((extension == "jpg") || (extension == "jpeg") || (extension == "png")) {
		image = QImage(QString::fromStdString(imagePath));
	}
	else if (extension == "ntim") {
		int width = 1;
		int height = 1;
		std::vector<uint8_t> pixelData;

		std::fstream imageFile(imagePath);
		if (imageFile.is_open()) {
			if (!nlohmann::json::accept(imageFile)) {
				m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_is_not_valid_json", { imagePath }));
				return;
			}
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_cannot_be_opened", { imagePath }));
			return;
		}

		imageFile = std::fstream(imagePath, std::ios::in);

		nlohmann::json j = nlohmann::json::parse(imageFile);

		if (j.contains("width")) {
			width = static_cast<uint32_t>(j["width"]);
		}

		if (j.contains("height")) {
			height = static_cast<uint32_t>(j["height"]);
		}

		if (j.contains("data")) {
			for (size_t i = 0; i < j["data"].size(); i++) {
				pixelData.push_back(static_cast<uint8_t>(j["data"][i]));
			}
		}
		image = QImage(pixelData.data(), width, height, QImage::Format_RGBA8888);
	}
	
	m_pixmap = QPixmap();
	m_pixmap.convertFromImage(image);
	setFixedSize(std::max(m_pixmap.width() + 20, 100), std::max(m_pixmap.height() + 20, 100));

	setLayout(new QVBoxLayout());
	imageLabel = new QLabel();
	imageLabel->setPixmap(m_pixmap);
	layout()->addWidget(imageLabel);
}
