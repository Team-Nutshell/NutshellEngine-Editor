#include "options_ntop_file_widget.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>
#include <cstdlib>
#include <cmath>

OptionsNtopFileWidget::OptionsNtopFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	windowTitleWidget = new StringWidget(globalInfo, "Window Title");
	layout()->addWidget(windowTitleWidget);
	windowIconImageWidget = new FileSelectorWidget(m_globalInfo, "Window Icon", "No window icon selected", m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(windowIconImageWidget);
	maxFPSWidget = new IntegerWidget(m_globalInfo, "Max FPS");
	maxFPSWidget->setMin(0);
	layout()->addWidget(maxFPSWidget);
	firstSceneWidget = new FileSelectorWidget(m_globalInfo, "First Scene", "No first scene selected", m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(firstSceneWidget);
	startProfilingWidget = new BooleanWidget(m_globalInfo, "Start Profiling");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + m_optionsFilePath + "\" is not a valid JSON file.");
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + m_optionsFilePath + "\" cannot be opened.");
		return;
	}

	optionsFile = std::fstream(m_optionsFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("windowTitle")) {
		std::string windowTitle = j["windowTitle"];
		optionsNtop.windowTitle = windowTitle;
	}
	if (j.contains("windowIconImagePath")) {
		std::string windowIconImagePath = j["windowIconImagePath"];
		optionsNtop.windowIconImagePath = windowIconImagePath;
	}
	if (j.contains("maxFPS")) {
		float maxFPSFloat = j["maxFPS"];
		int maxFPS = static_cast<int>(std::floor(maxFPSFloat));
		optionsNtop.maxFPS = maxFPS;
	}
	if (j.contains("firstScenePath")) {
		std::string firstScenePath = j["firstScenePath"];
		optionsNtop.firstScenePath = firstScenePath;
	}
	if (j.contains("startProfiling")) {
		bool startProfiling = j["startProfiling"];
		optionsNtop.startProfiling = startProfiling;
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
		m_globalInfo.undoStack->push(new ChangeOptionsNtopFile(m_globalInfo, newOptionsNtop, m_optionsFilePath));
	}
}

ChangeOptionsNtopFile::ChangeOptionsNtopFile(GlobalInfo& globalInfo, OptionsNtop newOptionsNtop, const std::string& filePath) : m_globalInfo(globalInfo) {
	setText("Change Options Ntop");

	m_optionsNtopFileWidget = globalInfo.mainWindow->infoPanel->assetInfoPanel->assetInfoScrollArea->assetInfoList->optionsNtopFileWidget;
	m_oldOptionsNtop = m_optionsNtopFileWidget->optionsNtop;
	m_newOptionsNtop = newOptionsNtop;
	m_filePath = filePath;
}

void ChangeOptionsNtopFile::undo() {
	emit m_globalInfo.signalEmitter.selectAssetSignal(m_filePath);

	m_optionsNtopFileWidget->optionsNtop = m_oldOptionsNtop;
	m_optionsNtopFileWidget->updateWidgets();

	m_optionsNtopFileWidget->save();
}

void ChangeOptionsNtopFile::redo() {
	emit m_globalInfo.signalEmitter.selectAssetSignal(m_filePath);

	m_optionsNtopFileWidget->optionsNtop = m_newOptionsNtop;
	m_optionsNtopFileWidget->updateWidgets();

	m_optionsNtopFileWidget->save();
}