#include "material_ntml_file_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include "../undo_commands/select_asset_entities_command.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <fstream>

MaterialNtmlFileWidget::MaterialNtmlFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(2, 0, 2, 0);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_material_file"))));
	diffuseTextureImageWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_diffuse_image"), m_globalInfo.localization.getString("no_image_selected"), "");
	layout()->addWidget(diffuseTextureImageWidget);
	diffuseTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_diffuse_image_sampler"), m_globalInfo.localization.getString("no_image_sampler_selected"), "");
	layout()->addWidget(diffuseTextureImageSamplerWidget);
	diffuseColorWidget = new ColorPickerWidget(globalInfo, m_globalInfo.localization.getString("assets_material_diffuse_color"), nml::vec3(0.5f, 0.5f, 0.5f));
	layout()->addWidget(diffuseColorWidget);
	opacityValueWidget = new ScalarWidget(globalInfo, m_globalInfo.localization.getString("assets_material_opacity_value"));
	opacityValueWidget->setValue(1.0f);
	opacityValueWidget->setMin(0.0f);
	opacityValueWidget->setMax(1.0f);
	layout()->addWidget(opacityValueWidget);
	layout()->addWidget(new SeparatorLine());
	normalTextureImageWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_normal_image"), m_globalInfo.localization.getString("no_image_selected"), "");
	layout()->addWidget(normalTextureImageWidget);
	normalTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_normal_image_sampler"), m_globalInfo.localization.getString("no_image_sampler_selected"), "");
	layout()->addWidget(normalTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine());
	metalnessTextureImageWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_metalness_image"), m_globalInfo.localization.getString("no_image_selected"), "");
	layout()->addWidget(metalnessTextureImageWidget);
	metalnessTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_metalness_image_sampler"), m_globalInfo.localization.getString("no_image_sampler_selected"), "");
	layout()->addWidget(metalnessTextureImageSamplerWidget);
	metalnessValueWidget = new ScalarWidget(globalInfo, m_globalInfo.localization.getString("assets_material_metalness_value"));
	metalnessValueWidget->setValue(0.5f);
	metalnessValueWidget->setMin(0.0f);
	metalnessValueWidget->setMax(1.0f);
	layout()->addWidget(metalnessValueWidget);
	layout()->addWidget(new SeparatorLine());
	roughnessTextureImageWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_roughness_image"), m_globalInfo.localization.getString("no_image_selected"), "");
	layout()->addWidget(roughnessTextureImageWidget);
	roughnessTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_roughness_image_sampler"), m_globalInfo.localization.getString("no_image_sampler_selected"), "");
	layout()->addWidget(roughnessTextureImageSamplerWidget);
	roughnessValueWidget = new ScalarWidget(globalInfo, m_globalInfo.localization.getString("assets_material_roughness_value"));
	roughnessValueWidget->setValue(0.5f);
	roughnessValueWidget->setMin(0.0f);
	roughnessValueWidget->setMax(1.0f);
	layout()->addWidget(roughnessValueWidget);
	layout()->addWidget(new SeparatorLine());
	occlusionTextureImageWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_occlusion_image"), m_globalInfo.localization.getString("no_image_selected"), "");
	layout()->addWidget(occlusionTextureImageWidget);
	occlusionTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_occlusion_image_sampler"), m_globalInfo.localization.getString("no_image_sampler_selected"), "");
	layout()->addWidget(occlusionTextureImageSamplerWidget);
	occlusionValueWidget = new ScalarWidget(globalInfo, m_globalInfo.localization.getString("assets_material_occlusion_value"));
	occlusionValueWidget->setValue(1.0f);
	occlusionValueWidget->setMin(0.0f);
	occlusionValueWidget->setMax(1.0f);
	layout()->addWidget(occlusionValueWidget);
	layout()->addWidget(new SeparatorLine());
	emissiveTextureImageWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_emissive_image"), m_globalInfo.localization.getString("no_image_selected"), "");
	layout()->addWidget(emissiveTextureImageWidget);
	emissiveTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, m_globalInfo.localization.getString("assets_material_emissive_image_sampler"), m_globalInfo.localization.getString("no_image_sampler_selected"), "");
	layout()->addWidget(emissiveTextureImageSamplerWidget);
	emissiveColorWidget = new ColorPickerWidget(globalInfo, m_globalInfo.localization.getString("assets_material_emissive_color"), nml::vec3(0.0f, 0.0f, 0.0f));
	layout()->addWidget(emissiveColorWidget);
	emissiveFactorWidget = new ScalarWidget(globalInfo, m_globalInfo.localization.getString("assets_material_emissive_factor"));
	emissiveFactorWidget->setValue(1.0f);
	layout()->addWidget(emissiveFactorWidget);
	layout()->addWidget(new SeparatorLine());
	alphaCutoffWidget = new ScalarWidget(globalInfo, m_globalInfo.localization.getString("assets_material_alpha_cutoff"));
	alphaCutoffWidget->setMin(0.0f);
	alphaCutoffWidget->setMax(1.0f);
	layout()->addWidget(alphaCutoffWidget);
	indexOfRefractionWidget = new ScalarWidget(globalInfo, m_globalInfo.localization.getString("assets_material_index_of_refraction"));
	layout()->addWidget(indexOfRefractionWidget);
	useTriplanarMappingWidget = new BooleanWidget(globalInfo, m_globalInfo.localization.getString("assets_material_use_triplanar_mapping"));
	useTriplanarMappingWidget->setValue(false);
	layout()->addWidget(new SeparatorLine());
	layout()->addWidget(useTriplanarMappingWidget);
	scaleUVWidget = new Vector2Widget(globalInfo, m_globalInfo.localization.getString("assets_material_scale_uv"));
	layout()->addWidget(scaleUVWidget);
	offsetUVWidget = new Vector2Widget(globalInfo, m_globalInfo.localization.getString("assets_material_offset_uv"));
	layout()->addWidget(offsetUVWidget);

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
	connect(useTriplanarMappingWidget, &BooleanWidget::stateChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(scaleUVWidget, &Vector2Widget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
	connect(offsetUVWidget, &Vector2Widget::valueChanged, this, &MaterialNtmlFileWidget::onValueChanged);
}

void MaterialNtmlFileWidget::setPath(const std::string& path) {
	m_materialFilePath = path;
	std::fstream materialFile(m_materialFilePath, std::ios::in);
	if (materialFile.is_open()) {
		if (!nlohmann::json::accept(materialFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_is_not_valid_json", { m_materialFilePath }));
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_cannot_be_opened", { m_materialFilePath }));
		return;
	}

	materialFile = std::fstream(m_materialFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(materialFile);

	materialNtml = MaterialNtml();

	if (j.contains("diffuse")) {
		if (j["diffuse"].contains("texture")) {
			if (j["diffuse"]["texture"].contains("imagePath")) {
				materialNtml.diffuseTextureImagePath = j["diffuse"]["texture"]["imagePath"];
			}
			if (j["diffuse"]["texture"].contains("imageSamplerPath")) {
				materialNtml.diffuseTextureImageSamplerPath = j["diffuse"]["texture"]["imageSamplerPath"];
			}
		}
		if (j["diffuse"].contains("color")) {
			materialNtml.diffuseColor.x = j["diffuse"]["color"][0];
			materialNtml.diffuseColor.y = j["diffuse"]["color"][1];
			materialNtml.diffuseColor.z = j["diffuse"]["color"][2];
			materialNtml.opacity = j["diffuse"]["color"][3];
		}
	}
	if (j.contains("normal")) {
		if (j["normal"].contains("texture")) {
			if (j["normal"]["texture"].contains("imagePath")) {
				materialNtml.normalTextureImagePath = j["normal"]["texture"]["imagePath"];
			}
			if (j["normal"]["texture"].contains("imageSamplerPath")) {
				materialNtml.normalTextureImageSamplerPath = j["normal"]["texture"]["imageSamplerPath"];
			}
		}
	}
	if (j.contains("metalness")) {
		if (j["metalness"].contains("texture")) {
			if (j["metalness"]["texture"].contains("imagePath")) {
				materialNtml.metalnessTextureImagePath = j["metalness"]["texture"]["imagePath"];
			}
			if (j["metalness"]["texture"].contains("imageSamplerPath")) {
				materialNtml.metalnessTextureImageSamplerPath = j["metalness"]["texture"]["imageSamplerPath"];
			}
		}
		if (j["metalness"].contains("value")) {
			materialNtml.metalnessValue = j["metalness"]["value"];
		}
	}
	if (j.contains("roughness")) {
		if (j["roughness"].contains("texture")) {
			if (j["roughness"]["texture"].contains("imagePath")) {
				materialNtml.roughnessTextureImagePath = j["roughness"]["texture"]["imagePath"];
			}
			if (j["roughness"]["texture"].contains("imageSamplerPath")) {
				materialNtml.roughnessTextureImageSamplerPath = j["roughness"]["texture"]["imageSamplerPath"];
			}
		}
		if (j["roughness"].contains("value")) {
			materialNtml.roughnessValue = j["roughness"]["value"];
		}
	}
	if (j.contains("occlusion")) {
		if (j["occlusion"].contains("texture")) {
			if (j["occlusion"]["texture"].contains("imagePath")) {
				materialNtml.occlusionTextureImagePath = j["occlusion"]["texture"]["imagePath"];
			}
			if (j["occlusion"]["texture"].contains("imageSamplerPath")) {
				materialNtml.occlusionTextureImageSamplerPath = j["occlusion"]["texture"]["imageSamplerPath"];
			}
		}
		if (j["occlusion"].contains("value")) {
			materialNtml.occlusionValue = j["occlusion"]["value"];
		}
	}
	if (j.contains("emissive")) {
		if (j["emissive"].contains("texture")) {
			if (j["emissive"]["texture"].contains("imagePath")) {
				std::string imagePath = j["emissive"]["texture"]["imagePath"];
				materialNtml.emissiveTextureImagePath = imagePath;
			}
			if (j["emissive"]["texture"].contains("imageSamplerPath")) {
				std::string imageSamplerPath = j["emissive"]["texture"]["imageSamplerPath"];
				materialNtml.emissiveTextureImageSamplerPath = imageSamplerPath;
			}
		}
		if (j["emissive"].contains("color")) {
			materialNtml.emissiveColor.x = j["emissive"]["color"][0];
			materialNtml.emissiveColor.y = j["emissive"]["color"][1];
			materialNtml.emissiveColor.z = j["emissive"]["color"][2];
		}
		if (j["emissive"].contains("factor")) {
			materialNtml.emissiveFactor = j["emissive"]["factor"];
		}
	}
	if (j.contains("alphaCutoff")) {
		materialNtml.alphaCutoff = j["alphaCutoff"];
	}
	if (j.contains("indexOfRefraction")) {
		materialNtml.indexOfRefraction = j["indexOfRefraction"];
	}
	if (j.contains("useTriplanarMapping")) {
		materialNtml.useTriplanarMapping = j["useTriplanarMapping"];
	}
	if (j.contains("scaleUV")) {
		materialNtml.scaleUV.x = j["scaleUV"][0];
		materialNtml.scaleUV.y = j["scaleUV"][1];
	}
	if (j.contains("offsetUV")) {
		materialNtml.offsetUV.x = j["offsetUV"][0];
		materialNtml.offsetUV.y = j["offsetUV"][1];
	}

	updateWidgets();
}

void MaterialNtmlFileWidget::updateWidgets() {
	diffuseTextureImageWidget->setPath(materialNtml.diffuseTextureImagePath);
	diffuseTextureImageSamplerWidget->setPath(materialNtml.diffuseTextureImageSamplerPath);
	diffuseColorWidget->setColor(materialNtml.diffuseColor);
	opacityValueWidget->setValue(materialNtml.opacity);
	normalTextureImageWidget->setPath(materialNtml.normalTextureImagePath);
	normalTextureImageSamplerWidget->setPath(materialNtml.normalTextureImageSamplerPath);
	metalnessTextureImageWidget->setPath(materialNtml.metalnessTextureImagePath);
	metalnessTextureImageSamplerWidget->setPath(materialNtml.metalnessTextureImageSamplerPath);
	metalnessValueWidget->setValue(materialNtml.metalnessValue);
	roughnessTextureImageWidget->setPath(materialNtml.roughnessTextureImagePath);
	roughnessTextureImageSamplerWidget->setPath(materialNtml.roughnessTextureImageSamplerPath);
	roughnessValueWidget->setValue(materialNtml.roughnessValue);
	occlusionTextureImageWidget->setPath(materialNtml.occlusionTextureImagePath);
	occlusionTextureImageSamplerWidget->setPath(materialNtml.occlusionTextureImageSamplerPath);
	occlusionValueWidget->setValue(materialNtml.occlusionValue);
	emissiveTextureImageWidget->setPath(materialNtml.emissiveTextureImagePath);
	emissiveTextureImageSamplerWidget->setPath(materialNtml.emissiveTextureImageSamplerPath);
	emissiveColorWidget->setColor(materialNtml.emissiveColor);
	emissiveFactorWidget->setValue(materialNtml.emissiveFactor);
	alphaCutoffWidget->setValue(materialNtml.alphaCutoff);
	indexOfRefractionWidget->setValue(materialNtml.indexOfRefraction);
	useTriplanarMappingWidget->setValue(materialNtml.useTriplanarMapping);
	scaleUVWidget->setValue(materialNtml.scaleUV);
	offsetUVWidget->setValue(materialNtml.offsetUV);
}

void MaterialNtmlFileWidget::save() {
	nlohmann::json j;
	if (!materialNtml.diffuseTextureImagePath.empty()) {
		j["diffuse"]["texture"]["imagePath"] = materialNtml.diffuseTextureImagePath;
	}
	if (!materialNtml.diffuseTextureImageSamplerPath.empty()) {
		j["diffuse"]["texture"]["imageSamplerPath"] = materialNtml.diffuseTextureImageSamplerPath;
	}
	nml::vec3 diffuseColor = materialNtml.diffuseColor;
	float opacityValue = materialNtml.opacity;
	j["diffuse"]["color"] = { diffuseColor.x, diffuseColor.y, diffuseColor.z, opacityValue };
	if (!materialNtml.normalTextureImagePath.empty()) {
		j["normal"]["texture"]["imagePath"] = materialNtml.normalTextureImagePath;
	}
	if (!materialNtml.normalTextureImageSamplerPath.empty()) {
		j["normal"]["texture"]["imageSamplerPath"] = materialNtml.normalTextureImageSamplerPath;
	}
	if (!materialNtml.metalnessTextureImagePath.empty()) {
		j["metalness"]["texture"]["imagePath"] = materialNtml.metalnessTextureImagePath;
	}
	if (!materialNtml.metalnessTextureImageSamplerPath.empty()) {
		j["metalness"]["texture"]["imageSamplerPath"] = materialNtml.metalnessTextureImageSamplerPath;
	}
	j["metalness"]["value"] = materialNtml.metalnessValue;
	if (!materialNtml.roughnessTextureImagePath.empty()) {
		j["roughness"]["texture"]["imagePath"] = materialNtml.roughnessTextureImagePath;
	}
	if (!materialNtml.roughnessTextureImageSamplerPath.empty()) {
		j["roughness"]["texture"]["imageSamplerPath"] = materialNtml.roughnessTextureImageSamplerPath;
	}
	j["roughness"]["value"] = materialNtml.roughnessValue;
	if (!materialNtml.occlusionTextureImagePath.empty()) {
		j["occlusion"]["texture"]["imagePath"] = materialNtml.occlusionTextureImagePath;
	}
	if (!materialNtml.occlusionTextureImageSamplerPath.empty()) {
		j["occlusion"]["texture"]["imageSamplerPath"] = materialNtml.occlusionTextureImageSamplerPath;
	}
	j["occlusion"]["value"] = materialNtml.occlusionValue;
	if (!materialNtml.emissiveTextureImagePath.empty()) {
		j["emissive"]["texture"]["imagePath"] = materialNtml.emissiveTextureImagePath;
	}
	if (!materialNtml.emissiveTextureImageSamplerPath.empty()) {
		j["emissive"]["texture"]["imageSamplerPath"] = materialNtml.emissiveTextureImageSamplerPath;
	}
	j["emissive"]["color"] = { materialNtml.emissiveColor.x, materialNtml.emissiveColor.y, materialNtml.emissiveColor.z };
	j["emissive"]["factor"] = materialNtml.emissiveFactor;
	j["alphaCutoff"] = materialNtml.alphaCutoff;
	j["indexOfRefraction"] = materialNtml.indexOfRefraction;
	j["useTriplanarMapping"] = materialNtml.useTriplanarMapping;
	j["scaleUV"] = { materialNtml.scaleUV.x, materialNtml.scaleUV.y };
	j["offsetUV"] = { materialNtml.offsetUV.x, materialNtml.offsetUV.y };

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
}

void MaterialNtmlFileWidget::onValueChanged() {
	MaterialNtml newMaterialNtml = materialNtml;

	QObject* senderWidget = sender();
	if (senderWidget == diffuseTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(diffuseTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.diffuseTextureImagePath = path;
	}
	else if (senderWidget == diffuseTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(diffuseTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.diffuseTextureImageSamplerPath = path;
	}
	else if (senderWidget == diffuseColorWidget) {
		newMaterialNtml.diffuseColor = diffuseColorWidget->getColor();
	}
	else if (senderWidget == opacityValueWidget) {
		newMaterialNtml.opacity = opacityValueWidget->getValue();
	}
	else if (senderWidget == normalTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(normalTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.normalTextureImagePath = path;
	}
	else if (senderWidget == normalTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(normalTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.normalTextureImageSamplerPath = path;
	}
	else if (senderWidget == metalnessTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(metalnessTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.metalnessTextureImagePath = path;
	}
	else if (senderWidget == metalnessTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(metalnessTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.metalnessTextureImageSamplerPath = path;
	}
	else if (senderWidget == metalnessValueWidget) {
		newMaterialNtml.metalnessValue = metalnessValueWidget->getValue();
	}
	else if (senderWidget == roughnessTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(roughnessTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.roughnessTextureImagePath = path;
	}
	else if (senderWidget == roughnessTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(roughnessTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.roughnessTextureImageSamplerPath = path;
	}
	else if (senderWidget == roughnessValueWidget) {
		newMaterialNtml.roughnessValue = roughnessValueWidget->getValue();
	}
	else if (senderWidget == occlusionTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(occlusionTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.occlusionTextureImagePath = path;
	}
	else if (senderWidget == occlusionTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(occlusionTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.occlusionTextureImageSamplerPath = path;
	}
	else if (senderWidget == occlusionValueWidget) {
		newMaterialNtml.occlusionValue = occlusionValueWidget->getValue();
	}
	else if (senderWidget == emissiveTextureImageWidget) {
		std::string path = AssetHelper::absoluteToRelative(emissiveTextureImageWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.emissiveTextureImagePath = path;
	}
	else if (senderWidget == emissiveTextureImageSamplerWidget) {
		std::string path = AssetHelper::absoluteToRelative(emissiveTextureImageSamplerWidget->getPath(), m_globalInfo.projectDirectory);
		newMaterialNtml.emissiveTextureImageSamplerPath = path;
	}
	else if (senderWidget == emissiveColorWidget) {
		newMaterialNtml.emissiveColor = emissiveColorWidget->getColor();
	}
	else if (senderWidget == emissiveFactorWidget) {
		newMaterialNtml.emissiveFactor = emissiveFactorWidget->getValue();
	}
	else if (senderWidget == alphaCutoffWidget) {
		newMaterialNtml.alphaCutoff = alphaCutoffWidget->getValue();
	}
	else if (senderWidget == indexOfRefractionWidget) {
		newMaterialNtml.indexOfRefraction = indexOfRefractionWidget->getValue();
	}
	else if (senderWidget == useTriplanarMappingWidget) {
		newMaterialNtml.useTriplanarMapping = useTriplanarMappingWidget->getValue();
	}
	else if (senderWidget == scaleUVWidget) {
		newMaterialNtml.scaleUV = scaleUVWidget->getValue();
	}
	else if (senderWidget == offsetUVWidget) {
		newMaterialNtml.offsetUV = offsetUVWidget->getValue();
	}

	if (newMaterialNtml != materialNtml) {
		m_globalInfo.actionUndoStack->push(new ChangeMaterialNtmlFile(m_globalInfo, newMaterialNtml, m_materialFilePath));
	}
}

ChangeMaterialNtmlFile::ChangeMaterialNtmlFile(GlobalInfo& globalInfo, MaterialNtml newMaterialNtml, const std::string& filePath) : m_globalInfo(globalInfo) {
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_material", { filePath })));

	m_materialNtmlFileWidget = globalInfo.mainWindow->infoPanel->assetInfoPanel->assetInfoScrollArea->assetInfoList->materialNtmlFileWidget;
	m_oldMaterialNtml = m_materialNtmlFileWidget->materialNtml;
	m_newMaterialNtml = newMaterialNtml;
	m_filePath = filePath;
}

void ChangeMaterialNtmlFile::undo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, {}));

	m_materialNtmlFileWidget->materialNtml = m_oldMaterialNtml;
	m_materialNtmlFileWidget->updateWidgets();

	m_materialNtmlFileWidget->save();
}

void ChangeMaterialNtmlFile::redo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, {}));

	m_materialNtmlFileWidget->materialNtml = m_newMaterialNtml;
	m_materialNtmlFileWidget->updateWidgets();

	m_materialNtmlFileWidget->save();
}