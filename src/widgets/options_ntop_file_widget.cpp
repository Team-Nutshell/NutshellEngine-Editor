#include "options_ntop_file_widget.h"
#include "../common/asset_helper.h"
#include "../common/save_title_changer.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>
#include <cstdlib>
#include <cmath>

OptionsNtopFileWidget::OptionsNtopFileWidget(GlobalInfo& globalInfo, const std::string& optionsFilePath) : m_globalInfo(globalInfo), m_optionsFilePath(optionsFilePath) {
	resize(640, 360);
	setWindowTitle("NutshellEngine - Options File - " + QString::fromStdString(optionsFilePath));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_menuBar = new QMenuBar(this);
	m_fileMenu = m_menuBar->addMenu("File");
	m_fileSaveAction = m_fileMenu->addAction("Save", this, &OptionsNtopFileWidget::save);
	m_fileSaveAction->setShortcut(QKeySequence::fromString("Ctrl+S"));

	setLayout(new QVBoxLayout());
	QMargins contentMargins = layout()->contentsMargins();
	contentMargins.setTop(contentMargins.top() + 10);
	layout()->setContentsMargins(contentMargins);
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
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

	std::fstream optionsFile(optionsFilePath, std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + optionsFilePath + "\" is not a valid JSON file.");
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + optionsFilePath + "\" cannot be opened.");
		return;
	}

	optionsFile = std::fstream(optionsFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("windowTitle")) {
		std::string windowTitle = j["windowTitle"];
		windowTitleWidget->setText(windowTitle);
	}
	if (j.contains("windowIconImagePath")) {
		std::string windowIconImagePath = j["windowIconImagePath"];
		windowIconImageWidget->setPath(windowIconImagePath);
	}
	if (j.contains("maxFPS")) {
		float maxFPSFloat = j["maxFPS"];
		int maxFPS = static_cast<int>(std::floor(maxFPSFloat));
		maxFPSWidget->setValue(maxFPS);
	}
	if (j.contains("firstScenePath")) {
		std::string firstScenePath = j["firstScenePath"];
		firstSceneWidget->setPath(firstScenePath);
	}
	if (j.contains("startProfiling")) {
		bool startProfiling = j["startProfiling"]; 
		{
			const QSignalBlocker signalBlocker(startProfilingWidget->checkBox);
			startProfilingWidget->checkBox->setChecked(startProfiling);
		}
	}
}

void OptionsNtopFileWidget::onValueChanged() {
	QObject* senderWidget = sender();
	if (senderWidget == windowIconImageWidget) {
		std::string iconImagePath = AssetHelper::absoluteToRelative(windowIconImageWidget->getPath(), m_globalInfo.projectDirectory);
		windowIconImageWidget->setPath(iconImagePath);
	}
	else if (senderWidget == firstSceneWidget) {
		std::string firstScenePath = AssetHelper::absoluteToRelative(firstSceneWidget->getPath(), m_globalInfo.projectDirectory);
		firstSceneWidget->setPath(firstScenePath);
	}

	SaveTitleChanger::change(this);
}

void OptionsNtopFileWidget::save() {
	nlohmann::json j;
	if (windowTitleWidget->getText() != "") {
		j["windowTitle"] = windowTitleWidget->getText();
	}
	if (windowIconImageWidget->getPath() != "") {
		j["windowIconImagePath"] = windowIconImageWidget->getPath();
	}
	j["maxFPS"] = maxFPSWidget->getValue();
	if (firstSceneWidget->getPath() != "") {
		j["firstScenePath"] = firstSceneWidget->getPath();
	}
	j["startProfiling"] = startProfilingWidget->checkBox->isChecked();

	std::fstream optionsFile(m_optionsFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		optionsFile << "{\n}";
	}
	else {
		optionsFile << j.dump(1, '\t');
	}

	SaveTitleChanger::reset(this);
}
