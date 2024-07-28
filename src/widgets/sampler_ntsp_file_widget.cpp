#include "sampler_ntsp_file_widget.h"
#include "../common/save_title_changer.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <fstream>

SamplerNtspFileWidget::SamplerNtspFileWidget(GlobalInfo& globalInfo, const std::string& samplerFilePath) : m_globalInfo(globalInfo), m_samplerFilePath(samplerFilePath) {
	resize(640, 360);
	setWindowTitle("NutshellEngine - Sampler File - " + QString::fromStdString(samplerFilePath));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_menuBar = new QMenuBar(this);
	m_fileMenu = m_menuBar->addMenu("File");
	m_fileSaveAction = m_fileMenu->addAction("Save", this, &SamplerNtspFileWidget::save);
	m_fileSaveAction->setShortcut(QKeySequence::fromString("Ctrl+S"));

	setLayout(new QVBoxLayout());
	QMargins contentMargins = layout()->contentsMargins();
	contentMargins.setTop(contentMargins.top() + 10);
	layout()->setContentsMargins(contentMargins);
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
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
	anisotropyLevelWidget = new ScalarWidget(globalInfo, "Anisotropy Level");


	std::fstream optionsFile(samplerFilePath, std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + samplerFilePath + "\" is not a valid JSON file.");
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + samplerFilePath + "\" cannot be opened.");
		return;
	}

	optionsFile = std::fstream(samplerFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("magFilter")) {
		std::string magFilter = j["magFilter"];
		int index = magFilterWidget->comboBox->findText(QString::fromStdString(magFilter));
		if (index != -1) {
			magFilterWidget->comboBox->setCurrentIndex(index);
		}
	}
	if (j.contains("minFilter")) {
		std::string minFilter = j["minFilter"];
		int index = minFilterWidget->comboBox->findText(QString::fromStdString(minFilter));
		if (index != -1) {
			minFilterWidget->comboBox->setCurrentIndex(index);
		}
	}
	if (j.contains("mipmapFilter")) {
		std::string mipmapFilter = j["mipmapFilter"];
		int index = mipmapFilterWidget->comboBox->findText(QString::fromStdString(mipmapFilter));
		if (index != -1) {
			mipmapFilterWidget->comboBox->setCurrentIndex(index);
		}
	}
	if (j.contains("addressModeU")) {
		std::string addressModeU = j["addressModeU"];
		int index = addressModeUWidget->comboBox->findText(QString::fromStdString(addressModeU));
		if (index != -1) {
			addressModeUWidget->comboBox->setCurrentIndex(index);
		}
	}
	if (j.contains("addressModeV")) {
		std::string addressModeV = j["addressModeV"];
		int index = addressModeVWidget->comboBox->findText(QString::fromStdString(addressModeV));
		if (index != -1) {
			addressModeVWidget->comboBox->setCurrentIndex(index);
		}
	}
	if (j.contains("addressModeW")) {
		std::string addressModeW = j["addressModeW"];
		int index = addressModeWWidget->comboBox->findText(QString::fromStdString(addressModeW));
		if (index != -1) {
			addressModeWWidget->comboBox->setCurrentIndex(index);
		}
	}
	if (j.contains("borderColor")) {
		std::string borderColor = j["borderColor"];
		int index = borderColorWidget->comboBox->findText(QString::fromStdString(borderColor));
		if (index != -1) {
			borderColorWidget->comboBox->setCurrentIndex(index);
		}
	}
	if (j.contains("anistropyLevel")) {
		float anisotropyLevel = j["anistropyLevel"];
		anisotropyLevelWidget->value = anisotropyLevel;
		anisotropyLevelWidget->valueLineEdit->setText(QString::number(anisotropyLevel));
	}

	connect(magFilterWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(minFilterWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(mipmapFilterWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(addressModeUWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(addressModeVWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(addressModeWWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(borderColorWidget, &ComboBoxWidget::elementSelected, this, &SamplerNtspFileWidget::onValueChanged);
	connect(anisotropyLevelWidget, &ScalarWidget::valueChanged, this, &SamplerNtspFileWidget::onValueChanged);
}

void SamplerNtspFileWidget::onValueChanged() {
	SaveTitleChanger::change(this);
}

void SamplerNtspFileWidget::save() {
	nlohmann::json j;
	if (magFilterWidget->comboBox->currentText() != "") {
		j["magFilter"] = magFilterWidget->comboBox->currentText().toStdString();
	}
	if (minFilterWidget->comboBox->currentText() != "") {
		j["minFilter"] = minFilterWidget->comboBox->currentText().toStdString();
	}
	if (mipmapFilterWidget->comboBox->currentText() != "") {
		j["mipmapFilter"] = mipmapFilterWidget->comboBox->currentText().toStdString();
	}
	if (addressModeUWidget->comboBox->currentText() != "") {
		j["addressModeU"] = addressModeUWidget->comboBox->currentText().toStdString();
	}
	if (addressModeVWidget->comboBox->currentText() != "") {
		j["addressModeV"] = addressModeVWidget->comboBox->currentText().toStdString();
	}
	if (addressModeWWidget->comboBox->currentText() != "") {
		j["addressModeW"] = addressModeWWidget->comboBox->currentText().toStdString();
	}
	if (borderColorWidget->comboBox->currentText() != "") {
		j["borderColor"] = borderColorWidget->comboBox->currentText().toStdString();
	}
	j["anisotropyLevel"] = anisotropyLevelWidget->value;

	std::fstream optionsFile(m_samplerFilePath, std::ios::out | std::ios::trunc);
	optionsFile << j.dump(1, '\t');

	SaveTitleChanger::reset(this);
}
