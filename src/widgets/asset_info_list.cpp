#include "asset_info_list.h"
#include "json_model.h"
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
	jsonFileWidget = new QTreeView();
	jsonFileWidget->hide();
	layout()->addWidget(jsonFileWidget);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectAssetSignal, this, &AssetInfoList::onAssetSelected);
}

void AssetInfoList::onAssetSelected(const std::string& path) {
	if (!path.empty() && !std::filesystem::is_directory(path)) {
		currentAssetPath = path;
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
				jsonFileWidget->hide();
			}
			else if (extension == "ntmd") {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->setPath(path);
				modelNtmdFileWidget->show();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->hide();
				imageFileWidget->hide();
				jsonFileWidget->hide();
			}
			else if (extension == "ntop") {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->setPath(path);
				optionsNtopFileWidget->show();
				samplerNtspFileWidget->hide();
				imageFileWidget->hide();
				jsonFileWidget->hide();
			}
			else if (extension == "ntsp") {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->setPath(path);
				samplerNtspFileWidget->show();
				imageFileWidget->hide();
				jsonFileWidget->hide();
			}
			else if ((extension == "jpg") || (extension == "jpeg") || (extension == "png") || (extension == "ntim")) {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->hide();
				imageFileWidget->setPath(path);
				imageFileWidget->show();
				jsonFileWidget->hide();
			}
			else if (extension == "json") {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->hide();
				imageFileWidget->hide();
				jsonFileWidget->setModel(new JSONModel(m_globalInfo, path));
				jsonFileWidget->model()->setHeaderData(0, Qt::Orientation::Horizontal, QString::fromStdString(m_globalInfo.localization.getString("assets_json_key")));
				jsonFileWidget->model()->setHeaderData(1, Qt::Orientation::Horizontal, QString::fromStdString(m_globalInfo.localization.getString("assets_json_value")));
				jsonFileWidget->show();
			}
			else {
				materialNtmlFileWidget->hide();
				modelNtmdFileWidget->hide();
				optionsNtopFileWidget->hide();
				samplerNtspFileWidget->hide();
				imageFileWidget->hide();
				jsonFileWidget->hide();
			}
		}
	}
	else {
		materialNtmlFileWidget->hide();
		modelNtmdFileWidget->hide();
		optionsNtopFileWidget->hide();
		samplerNtspFileWidget->hide();
		imageFileWidget->hide();
		jsonFileWidget->hide();
	}
}
