#include "asset_info_list.h"
#include <QVBoxLayout>

AssetInfoList::AssetInfoList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	materialNtmlFileWidget = new MaterialNtmlFileWidget(m_globalInfo);
	materialNtmlFileWidget->hide();
	layout()->addWidget(materialNtmlFileWidget);
	modelNtmdFileWidget = new ModelNtmdFileWidget(m_globalInfo);
	modelNtmdFileWidget->hide();
	layout()->addWidget(modelNtmdFileWidget);
	optionsNtopFileWidget = new OptionsNtopFileWidget(m_globalInfo);
	optionsNtopFileWidget->hide();
	layout()->addWidget(optionsNtopFileWidget);
	samplerNtspFileWidget = new SamplerNtspFileWidget(m_globalInfo);
	samplerNtspFileWidget->hide();
	layout()->addWidget(samplerNtspFileWidget);
	imageFileWidget = new ImageFileWidget(m_globalInfo);
	imageFileWidget->hide();
	layout()->addWidget(imageFileWidget);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectAssetSignal, this, &AssetInfoList::onAssetSelected);
}

void AssetInfoList::onAssetSelected(const std::string& path) {
	if (!path.empty() && !std::filesystem::is_directory(path)) {
		size_t lastDot = path.rfind('.');
		if (lastDot != std::string::npos) {
			std::string extension = path.substr(lastDot + 1);

			if (extension == "ntml") {
				materialNtmlFileWidget->setPath(path);
				materialNtmlFileWidget->show();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->hide();
				imageFileWidget->hide();
			}
			else if (extension == "ntmd") {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->setPath(path);
				modelNtmdFileWidget->show();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->hide();
				imageFileWidget->hide();
			}
			else if (extension == "ntop") {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->setPath(path);
				optionsNtopFileWidget->show();
				samplerNtspFileWidget->hide();
				imageFileWidget->hide();
			}
			else if (extension == "ntsp") {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->setPath(path);
				samplerNtspFileWidget->show();
				imageFileWidget->hide();
			}
			else if ((extension == "jpg") || (extension == "jpeg") || (extension == "png") || (extension == "ntim")) {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->hide();
				imageFileWidget->setPath(path);
				imageFileWidget->show();
			}
			else {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->hide();
				imageFileWidget->hide();
			}
		}
	}
	else {
		materialNtmlFileWidget->hide();
		modelNtmdFileWidget->hide();
		optionsNtopFileWidget->hide();
		samplerNtspFileWidget->hide();
		imageFileWidget->hide();
	}
}
