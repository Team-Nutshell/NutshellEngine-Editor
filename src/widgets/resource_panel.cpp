#include "resource_panel.h"
#include <QVBoxLayout>

ResourcePanel::ResourcePanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(2, 2, 2, 0);
	assetsLabel = new QLabel("Assets");
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/assets/")) {
		assetsLabel->setText("Assets (<i>assets/</i>)");
	}
	else {
		assetsLabel->setText("Assets (<i>\"assets/\" directory missing</i>)");
	}
	layout()->addWidget(assetsLabel);
	assetList = new AssetList(m_globalInfo);
	layout()->addWidget(assetList);

	connect(assetList, &AssetList::directoryChanged, this, &ResourcePanel::onDirectoryChanged);
}

void ResourcePanel::onDirectoryChanged(const std::string& directory) {
	if (!directory.empty()) {
		assetsLabel->setText("Assets (<i>" + QString::fromStdString(directory) + "/</i>)");
	}
	else {
		assetsLabel->setText("Assets (<i>\"assets/\" directory missing</i>)");
	}
}