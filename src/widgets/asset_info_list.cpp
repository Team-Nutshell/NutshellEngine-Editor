#include "asset_info_list.h"
#include "json_model.h"
#include "../common/asset_helper.h"
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
	textFileWidget = new TextFileWidget(m_globalInfo);
	textFileWidget->hide();
	layout()->addWidget(textFileWidget);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectAssetSignal, this, &AssetInfoList::onAssetSelected);
}

void AssetInfoList::onAssetSelected(const std::string& path) {
	if (!path.empty() && !std::filesystem::is_directory(path)) {
		currentAssetPath = path;
		AssetHelper::FileType fileType = AssetHelper::fileType(path);
		if (fileType == AssetHelper::FileType::Material) {
			materialNtmlFileWidget->setPath(path);
			materialNtmlFileWidget->show();
			modelNtmdFileWidget->hide();
			optionsNtopFileWidget->hide();
			samplerNtspFileWidget->hide();
			imageFileWidget->hide();
			jsonFileWidget->hide();
			textFileWidget->hide();
		}
		else if (fileType == AssetHelper::FileType::Model) {
			materialNtmlFileWidget->hide();
			modelNtmdFileWidget->setPath(path);
			modelNtmdFileWidget->show();
			optionsNtopFileWidget->hide();
			samplerNtspFileWidget->hide();
			imageFileWidget->hide();
			jsonFileWidget->hide();
			textFileWidget->hide();
		}
		else if (fileType == AssetHelper::FileType::Options) {
			materialNtmlFileWidget->hide();
			modelNtmdFileWidget->hide();
			optionsNtopFileWidget->setPath(path);
			optionsNtopFileWidget->show();
			samplerNtspFileWidget->hide();
			imageFileWidget->hide();
			jsonFileWidget->hide();
			textFileWidget->hide();
		}
		else if (fileType == AssetHelper::FileType::ImageSampler) {
			materialNtmlFileWidget->hide();
			modelNtmdFileWidget->hide();
			optionsNtopFileWidget->hide();
			samplerNtspFileWidget->setPath(path);
			samplerNtspFileWidget->show();
			imageFileWidget->hide();
			jsonFileWidget->hide();
			textFileWidget->hide();
		}
		else if (fileType == AssetHelper::FileType::Image) {
			materialNtmlFileWidget->hide();
			modelNtmdFileWidget->hide();
			optionsNtopFileWidget->hide();
			samplerNtspFileWidget->hide();
			imageFileWidget->setPath(path);
			imageFileWidget->show();
			jsonFileWidget->hide();
			textFileWidget->hide();
		}
		else if (fileType == AssetHelper::FileType::Json) {
			materialNtmlFileWidget->hide();
			modelNtmdFileWidget->hide();
			optionsNtopFileWidget->hide();
			samplerNtspFileWidget->hide();
			imageFileWidget->hide();
			jsonFileWidget->setModel(new JSONModel(m_globalInfo, path));
			jsonFileWidget->model()->setHeaderData(0, Qt::Orientation::Horizontal, QString::fromStdString(m_globalInfo.localization.getString("assets_json_key")));
			jsonFileWidget->model()->setHeaderData(1, Qt::Orientation::Horizontal, QString::fromStdString(m_globalInfo.localization.getString("assets_json_value")));
			jsonFileWidget->show();
			textFileWidget->hide();
		}
		else if (fileType == AssetHelper::FileType::Text) {
			materialNtmlFileWidget->hide();
			modelNtmdFileWidget->hide();
			optionsNtopFileWidget->hide();
			samplerNtspFileWidget->hide();
			imageFileWidget->hide();
			jsonFileWidget->hide();
			textFileWidget->setPath(path);
			textFileWidget->show();
		}
		else {
			materialNtmlFileWidget->hide();
			modelNtmdFileWidget->hide();
			optionsNtopFileWidget->hide();
			samplerNtspFileWidget->hide();
			imageFileWidget->hide();
			jsonFileWidget->hide();
			textFileWidget->hide();
		}
	}
	else {
		materialNtmlFileWidget->hide();
		modelNtmdFileWidget->hide();
		optionsNtopFileWidget->hide();
		samplerNtspFileWidget->hide();
		imageFileWidget->hide();
		jsonFileWidget->hide();
		textFileWidget->hide();
	}
}
