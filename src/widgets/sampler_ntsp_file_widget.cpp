#include "sampler_ntsp_file_widget.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include "../undo_commands/select_asset_entities_command.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>

SamplerNtspFileWidget::SamplerNtspFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(2, 0, 2, 0);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_image_sampler_file"))));
	std::vector<std::string> filterElements = { m_globalInfo.localization.getString("assets_image_sampler_filter_linear"), m_globalInfo.localization.getString("assets_image_sampler_filter_nearest"), m_globalInfo.localization.getString("assets_image_sampler_filter_unknown") };
	magFilterWidget = new ComboBoxWidget(globalInfo, m_globalInfo.localization.getString("assets_image_sampler_mag_filter"), filterElements);
	layout()->addWidget(magFilterWidget);
	minFilterWidget = new ComboBoxWidget(globalInfo, m_globalInfo.localization.getString("assets_image_sampler_min_filter"), filterElements);
	layout()->addWidget(minFilterWidget);
	mipmapFilterWidget = new ComboBoxWidget(globalInfo, m_globalInfo.localization.getString("assets_image_sampler_mipmap_filter"), filterElements);
	layout()->addWidget(mipmapFilterWidget);
	std::vector<std::string> addressModeElements = { m_globalInfo.localization.getString("assets_image_sampler_address_mode_repeat"), m_globalInfo.localization.getString("assets_image_sampler_address_mode_mirrored_repeat"), m_globalInfo.localization.getString("assets_image_sampler_address_mode_clamp_to_edge"), m_globalInfo.localization.getString("assets_image_sampler_address_mode_clamp_to_border"), m_globalInfo.localization.getString("assets_image_sampler_address_mode_unknown") };
	addressModeUWidget = new ComboBoxWidget(globalInfo, m_globalInfo.localization.getString("assets_image_sampler_address_mode_u"), addressModeElements);
	layout()->addWidget(addressModeUWidget);
	addressModeVWidget = new ComboBoxWidget(globalInfo, m_globalInfo.localization.getString("assets_image_sampler_address_mode_v"), addressModeElements);
	layout()->addWidget(addressModeVWidget);
	addressModeWWidget = new ComboBoxWidget(globalInfo, m_globalInfo.localization.getString("assets_image_sampler_address_mode_w"), addressModeElements);
	layout()->addWidget(addressModeWWidget);
	std::vector<std::string> borderColorElements = { m_globalInfo.localization.getString("assets_image_sampler_border_color_float_transparent_black"), m_globalInfo.localization.getString("assets_image_sampler_border_color_int_transparent_black"), m_globalInfo.localization.getString("assets_image_sampler_border_color_float_opaque_black"), m_globalInfo.localization.getString("assets_image_sampler_border_color_int_opaque_black"), m_globalInfo.localization.getString("assets_image_sampler_border_color_float_opaque_white"), m_globalInfo.localization.getString("assets_image_sampler_border_color_int_opaque_white"), m_globalInfo.localization.getString("assets_image_sampler_border_color_unknown") };
	borderColorWidget = new ComboBoxWidget(globalInfo, m_globalInfo.localization.getString("assets_image_sampler_border_color"), borderColorElements);
	layout()->addWidget(borderColorWidget);
	anisotropyLevelWidget = new IntegerWidget(globalInfo, m_globalInfo.localization.getString("assets_image_sampler_anisotropy_level"));
	anisotropyLevelWidget->setMin(0);
	layout()->addWidget(anisotropyLevelWidget);

	connect(magFilterWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(minFilterWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(mipmapFilterWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(addressModeUWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(addressModeVWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(addressModeWWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(borderColorWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(anisotropyLevelWidget, &IntegerWidget::valueChanged, this, &SamplerNtspFileWidget::onValueChanged);
}

void SamplerNtspFileWidget::setPath(const std::string& path) {
	m_samplerFilePath = path;
	std::fstream samplerFile(m_samplerFilePath, std::ios::in);
	if (samplerFile.is_open()) {
		if (!nlohmann::json::accept(samplerFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_is_not_valid_json", { m_samplerFilePath }));
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_cannot_be_opened", { m_samplerFilePath }));
		return;
	}

	samplerFile = std::fstream(m_samplerFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(samplerFile);

	samplerNtsp = SamplerNtsp();

	if (j.contains("magFilter")) {
		samplerNtsp.magFilter = j["magFilter"];
	}
	if (j.contains("minFilter")) {
		samplerNtsp.minFilter = j["minFilter"];
	}
	if (j.contains("mipmapFilter")) {
		samplerNtsp.mipmapFilter = j["mipmapFilter"];
	}
	if (j.contains("addressModeU")) {
		samplerNtsp.addressModeU = j["addressModeU"];
	}
	if (j.contains("addressModeV")) {
		samplerNtsp.addressModeV = j["addressModeV"];
	}
	if (j.contains("addressModeW")) {
		samplerNtsp.addressModeW = j["addressModeW"];
	}
	if (j.contains("borderColor")) {
		samplerNtsp.borderColor = j["borderColor"];
	}
	if (j.contains("anisotropyLevel")) {
		samplerNtsp.anisotropyLevel = j["anisotropyLevel"];
	}

	updateWidgets();
}

void SamplerNtspFileWidget::updateWidgets() {
	magFilterWidget->setElementByText(typeToFilter(samplerNtsp.magFilter));
	minFilterWidget->setElementByText(typeToFilter(samplerNtsp.minFilter));
	mipmapFilterWidget->setElementByText(typeToFilter(samplerNtsp.mipmapFilter));
	addressModeUWidget->setElementByText(typeToAddressMode(samplerNtsp.addressModeU));
	addressModeVWidget->setElementByText(typeToAddressMode(samplerNtsp.addressModeV));
	addressModeWWidget->setElementByText(typeToAddressMode(samplerNtsp.addressModeW));
	borderColorWidget->setElementByText(typeToBorderColor(samplerNtsp.borderColor));
	anisotropyLevelWidget->setValue(samplerNtsp.anisotropyLevel);
}

void SamplerNtspFileWidget::save() {
	nlohmann::json j;
	j["magFilter"] = samplerNtsp.magFilter;
	j["minFilter"] = samplerNtsp.minFilter;
	j["mipmapFilter"] = samplerNtsp.mipmapFilter;
	j["addressModeU"] = samplerNtsp.addressModeU;
	j["addressModeV"] = samplerNtsp.addressModeV;
	j["addressModeW"] = samplerNtsp.addressModeW;
	j["borderColor"] = samplerNtsp.borderColor;
	j["anisotropyLevel"] = samplerNtsp.anisotropyLevel;

	std::fstream samplerFile(m_samplerFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		samplerFile << "{\n}";
	}
	else {
		samplerFile << j.dump(1, '\t');
	}
	samplerFile.close();

	std::string samplerPath = AssetHelper::absoluteToRelative(m_samplerFilePath, m_globalInfo.projectDirectory);
	m_globalInfo.rendererResourceManager.loadSampler(m_samplerFilePath, samplerPath);
}

std::string SamplerNtspFileWidget::filterToType(const std::string& filterName) {
	if (filterName == m_globalInfo.localization.getString("assets_image_sampler_filter_linear")) {
		return "Linear";
	}
	else if (filterName == m_globalInfo.localization.getString("assets_image_sampler_filter_nearest")) {
		return "Nearest";
	}
	else {
		return "Unknown";
	}
}

std::string SamplerNtspFileWidget::typeToFilter(const std::string& type) {
	if (type == "Linear") {
		return m_globalInfo.localization.getString("assets_image_sampler_filter_linear");
	}
	else if (type == "Nearest") {
		return m_globalInfo.localization.getString("assets_image_sampler_filter_nearest");
	}
	else {
		return "Unknown";
	}
}

std::string SamplerNtspFileWidget::addressModeToType(const std::string& addressModeName) {
	if (addressModeName == m_globalInfo.localization.getString("assets_image_sampler_address_mode_repeat")) {
		return "Repeat";
	}
	else if (addressModeName == m_globalInfo.localization.getString("assets_image_sampler_address_mode_mirrored_repeat")) {
		return "MirroredRepeat";
	}
	else if (addressModeName == m_globalInfo.localization.getString("assets_image_sampler_address_mode_clamp_to_edge")) {
		return "ClampToEdge";
	}
	else if (addressModeName == m_globalInfo.localization.getString("assets_image_sampler_address_mode_clamp_to_border")) {
		return "ClampToBorder";
	}
	else {
		return "Unknown";
	}
}

std::string SamplerNtspFileWidget::typeToAddressMode(const std::string& type) {
	if (type == "Repeat") {
		return m_globalInfo.localization.getString("assets_image_sampler_address_mode_repeat");
	}
	else if (type == "MirroredRepeat") {
		return m_globalInfo.localization.getString("assets_image_sampler_address_mode_mirrored_repeat");
	}
	else if (type == "ClampToEdge") {
		return m_globalInfo.localization.getString("assets_image_sampler_address_mode_clamp_to_edge");
	}
	else if (type == "ClampToBorder") {
		return m_globalInfo.localization.getString("assets_image_sampler_address_mode_clamp_to_border");
	}
	else {
		return "Unknown";
	}
}

std::string SamplerNtspFileWidget::borderColorToType(const std::string& borderColorName) {
	if (borderColorName == m_globalInfo.localization.getString("assets_image_sampler_border_color_float_transparent_black")) {
		return "FloatTransparentBlack";
	}
	else if (borderColorName == m_globalInfo.localization.getString("assets_image_sampler_border_color_int_transparent_black")) {
		return "IntTransparentBlack";
	}
	else if (borderColorName == m_globalInfo.localization.getString("assets_image_sampler_border_color_float_opaque_black")) {
		return "FloatOpaqueBlack";
	}
	else if (borderColorName == m_globalInfo.localization.getString("assets_image_sampler_border_color_int_opaque_black")) {
		return "IntOpaqueBlack";
	}
	else if (borderColorName == m_globalInfo.localization.getString("assets_image_sampler_border_color_float_opaque_white")) {
		return "FloatOpaqueWhite";
	}
	else if (borderColorName == m_globalInfo.localization.getString("assets_image_sampler_border_color_int_opaque_white")) {
		return "IntOpaqueWhite";
	}
	else {
		return "Unknown";
	}
}

std::string SamplerNtspFileWidget::typeToBorderColor(const std::string& type) {
	if (type == "FloatTransparentBlack") {
		return m_globalInfo.localization.getString("assets_image_sampler_border_color_float_transparent_black");
	}
	else if (type == "IntTransparentBlack") {
		return m_globalInfo.localization.getString("assets_image_sampler_border_color_int_transparent_black");
	}
	else if (type == "FloatOpaqueBlack") {
		return m_globalInfo.localization.getString("assets_image_sampler_border_color_float_opaque_black");
	}
	else if (type == "IntOpaqueBlack") {
		return m_globalInfo.localization.getString("assets_image_sampler_border_color_int_opaque_black");
	}
	else if (type == "FloatOpaqueWhite") {
		return m_globalInfo.localization.getString("assets_image_sampler_border_color_float_opaque_white");
	}
	else if (type == "IntOpaqueWhite") {
		return m_globalInfo.localization.getString("assets_image_sampler_border_color_int_opaque_white");
	}
	else {
		return "Unknown";
	}
}

void SamplerNtspFileWidget::onValueChanged() {
	SamplerNtsp newSamplerNtsp = samplerNtsp;

	QObject* senderWidget = sender();
	if (senderWidget == magFilterWidget) {
		newSamplerNtsp.magFilter = filterToType(magFilterWidget->getElementText());
	}
	else if (senderWidget == minFilterWidget) {
		newSamplerNtsp.minFilter = filterToType(minFilterWidget->getElementText());
	}
	else if (senderWidget == mipmapFilterWidget) {
		newSamplerNtsp.mipmapFilter = filterToType(mipmapFilterWidget->getElementText());
	}
	else if (senderWidget == addressModeUWidget) {
		newSamplerNtsp.addressModeU = addressModeToType(addressModeUWidget->getElementText());
	}
	else if (senderWidget == addressModeVWidget) {
		newSamplerNtsp.addressModeV = addressModeToType(addressModeVWidget->getElementText());
	}
	else if (senderWidget == addressModeWWidget) {
		newSamplerNtsp.addressModeW = addressModeToType(addressModeWWidget->getElementText());
	}
	else if (senderWidget == borderColorWidget) {
		newSamplerNtsp.borderColor = borderColorToType(borderColorWidget->getElementText());
	}
	else if (senderWidget == anisotropyLevelWidget) {
		newSamplerNtsp.anisotropyLevel = anisotropyLevelWidget->getValue();
	}

	if (newSamplerNtsp != samplerNtsp) {
		m_globalInfo.actionUndoStack->push(new ChangeSamplerNtspFile(m_globalInfo, newSamplerNtsp, m_samplerFilePath));
	}
}

ChangeSamplerNtspFile::ChangeSamplerNtspFile(GlobalInfo& globalInfo, SamplerNtsp newSamplerNtsp, const std::string& filePath) : m_globalInfo(globalInfo) {
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_sampler", { filePath })));

	m_samplerNtspFileWidget = globalInfo.mainWindow->infoPanel->assetInfoPanel->assetInfoScrollArea->assetInfoList->samplerNtspFileWidget;
	m_oldSamplerNtsp = m_samplerNtspFileWidget->samplerNtsp;
	m_newSamplerNtsp = newSamplerNtsp;
	m_filePath = filePath;
}

void ChangeSamplerNtspFile::undo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, std::set<EntityID>()));

	m_samplerNtspFileWidget->samplerNtsp = m_oldSamplerNtsp;
	m_samplerNtspFileWidget->updateWidgets();

	m_samplerNtspFileWidget->save();
}

void ChangeSamplerNtspFile::redo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, std::set<EntityID>()));

	m_samplerNtspFileWidget->samplerNtsp = m_newSamplerNtsp;
	m_samplerNtspFileWidget->updateWidgets();

	m_samplerNtspFileWidget->save();
}