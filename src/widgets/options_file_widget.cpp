#include "options_file_widget.h"
#include "../common/save_title_changer.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>
#include <cstdlib>
#include <cmath>

OptionsFileWidget::OptionsFileWidget(GlobalInfo& globalInfo, const std::string& optionsFilePath) : m_globalInfo(globalInfo), m_optionsFilePath(optionsFilePath) {
	resize(640, 360);
	setWindowTitle("NutshellEngine - Options File - " + QString::fromStdString(optionsFilePath));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_menuBar = new QMenuBar(this);
	m_fileMenu = m_menuBar->addMenu("File");
	m_fileSaveAction = m_fileMenu->addAction("Save", this, &OptionsFileWidget::save);
	m_fileSaveAction->setShortcut(QKeySequence::fromString("Ctrl+S"));

	setLayout(new QVBoxLayout());
	QMargins contentMargins = layout()->contentsMargins();
	contentMargins.setTop(contentMargins.top() + 10);
	layout()->setContentsMargins(contentMargins);
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	windowTitleWidget = new StringWidget(globalInfo, "Window Title");
	layout()->addWidget(windowTitleWidget);
	windowIconImageWidget = new FileSelectorWidget(m_globalInfo, "No window icon selected", "Select a window icon", m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(windowIconImageWidget);
	maxFPSWidget = new IntegerWidget(m_globalInfo, "Max FPS");
	layout()->addWidget(maxFPSWidget);
	firstSceneWidget = new FileSelectorWidget(m_globalInfo, "No first scene selected", "Select a first scene", m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(firstSceneWidget);
	startProfilingWidget = new BooleanWidget(m_globalInfo, "Start Profiling");
	layout()->addWidget(startProfilingWidget);

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
		windowTitleWidget->value = windowTitle;
		windowTitleWidget->valueLineEdit->setText(QString::fromStdString(windowTitle));
	}
	if (j.contains("windowIconImagePath")) {
		std::string windowIconImagePath = j["windowIconImagePath"];
		windowIconImageWidget->filePathButton->path = windowIconImagePath;
		windowIconImageWidget->filePathLabel->setText(QString::fromStdString(windowIconImagePath.substr(windowIconImagePath.rfind('/') + 1)));
		windowIconImageWidget->filePathLabel->setToolTip(QString::fromStdString(windowIconImagePath));
	}
	if (j.contains("maxFPS")) {
		float maxFPSFloat = j["maxFPS"];
		int maxFPS = static_cast<int>(std::floor(maxFPSFloat));
		maxFPSWidget->value = maxFPS;
		maxFPSWidget->valueLineEdit->setText(QString::number(maxFPS));
	}
	if (j.contains("firstScenePath")) {
		std::string firstScenePath = j["firstScenePath"];
		firstSceneWidget->filePathButton->path = firstScenePath;
		firstSceneWidget->filePathLabel->setText(QString::fromStdString(firstScenePath.substr(firstScenePath.rfind('/') + 1)));
		firstSceneWidget->filePathLabel->setToolTip(QString::fromStdString(firstScenePath));
	}
	if (j.contains("startProfiling")) {
		bool startProfiling = j["startProfiling"]; 
		{
			const QSignalBlocker signalBlocker(startProfilingWidget->checkBox);
			startProfilingWidget->checkBox->setChecked(startProfiling);
		}
	}

	connect(windowTitleWidget, &StringWidget::valueChanged, this, &OptionsFileWidget::valueChanged);
	connect(windowIconImageWidget, &FileSelectorWidget::fileSelected, this, &OptionsFileWidget::valueChanged);
	connect(maxFPSWidget, &IntegerWidget::valueChanged, this, &OptionsFileWidget::valueChanged);
	connect(firstSceneWidget, &FileSelectorWidget::fileSelected, this, &OptionsFileWidget::valueChanged);
	connect(startProfilingWidget, &BooleanWidget::stateChanged, this, &OptionsFileWidget::valueChanged);
}

void OptionsFileWidget::valueChanged() {
	SaveTitleChanger::change(this);
}

void OptionsFileWidget::save() {
	nlohmann::json j;
	if (windowTitleWidget->value != "") {
		j["windowTitle"] = windowTitleWidget->value;
	}
	if (windowIconImageWidget->filePathButton->path != "") {
		std::string windowIconImagePath = windowIconImageWidget->filePathButton->path;
		std::replace(windowIconImagePath.begin(), windowIconImagePath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(windowIconImagePath).is_absolute()) {
				if (windowIconImagePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					windowIconImagePath = windowIconImagePath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["windowIconImagePath"] = windowIconImagePath;
	}
	if (maxFPSWidget->value != 0) {
		j["maxFPS"] = maxFPSWidget->value;
	}
	if (firstSceneWidget->filePathButton->path != "") {
		std::string firstScenePath = firstSceneWidget->filePathButton->path;
		std::replace(firstScenePath.begin(), firstScenePath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(firstScenePath).is_absolute()) {
				if (firstScenePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					firstScenePath = firstScenePath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["firstScenePath"] = firstScenePath;
	}
	j["startProfiling"] = startProfilingWidget->checkBox->isChecked();
	std::fstream optionsFile(m_optionsFilePath, std::ios::out | std::ios::trunc);
	optionsFile << j.dump(1, '\t');

	SaveTitleChanger::reset(this);
}