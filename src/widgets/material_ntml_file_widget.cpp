#include "material_ntml_file_widget.h"
#include "separator_line.h"
#include "../common/asset_helper.h"
#include "../common/save_title_changer.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <fstream>

MaterialNtmlFileWidget::MaterialNtmlFileWidget(GlobalInfo& globalInfo, const std::string& materialFilePath) : m_globalInfo(globalInfo), m_materialFilePath(materialFilePath) {
	resize(640, 360);
	setWindowTitle("NutshellEngine - Material File - " + QString::fromStdString(materialFilePath));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_menuBar = new QMenuBar(this);
	m_fileMenu = m_menuBar->addMenu("File");
	m_fileSaveAction = m_fileMenu->addAction("Save", this, &MaterialNtmlFileWidget::save);
	m_fileSaveAction->setShortcut(QKeySequence::fromString("Ctrl+S"));

	setLayout(new QVBoxLayout());
	QMargins contentMargins = layout()->contentsMargins();
	contentMargins.setTop(contentMargins.top() + 10);
	layout()->setContentsMargins(contentMargins);
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	diffuseTextureImageWidget = new FileSelectorWidget(globalInfo, "Diffuse Image", "No image selected", "");
	layout()->addWidget(diffuseTextureImageWidget);
	diffuseTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Diffuse Image Sampler", "No image sampler selected", "");
	layout()->addWidget(diffuseTextureImageSamplerWidget);
	diffuseColorWidget = new ColorPickerWidget(globalInfo, "Diffuse Color", nml::vec3(0.5f, 0.5f, 0.5f));
	layout()->addWidget(diffuseColorWidget);
	opacityValueWidget = new ScalarWidget(globalInfo, "Opacity Value");
	opacityValueWidget->setValue(1.0f);
	opacityValueWidget->setMin(0.0f);
	opacityValueWidget->setMax(1.0f);
	layout()->addWidget(opacityValueWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	normalTextureImageWidget = new FileSelectorWidget(globalInfo, "Normal Image", "No image selected", "");
	layout()->addWidget(normalTextureImageWidget);
	normalTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Normal Image Sampler", "No image sampler selected", "");
	layout()->addWidget(normalTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	metalnessTextureImageWidget = new FileSelectorWidget(globalInfo, "Metalness Image", "No image selected", "");
	layout()->addWidget(metalnessTextureImageWidget);
	metalnessTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Metalness Image Sampler", "No image sampler selected", "");
	layout()->addWidget(metalnessTextureImageSamplerWidget);
	metalnessValueWidget = new ScalarWidget(globalInfo, "Metalness Value");
	metalnessValueWidget->setValue(0.5f);
	metalnessValueWidget->setMin(0.0f);
	metalnessValueWidget->setMax(1.0f);
	layout()->addWidget(metalnessValueWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	roughnessTextureImageWidget = new FileSelectorWidget(globalInfo, "Roughness Image", "No image selected", "");
	layout()->addWidget(roughnessTextureImageWidget);
	roughnessTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Roughness Image Sampler", "No image sampler selected", "");
	layout()->addWidget(roughnessTextureImageSamplerWidget);
	roughnessValueWidget = new ScalarWidget(globalInfo, "Roughness Value");
	roughnessValueWidget->setValue(0.5f);
	roughnessValueWidget->setMin(0.0f);
	roughnessValueWidget->setMax(1.0f);
	layout()->addWidget(roughnessValueWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	occlusionTextureImageWidget = new FileSelectorWidget(globalInfo, "Occlusion Image", "No image selected", "");
	layout()->addWidget(occlusionTextureImageWidget);
	occlusionTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Occlusion Image Sampler", "No image sampler selected", "");
	layout()->addWidget(occlusionTextureImageSamplerWidget);
	occlusionValueWidget = new ScalarWidget(globalInfo, "Occlusion Value");
	occlusionValueWidget->setValue(1.0f);
	occlusionValueWidget->setMin(0.0f);
	occlusionValueWidget->setMax(1.0f);
	layout()->addWidget(occlusionValueWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	emissiveTextureImageWidget = new FileSelectorWidget(globalInfo, "Emissive Image", "No image selected", "");
	layout()->addWidget(emissiveTextureImageWidget);
	emissiveTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Emissive Image Sampler", "No image sampler selected", "");
	layout()->addWidget(emissiveTextureImageSamplerWidget);
	emissiveColorWidget = new ColorPickerWidget(globalInfo, "Emissive Color", nml::vec3(0.0f, 0.0f, 0.0f));
	layout()->addWidget(emissiveColorWidget);
	emissiveFactorWidget = new ScalarWidget(globalInfo, "Emissive Factor");
	emissiveFactorWidget->setValue(1.0f);
	layout()->addWidget(emissiveFactorWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	alphaCutoffWidget = new ScalarWidget(globalInfo, "Alpha Cutoff");
	alphaCutoffWidget->setMin(0.0f);
	alphaCutoffWidget->setMax(1.0f);
	layout()->addWidget(alphaCutoffWidget);
	indexOfRefractionWidget = new ScalarWidget(globalInfo, "Index of Refraction");
	layout()->addWidget(indexOfRefractionWidget);

	connect(diffuseTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(diffuseTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(diffuseColorWidget, &ColorPickerWidget::colorChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(opacityValueWidget, &ScalarWidget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(normalTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(normalTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(metalnessTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(metalnessTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(metalnessValueWidget, &ScalarWidget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(roughnessTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(roughnessTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(roughnessValueWidget, &ScalarWidget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(occlusionTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(occlusionTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(occlusionValueWidget, &ScalarWidget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(emissiveTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(emissiveTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(emissiveFactorWidget, &ScalarWidget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(emissiveColorWidget, &ColorPickerWidget::colorChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(alphaCutoffWidget, &ScalarWidget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(indexOfRefractionWidget, &ScalarWidget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);

	std::fstream optionsFile(materialFilePath, std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + materialFilePath + "\" is not a valid JSON file.");
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + materialFilePath + "\" cannot be opened.");
		return;
	}

	optionsFile = std::fstream(materialFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("diffuse")) {
		if (j["diffuse"].contains("texture")) {
			if (j["diffuse"]["texture"].contains("imagePath")) {
				std::string imagePath = j["diffuse"]["texture"]["imagePath"];
				diffuseTextureImageWidget->setPath(imagePath);
			}
			if (j["diffuse"]["texture"].contains("imageSamplerPath")) {
				std::string imageSamplerPath = j["diffuse"]["texture"]["imageSamplerPath"];
				diffuseTextureImageSamplerWidget->setPath(imageSamplerPath);
			}
		}
		if (j["diffuse"].contains("color")) {
			nml::vec3 diffuseColor;
			diffuseColor.x = j["diffuse"]["color"][0];
			diffuseColor.y = j["diffuse"]["color"][1];
			diffuseColor.z = j["diffuse"]["color"][2];
			diffuseColorWidget->setColor(diffuseColor);
			opacityValueWidget->setValue(j["diffuse"]["color"][3]);
		}
	}
	if (j.contains("normal")) {
		if (j["normal"].contains("texture")) {
			if (j["normal"]["texture"].contains("imagePath")) {
				std::string imagePath = j["normal"]["texture"]["imagePath"];
				normalTextureImageWidget->setPath(imagePath);
			}
			if (j["normal"]["texture"].contains("imageSamplerPath")) {
				std::string imageSamplerPath = j["normal"]["texture"]["imageSamplerPath"];
				normalTextureImageSamplerWidget->setPath(imageSamplerPath);
			}
		}
	}
	if (j.contains("metalness")) {
		if (j["metalness"].contains("texture")) {
			if (j["metalness"]["texture"].contains("imagePath")) {
				std::string imagePath = j["metalness"]["texture"]["imagePath"];
				metalnessTextureImageWidget->setPath(imagePath);
			}
			if (j["metalness"]["texture"].contains("imageSamplerPath")) {
				std::string imageSamplerPath = j["metalness"]["texture"]["imageSamplerPath"];
				metalnessTextureImageSamplerWidget->setPath(imageSamplerPath);
			}
		}
		if (j["metalness"].contains("value")) {
			float metalnessValue = j["metalness"]["value"];
			metalnessValueWidget->setValue(metalnessValue);
		}
	}
	if (j.contains("roughness")) {
		if (j["roughness"].contains("texture")) {
			if (j["roughness"]["texture"].contains("imagePath")) {
				std::string imagePath = j["roughness"]["texture"]["imagePath"];
				roughnessTextureImageWidget->setPath(imagePath);
			}
			if (j["roughness"]["texture"].contains("imageSamplerPath")) {
				std::string imageSamplerPath = j["roughness"]["texture"]["imageSamplerPath"];
				roughnessTextureImageSamplerWidget->setPath(imageSamplerPath);
			}
		}
		if (j["roughness"].contains("value")) {
			float roughnessValue = j["roughness"]["value"];
			roughnessValueWidget->setValue(roughnessValue);
		}
	}
	if (j.contains("occlusion")) {
		if (j["occlusion"].contains("texture")) {
			if (j["occlusion"]["texture"].contains("imagePath")) {
				std::string imagePath = j["occlusion"]["texture"]["imagePath"];
				occlusionTextureImageWidget->setPath(imagePath);
			}
			if (j["occlusion"]["texture"].contains("imageSamplerPath")) {
				std::string imageSamplerPath = j["occlusion"]["texture"]["imageSamplerPath"];
				occlusionTextureImageSamplerWidget->setPath(imageSamplerPath);
			}
		}
		if (j["occlusion"].contains("value")) {
			float occlusionValue = j["occlusion"]["value"];
			occlusionValueWidget->setValue(occlusionValue);
		}
	}
	if (j.contains("emissive")) {
		if (j["emissive"].contains("texture")) {
			if (j["emissive"]["texture"].contains("imagePath")) {
				std::string imagePath = j["emissive"]["texture"]["imagePath"];
				emissiveTextureImageWidget->setPath(imagePath);
			}
			if (j["emissive"]["texture"].contains("imageSamplerPath")) {
				std::string imageSamplerPath = j["emissive"]["texture"]["imageSamplerPath"];
				emissiveTextureImageSamplerWidget->setPath(imageSamplerPath);
			}
		}
		if (j["emissive"].contains("color")) {
			nml::vec3 emissiveColor;
			emissiveColor.x = j["emissive"]["color"][0];
			emissiveColor.y = j["emissive"]["color"][1];
			emissiveColor.z = j["emissive"]["color"][2];
			emissiveColorWidget->setColor(emissiveColor);
		}
		if (j["emissive"].contains("factor")) {
			float emissiveFactor = j["emissive"]["factor"];
			emissiveFactorWidget->setValue(emissiveFactor);
		}
	}
	if (j.contains("alphaCutoff")) {
		float alphaCutoff = j["alphaCutoff"];
		alphaCutoffWidget->setValue(alphaCutoff);
	}
	if (j.contains("indexOfRefraction")) {
		float indexOfRefraction = j["indexOfRefraction"];
		indexOfRefractionWidget->setValue(indexOfRefraction);
	}
}

void MaterialNtmlFileWidget::onValueChanged() {
	QObject* senderWidget = sender();
	if (senderWidget == diffuseTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(diffuseTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		diffuseTextureImageWidget->setPath(path);
	}
	else if (senderWidget == diffuseTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(diffuseTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		diffuseTextureImageSamplerWidget->setPath(path);
	}
	else if (senderWidget == normalTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(normalTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		normalTextureImageWidget->setPath(path);
	}
	else if (senderWidget == normalTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(normalTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		normalTextureImageSamplerWidget->setPath(path);
	}
	else if (senderWidget == metalnessTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(metalnessTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		metalnessTextureImageWidget->setPath(path);
	}
	else if (senderWidget == metalnessTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(metalnessTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		metalnessTextureImageSamplerWidget->setPath(path);
	}
	else if (senderWidget == roughnessTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(roughnessTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		roughnessTextureImageWidget->setPath(path);
	}
	else if (senderWidget == roughnessTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(roughnessTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		roughnessTextureImageSamplerWidget->setPath(path);
	}
	else if (senderWidget == occlusionTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(occlusionTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		occlusionTextureImageWidget->setPath(path);
	}
	else if (senderWidget == occlusionTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(occlusionTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		occlusionTextureImageSamplerWidget->setPath(path);
	}
	else if (senderWidget == emissiveTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(emissiveTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		emissiveTextureImageWidget->setPath(path);
	}
	else if (senderWidget == emissiveTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(emissiveTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		emissiveTextureImageSamplerWidget->setPath(path);
	}

	SaveTitleChanger::change(this);
}

void MaterialNtmlFileWidget::save() {
	nlohmann::json j;
	if (!diffuseTextureImageWidget->getPath().empty()) {
		j["diffuse"]["texture"]["imagePath"] = diffuseTextureImageWidget->getPath();
	}
	if (!diffuseTextureImageSamplerWidget->getPath().empty()) {
		j["diffuse"]["texture"]["imageSamplerPath"] = diffuseTextureImageSamplerWidget->getPath();
	}
	nml::vec3 diffuseColor = diffuseColorWidget->getColor();
	float opacityValue = opacityValueWidget->getValue();
	j["diffuse"]["color"] = { diffuseColor.x, diffuseColor.y, diffuseColor.z, opacityValue };
	if (!normalTextureImageWidget->getPath().empty()) {
		j["normal"]["texture"]["imagePath"] = normalTextureImageWidget->getPath();
	}
	if (!normalTextureImageSamplerWidget->getPath().empty()) {
		j["normal"]["texture"]["imageSamplerPath"] = normalTextureImageSamplerWidget->getPath();
	}
	if (!metalnessTextureImageWidget->getPath().empty()) {
		j["metalness"]["texture"]["imagePath"] = metalnessTextureImageWidget->getPath();
	}
	if (!metalnessTextureImageSamplerWidget->getPath().empty()) {
		j["metalness"]["texture"]["imageSamplerPath"] = metalnessTextureImageSamplerWidget->getPath();
	}
	j["metalness"]["value"] = metalnessValueWidget->getValue();
	if (!roughnessTextureImageWidget->getPath().empty()) {
		j["roughness"]["texture"]["imagePath"] = roughnessTextureImageWidget->getPath();
	}
	if (!roughnessTextureImageSamplerWidget->getPath().empty()) {
		j["roughness"]["texture"]["imageSamplerPath"] = roughnessTextureImageSamplerWidget->getPath();
	}
	j["roughness"]["value"] = roughnessValueWidget->getValue();
	if (!occlusionTextureImageWidget->getPath().empty()) {
		j["occlusion"]["texture"]["imagePath"] = occlusionTextureImageWidget->getPath();
	}
	if (!occlusionTextureImageSamplerWidget->getPath().empty()) {
		j["occlusion"]["texture"]["imageSamplerPath"] = occlusionTextureImageSamplerWidget->getPath();
	}
	j["occlusion"]["value"] = occlusionValueWidget->getValue();
	if (!emissiveTextureImageWidget->getPath().empty()) {
		j["emissive"]["texture"]["imagePath"] = emissiveTextureImageWidget->getPath();
	}
	if (!emissiveTextureImageSamplerWidget->getPath().empty()) {
		j["emissive"]["texture"]["imageSamplerPath"] = emissiveTextureImageSamplerWidget->getPath();
	}
	nml::vec3 emissiveColor = emissiveColorWidget->getColor();
	j["emissive"]["color"] = { emissiveColor.x, emissiveColor.y, emissiveColor.z };
	j["emissive"]["factor"] = emissiveFactorWidget->getValue();
	j["alphaCutoff"] = alphaCutoffWidget->getValue();
	j["indexOfRefraction"] = indexOfRefractionWidget->getValue();

	std::fstream materialFile(m_materialFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		materialFile << "{\n}";
	}
	else {
		materialFile << j.dump(1, '\t');
	}
	materialFile.close();
	
	std::string materialPath = AssetHelper::absoluteToRelative(m_materialFilePath, m_globalInfo.projectDirectory);
	m_globalInfo.rendererResourceManager.loadMaterial(m_materialFilePath, materialPath);

	SaveTitleChanger::reset(this);
}
