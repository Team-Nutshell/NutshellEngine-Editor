#include "options_ntop_file_widget.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include "../undo_commands/select_asset_entities_command.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>
#include <cstdlib>
#include <cmath>

OptionsNtopFileWidget::OptionsNtopFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(2, 0, 2, 0);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_options_file"))));
	windowTitleWidget = new StringWidget(globalInfo, m_globalInfo.localization.getString("assets_options_window_title"));
	layout()->addWidget(windowTitleWidget);
	windowIconImageWidget = new FileSelectorWidget(m_globalInfo, m_globalInfo.localization.getString("assets_options_window_icon"), m_globalInfo.localization.getString("assets_options_no_window_icon_selected"), m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(windowIconImageWidget);
	maxFPSWidget = new IntegerWidget(m_globalInfo, m_globalInfo.localization.getString("assets_options_max_fps"));
	maxFPSWidget->setMin(0);
	layout()->addWidget(maxFPSWidget);
	firstSceneWidget = new FileSelectorWidget(m_globalInfo, m_globalInfo.localization.getString("assets_options_first_scene"), m_globalInfo.localization.getString("assets_options_no_first_scene_selected"), m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(firstSceneWidget);
	startProfilingWidget = new BooleanWidget(m_globalInfo, m_globalInfo.localization.getString("assets_options_start_profiling"));
	layout()->addWidget(startProfilingWidget);

	connect(windowTitleWidget, &StringWidget::valueChanged, this, &OptionsNtopFileWidget::onValueChanged);
	connect(windowIconImageWidget, &FileSelectorWidget::fileSelected, this, &OptionsNtopFileWidget::onValueChanged);
	connect(maxFPSWidget, &IntegerWidget::valueChanged, this, &OptionsNtopFileWidget::onValueChanged);
	connect(firstSceneWidget, &FileSelectorWidget::fileSelected, this, &OptionsNtopFileWidget::onValueChanged);
	connect(startProfilingWidget, &BooleanWidget::stateChanged, this, &OptionsNtopFileWidget::onValueChanged);
}

void OptionsNtopFileWidget::setPath(const std::string& path) {
	m_optionsFilePath = path;
	std::fstream optionsFile(m_optionsFilePath, std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_is_not_valid_json", { m_optionsFilePath }));
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_cannot_be_opened", { m_optionsFilePath }));
		return;
	}

	optionsFile = std::fstream(m_optionsFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	optionsNtop = OptionsNtop();

	if (j.contains("windowTitle")) {
		optionsNtop.windowTitle = j["windowTitle"];
	}
	if (j.contains("windowIconImagePath")) {
		optionsNtop.windowIconImagePath = j["windowIconImagePath"];
	}
	if (j.contains("maxFPS")) {
		optionsNtop.maxFPS = j["maxFPS"];
	}
	if (j.contains("firstScenePath")) {
		optionsNtop.firstScenePath = j["firstScenePath"];
	}
	if (j.contains("startProfiling")) {
		optionsNtop.startProfiling = j["startProfiling"];
	}

	updateWidgets();
}

void OptionsNtopFileWidget::updateWidgets() {
	windowTitleWidget->setText(optionsNtop.windowTitle);
	windowIconImageWidget->setPath(optionsNtop.windowIconImagePath);
	maxFPSWidget->setValue(optionsNtop.maxFPS);
	firstSceneWidget->setPath(optionsNtop.firstScenePath);
	startProfilingWidget->setValue(optionsNtop.startProfiling);
}

void OptionsNtopFileWidget::save() {
	nlohmann::json j;
	if (!optionsNtop.windowTitle.empty()) {
		j["windowTitle"] = optionsNtop.windowTitle;
	}
	if (!optionsNtop.windowIconImagePath.empty()) {
		j["windowIconImagePath"] = optionsNtop.windowIconImagePath;
	}
	j["maxFPS"] = optionsNtop.maxFPS;
	if (!optionsNtop.firstScenePath.empty()) {
		j["firstScenePath"] = optionsNtop.firstScenePath;
	}
	j["startProfiling"] = optionsNtop.startProfiling;

	std::fstream optionsFile(m_optionsFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		optionsFile << "{\n}";
	}
	else {
		optionsFile << j.dump(1, '\t');
	}
}

void OptionsNtopFileWidget::onValueChanged() {
	OptionsNtop newOptionsNtop = optionsNtop;

	QObject* senderWidget = sender();
	if (senderWidget == windowTitleWidget) {
		newOptionsNtop.windowTitle = windowTitleWidget->getText();
	}
	else if (senderWidget == windowIconImageWidget) {
		std::string iconImagePath = AssetHelper::absoluteToRelative(windowIconImageWidget->getPath(), m_globalInfo.projectDirectory);
		newOptionsNtop.windowIconImagePath = iconImagePath;
	}
	else if (senderWidget == maxFPSWidget) {
		newOptionsNtop.maxFPS = maxFPSWidget->getValue();
	}
	else if (senderWidget == firstSceneWidget) {
		std::string firstScenePath = AssetHelper::absoluteToRelative(firstSceneWidget->getPath(), m_globalInfo.projectDirectory);
		newOptionsNtop.firstScenePath = firstScenePath;
	}
	else if (senderWidget == startProfilingWidget) {
		newOptionsNtop.startProfiling = startProfilingWidget->getValue();
	}

	if (newOptionsNtop != optionsNtop) {
		m_globalInfo.actionUndoStack->push(new ChangeOptionsNtopFile(m_globalInfo, newOptionsNtop, m_optionsFilePath));
	}
}

ChangeOptionsNtopFile::ChangeOptionsNtopFile(GlobalInfo& globalInfo, OptionsNtop newOptionsNtop, const std::string& filePath) : m_globalInfo(globalInfo) {
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_options", { filePath })));

	m_optionsNtopFileWidget = globalInfo.mainWindow->infoPanel->assetInfoPanel->assetInfoScrollArea->assetInfoList->optionsNtopFileWidget;
	m_oldOptionsNtop = m_optionsNtopFileWidget->optionsNtop;
	m_newOptionsNtop = newOptionsNtop;
	m_filePath = filePath;
}

void ChangeOptionsNtopFile::undo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, {}));

	m_optionsNtopFileWidget->optionsNtop = m_oldOptionsNtop;
	m_optionsNtopFileWidget->updateWidgets();

	m_optionsNtopFileWidget->save();
}

void ChangeOptionsNtopFile::redo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, {}));

	m_optionsNtopFileWidget->optionsNtop = m_newOptionsNtop;
	m_optionsNtopFileWidget->updateWidgets();

	m_optionsNtopFileWidget->save();
}