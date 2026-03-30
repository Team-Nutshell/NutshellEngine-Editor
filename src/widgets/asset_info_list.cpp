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
	shaderFileWidget = new ShaderFileWidget(m_globalInfo);
	shaderFileWidget->hide();
	layout()->addWidget(shaderFileWidget);
	projectNtpjFileWidget = new ProjectNtpjFileWidget(m_globalInfo);
	projectNtpjFileWidget->hide();
	layout()->addWidget(projectNtpjFileWidget);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectAssetSignal, this, &AssetInfoList::onAssetSelected);
}

void AssetInfoList::hideAllWidgets() {
	materialNtmlFileWidget->hide();
	modelNtmdFileWidget->hide();
	optionsNtopFileWidget->hide();
	samplerNtspFileWidget->hide();
	imageFileWidget->hide();
	jsonFileWidget->hide();
	textFileWidget->hide();
	shaderFileWidget->hide();
	projectNtpjFileWidget->hide();
}

void AssetInfoList::onAssetSelected(const std::string& path) {
	if (!path.empty() && !std::filesystem::is_directory(path)) {
		currentAssetPath = path;

		std::string extension;
		size_t lastDot = path.rfind('.');
		if (lastDot != std::string::npos) {
			if (extension != "ntmd") {
				extension = path.substr(lastDot + 1);
			}
		}

		AssetHelper::FileType fileType = AssetHelper::fileType(path);
		if (fileType == AssetHelper::FileType::Material) {
			hideAllWidgets();
			materialNtmlFileWidget->setPath(path);
			materialNtmlFileWidget->show();
		}
		else if ((fileType == AssetHelper::FileType::Model) && (extension == "ntmd")) {
			hideAllWidgets();
			modelNtmdFileWidget->setPath(path);
			modelNtmdFileWidget->show();
		}
		else if (fileType == AssetHelper::FileType::Options) {
			hideAllWidgets();
			optionsNtopFileWidget->setPath(path);
			optionsNtopFileWidget->show();
		}
		else if (fileType == AssetHelper::FileType::ImageSampler) {
			hideAllWidgets();
			samplerNtspFileWidget->setPath(path);
			samplerNtspFileWidget->show();
		}
		else if ((fileType == AssetHelper::FileType::Image) ||
			(fileType == AssetHelper::FileType::Icon)) {
			hideAllWidgets();
			imageFileWidget->setPath(path);
			imageFileWidget->show();
		}
		else if (fileType == AssetHelper::FileType::Json) {
			hideAllWidgets();
			jsonFileWidget->setModel(new JSONModel(m_globalInfo, path));
			jsonFileWidget->model()->setHeaderData(0, Qt::Orientation::Horizontal, QString::fromStdString(m_globalInfo.localization.getString("assets_json_key")));
			jsonFileWidget->model()->setHeaderData(1, Qt::Orientation::Horizontal, QString::fromStdString(m_globalInfo.localization.getString("assets_json_value")));
			jsonFileWidget->show();
		}
		else if (fileType == AssetHelper::FileType::Text) {
			hideAllWidgets();
			textFileWidget->setPath(path);
			textFileWidget->show();
		}
		else if (fileType == AssetHelper::FileType::FragmentShader) {
			hideAllWidgets();
			shaderFileWidget->setPath(path);
			shaderFileWidget->show();
		}
		else if (extension == "ntpj") {
			hideAllWidgets();
			projectNtpjFileWidget->show();
		}
		else {
			hideAllWidgets();
		}
	}
	else {
		hideAllWidgets();
	}
}
