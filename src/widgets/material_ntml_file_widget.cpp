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
	layout()->addWidget(new SeparatorLine(globalInfo));
	roughnessTextureImageWidget = new FileSelectorWidget(globalInfo, "Roughness Image", "No image selected", "");
	layout()->addWidget(roughnessTextureImageWidget);
	roughnessTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Roughness Image Sampler", "No image sampler selected", "");
	layout()->addWidget(roughnessTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	occlusionTextureImageWidget = new FileSelectorWidget(globalInfo, "Occlusion Image", "No image selected", "");
	layout()->addWidget(occlusionTextureImageWidget);
	occlusionTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Occlusion Image Sampler", "No image sampler selected", "");
	layout()->addWidget(occlusionTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	emissiveTextureImageWidget = new FileSelectorWidget(globalInfo, "Emissive Image", "No image selected", "");
	layout()->addWidget(emissiveTextureImageWidget);
	emissiveTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Emissive Image Sampler", "No image sampler selected", "");
	layout()->addWidget(emissiveTextureImageSamplerWidget);
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
	connect(normalTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(normalTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(metalnessTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(metalnessTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(roughnessTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(roughnessTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(occlusionTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(occlusionTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(emissiveTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(emissiveTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(emissiveFactorWidget, &ScalarWidget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
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

	if (j.contains("diffuseTexture")) {
		if (j["diffuseTexture"].contains("imagePath")) {
			std::string imagePath = j["diffuseTexture"]["imagePath"];
			diffuseTextureImageWidget->setPath(imagePath);
		}
		if (j["diffuseTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["diffuseTexture"]["imageSamplerPath"];
			diffuseTextureImageSamplerWidget->setPath(imageSamplerPath);
		}
	}
	if (j.contains("normalTexture")) {
		if (j["normalTexture"].contains("imagePath")) {
			std::string imagePath = j["normalTexture"]["imagePath"];
			normalTextureImageWidget->setPath(imagePath);
		}
		if (j["normalTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["normalTexture"]["imageSamplerPath"];
			normalTextureImageSamplerWidget->setPath(imageSamplerPath);
		}
	}
	if (j.contains("metalnessTexture")) {
		if (j["metalnessTexture"].contains("imagePath")) {
			std::string imagePath = j["metalnessTexture"]["imagePath"];
			metalnessTextureImageWidget->setPath(imagePath);
		}
		if (j["metalnessTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["metalnessTexture"]["imageSamplerPath"];
			metalnessTextureImageSamplerWidget->setPath(imageSamplerPath);
		}
	}
	if (j.contains("roughnessTexture")) {
		if (j["roughnessTexture"].contains("imagePath")) {
			std::string imagePath = j["roughnessTexture"]["imagePath"];
			roughnessTextureImageWidget->setPath(imagePath);
		}
		if (j["roughnessTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["roughnessTexture"]["imageSamplerPath"];
			roughnessTextureImageSamplerWidget->setPath(imageSamplerPath);
		}
	}
	if (j.contains("occlusionTexture")) {
		if (j["occlusionTexture"].contains("imagePath")) {
			std::string imagePath = j["occlusionTexture"]["imagePath"];
			occlusionTextureImageWidget->setPath(imagePath);
		}
		if (j["occlusionTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["occlusionTexture"]["imageSamplerPath"];
			occlusionTextureImageSamplerWidget->setPath(imageSamplerPath);
		}
	}
	if (j.contains("emissiveTexture")) {
		if (j["emissiveTexture"].contains("imagePath")) {
			std::string imagePath = j["emissiveTexture"]["imagePath"];
			emissiveTextureImageWidget->setPath(imagePath);
		}
		if (j["emissiveTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["emissiveTexture"]["imageSamplerPath"];
			emissiveTextureImageSamplerWidget->setPath(imageSamplerPath);
		}
	}
	if (j.contains("emissiveFactor")) {
		float emissiveFactor = j["emissiveFactor"];
		emissiveFactorWidget->setValue(emissiveFactor);
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
		j["diffuseTexture"]["imagePath"] = diffuseTextureImageWidget->getPath();
	}
	if (!diffuseTextureImageSamplerWidget->getPath().empty()) {
		j["diffuseTexture"]["imageSamplerPath"] = diffuseTextureImageSamplerWidget->getPath();
	}
	if (!normalTextureImageWidget->getPath().empty()) {
		j["normalTexture"]["imagePath"] = normalTextureImageWidget->getPath();
	}
	if (!normalTextureImageSamplerWidget->getPath().empty()) {
		j["normalTexture"]["imageSamplerPath"] = normalTextureImageSamplerWidget->getPath();
	}
	if (!metalnessTextureImageWidget->getPath().empty()) {
		j["metalnessTexture"]["imagePath"] = metalnessTextureImageWidget->getPath();
	}
	if (!metalnessTextureImageSamplerWidget->getPath().empty()) {
		j["metalnessTexture"]["imageSamplerPath"] = metalnessTextureImageSamplerWidget->getPath();
	}
	if (!roughnessTextureImageWidget->getPath().empty()) {
		j["roughnessTexture"]["imagePath"] = roughnessTextureImageWidget->getPath();
	}
	if (!roughnessTextureImageSamplerWidget->getPath().empty()) {
		j["roughnessTexture"]["imageSamplerPath"] = roughnessTextureImageSamplerWidget->getPath();
	}
	if (!occlusionTextureImageWidget->getPath().empty()) {
		j["occlusionTexture"]["imagePath"] = occlusionTextureImageWidget->getPath();
	}
	if (!occlusionTextureImageSamplerWidget->getPath().empty()) {
		j["occlusionTexture"]["imageSamplerPath"] = occlusionTextureImageSamplerWidget->getPath();
	}
	if (!emissiveTextureImageWidget->getPath().empty()) {
		j["emissiveTexture"]["imagePath"] = emissiveTextureImageWidget->getPath();
	}
	if (!emissiveTextureImageSamplerWidget->getPath().empty()) {
		j["emissiveTexture"]["imageSamplerPath"] = emissiveTextureImageSamplerWidget->getPath();
	}
	j["emissiveFactor"] = emissiveFactorWidget->getValue();
	j["alphaCutoff"] = alphaCutoffWidget->getValue();
	j["indexOfRefraction"] = indexOfRefractionWidget->getValue();

	std::fstream materialFile(m_materialFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		materialFile << "{\n}";
	}
	else {
		materialFile << j.dump(1, '\t');
	}

	SaveTitleChanger::reset(this);
}
