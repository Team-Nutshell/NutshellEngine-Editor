#include "sampler_ntsp_file_widget.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>

SamplerNtspFileWidget::SamplerNtspFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	std::vector<std::string> filterElements = { "Linear", "Nearest", "Unknown" };
	magFilterWidget = new ComboBoxWidget(globalInfo, "Mag Filter", filterElements);
	layout()->addWidget(magFilterWidget);
	minFilterWidget = new ComboBoxWidget(globalInfo, "Min Filter", filterElements);
	layout()->addWidget(minFilterWidget);
	mipmapFilterWidget = new ComboBoxWidget(globalInfo, "Mipmap Filter", filterElements);
	layout()->addWidget(mipmapFilterWidget);
	std::vector<std::string> addressModeElements = { "Repeat", "MirroredRepeat", "ClampToEdge", "ClampToBorder", "Unknown" };
	addressModeUWidget = new ComboBoxWidget(globalInfo, "Address Mode U", addressModeElements);
	layout()->addWidget(addressModeUWidget);
	addressModeVWidget = new ComboBoxWidget(globalInfo, "Address Mode V", addressModeElements);
	layout()->addWidget(addressModeVWidget);
	addressModeWWidget = new ComboBoxWidget(globalInfo, "Address Mode W", addressModeElements);
	layout()->addWidget(addressModeWWidget);
	std::vector<std::string> borderColorElements = { "FloatTransparentBlack", "IntTransparentBlack", "FloatOpaqueBlack", "IntOpaqueBlack", "FloatOpaqueWhite", "IntOpaqueWhite", "Unknown" };
	borderColorWidget = new ComboBoxWidget(globalInfo, "Border Color", borderColorElements);
	layout()->addWidget(borderColorWidget);
	anisotropyLevelWidget = new IntegerWidget(globalInfo, "Anisotropy Level");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + m_samplerFilePath + "\" is not a valid JSON file.");
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + m_samplerFilePath + "\" cannot be opened.");
		return;
	}

	samplerFile = std::fstream(m_samplerFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(samplerFile);

	if (j.contains("magFilter")) {
		std::string magFilter = j["magFilter"];
		samplerNtsp.magFilter = magFilter;
	}
	if (j.contains("minFilter")) {
		std::string minFilter = j["minFilter"];
		samplerNtsp.minFilter = minFilter;
	}
	if (j.contains("mipmapFilter")) {
		std::string mipmapFilter = j["mipmapFilter"];
		samplerNtsp.mipmapFilter = mipmapFilter;
	}
	if (j.contains("addressModeU")) {
		std::string addressModeU = j["addressModeU"];
		samplerNtsp.addressModeU = addressModeU;
	}
	if (j.contains("addressModeV")) {
		std::string addressModeV = j["addressModeV"];
		samplerNtsp.addressModeV = addressModeV;
	}
	if (j.contains("addressModeW")) {
		std::string addressModeW = j["addressModeW"];
		samplerNtsp.addressModeW = addressModeW;
	}
	if (j.contains("borderColor")) {
		std::string borderColor = j["borderColor"];
		samplerNtsp.borderColor = borderColor;
	}
	if (j.contains("anistropyLevel")) {
		int anisotropyLevel = j["anisotropyLevel"];
		samplerNtsp.anisotropyLevel = anisotropyLevel;
	}

	updateWidgets();
}

void SamplerNtspFileWidget::updateWidgets() {
	magFilterWidget->setElementByText(samplerNtsp.magFilter);
	minFilterWidget->setElementByText(samplerNtsp.minFilter);
	mipmapFilterWidget->setElementByText(samplerNtsp.mipmapFilter);
	addressModeUWidget->setElementByText(samplerNtsp.addressModeU);
	addressModeVWidget->setElementByText(samplerNtsp.addressModeV);
	addressModeWWidget->setElementByText(samplerNtsp.addressModeW);
	borderColorWidget->setElementByText(samplerNtsp.borderColor);
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

void SamplerNtspFileWidget::onValueChanged() {
	SamplerNtsp newSamplerNtsp = samplerNtsp;

	QObject* senderWidget = sender();
	if (senderWidget == magFilterWidget) {
		newSamplerNtsp.magFilter = magFilterWidget->getElementText();
	}
	else if (senderWidget == minFilterWidget) {
		newSamplerNtsp.minFilter = minFilterWidget->getElementText();
	}
	else if (senderWidget == mipmapFilterWidget) {
		newSamplerNtsp.mipmapFilter = mipmapFilterWidget->getElementText();
	}
	else if (senderWidget == addressModeUWidget) {
		newSamplerNtsp.addressModeU = addressModeUWidget->getElementText();
	}
	else if (senderWidget == addressModeVWidget) {
		newSamplerNtsp.addressModeV = addressModeVWidget->getElementText();
	}
	else if (senderWidget == addressModeWWidget) {
		newSamplerNtsp.addressModeW = addressModeWWidget->getElementText();
	}
	else if (senderWidget == borderColorWidget) {
		newSamplerNtsp.borderColor = borderColorWidget->getElementText();
	}
	else if (senderWidget == anisotropyLevelWidget) {
		newSamplerNtsp.anisotropyLevel = anisotropyLevelWidget->getValue();
	}

	if (newSamplerNtsp != samplerNtsp) {
		save();
	}
}

ChangeSamplerNtspFile::ChangeSamplerNtspFile(GlobalInfo& globalInfo, SamplerNtsp newSamplerNtsp, const std::string& filePath) : m_globalInfo(globalInfo) {
	setText("Change Sampler Ntsp");

	m_samplerNtspFileWidget = globalInfo.mainWindow->infoPanel->assetInfoPanel->assetInfoScrollArea->assetInfoList->samplerNtspFileWidget;
	m_oldSamplerNtsp = m_samplerNtspFileWidget->samplerNtsp;
	m_newSamplerNtsp = newSamplerNtsp;
	m_filePath = filePath;
}

void ChangeSamplerNtspFile::undo() {
	emit m_globalInfo.signalEmitter.selectAssetSignal(m_filePath);

	m_samplerNtspFileWidget->samplerNtsp = m_oldSamplerNtsp;
	m_samplerNtspFileWidget->updateWidgets();

	m_samplerNtspFileWidget->save();
}

void ChangeSamplerNtspFile::redo() {
	emit m_globalInfo.signalEmitter.selectAssetSignal(m_filePath);

	m_samplerNtspFileWidget->samplerNtsp = m_newSamplerNtsp;
	m_samplerNtspFileWidget->updateWidgets();

	m_samplerNtspFileWidget->save();
}