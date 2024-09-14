#include "asset_info_name_widget.h"
#include <regex>

AssetInfoNameWidget::AssetInfoNameWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	connect(this, &QLineEdit::editingFinished, this, &AssetInfoNameWidget::onEditingFinished);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectAssetSignal, this, &AssetInfoNameWidget::onAssetSelected);
}

void AssetInfoNameWidget::onAssetSelected(const std::string& path) {
	std::string filename = path;
	if (filename.back() == '/') {
		filename.pop_back();
	}
	size_t lastSlashPos = filename.find_last_of("/");
	if (lastSlashPos != std::string::npos) {
		filename = filename.substr(lastSlashPos + 1);
		m_directory = path.substr(0, lastSlashPos);
	}

	setText(QString::fromStdString(filename));
	m_previousName = filename;
}

void AssetInfoNameWidget::onEditingFinished() {
	std::string newName = text().toStdString();
	if (m_previousName != newName) {
		const std::regex validFilenameRegex(R"(^[a-zA-Z0-9._ -]+$)");
		if (!std::regex_search(newName, validFilenameRegex)) {
			setText(QString::fromStdString(m_previousName));
			return;
		}

		if (newName.empty()) {
			setText(QString::fromStdString(m_previousName));
			return;
		}

		if (std::filesystem::exists(m_directory + "/" + newName)) {
			setText(QString::fromStdString(m_previousName));
			return;
		}

		if (std::filesystem::exists(m_directory + "/" + m_previousName)) {
			std::filesystem::rename(m_directory + "/" + m_previousName, m_directory + "/" + newName);
			emit m_globalInfo.signalEmitter.renameFileSignal(m_directory + "/" + m_previousName, m_directory + "/" + newName);
		}

		m_previousName = newName;
	}
}