#include "resource_panel.h"
#include <QVBoxLayout>

ResourcePanel::ResourcePanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(2, 2, 2, 0);
	assetsLabel = new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_list_assets")));
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/assets/")) {
		assetsLabel->setText(QString::fromStdString(m_globalInfo.localization.getString("assets_list_assets")) + " (<i>assets/</i>)");
	}
	else {
		assetsLabel->setText(QString::fromStdString(m_globalInfo.localization.getString("assets_list_assets")) + " (<i>\"assets/\"" + QString::fromStdString(m_globalInfo.localization.getString("assets_list_assets_directory_missing")) + "</i>)");
	}
	layout()->addWidget(assetsLabel);
	assetList = new AssetList(m_globalInfo);
	layout()->addWidget(assetList);

	connect(assetList, &AssetList::directoryChanged, this, &ResourcePanel::onDirectoryChanged);
}

void ResourcePanel::onDirectoryChanged(const std::string& directory) {
	if (!directory.empty()) {
		assetsLabel->setText(QString::fromStdString(m_globalInfo.localization.getString("assets_list_assets")) + " (<i>" + QString::fromStdString(directory) + "/</i>)");
	}
	else {
		assetsLabel->setText(QString::fromStdString(m_globalInfo.localization.getString("assets_list_assets")) + " (<i>\"assets/\"" + QString::fromStdString(m_globalInfo.localization.getString("assets_list_assets_directory_missing")) + "</i>)");
	}
}