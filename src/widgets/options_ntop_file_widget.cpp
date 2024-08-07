#include "options_ntop_file_widget.h"
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
	layout()->addWidget(maxFPSWidget);
	firstSceneWidget = new FileSelectorWidget(m_globalInfo, "First Scene", "No first scene selected", m_globalInfo.projectDirectory + "/assets");
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
		windowIconImageWidget->filePathButton->setText(QString::fromStdString(windowIconImagePath.substr(windowIconImagePath.rfind('/') + 1)));
		windowIconImageWidget->filePathButton->setToolTip(QString::fromStdString(windowIconImagePath));
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
		firstSceneWidget->filePathButton->setText(QString::fromStdString(firstScenePath.substr(firstScenePath.rfind('/') + 1)));
		firstSceneWidget->filePathButton->setToolTip(QString::fromStdString(firstScenePath));
	}
	if (j.contains("startProfiling")) {
		bool startProfiling = j["startProfiling"]; 
		{
			const QSignalBlocker signalBlocker(startProfilingWidget->checkBox);
			startProfilingWidget->checkBox->setChecked(startProfiling);
		}
	}

	connect(windowTitleWidget, &StringWidget::valueChanged, this, &OptionsNtopFileWidget::onValueChanged);
	connect(windowIconImageWidget, &FileSelectorWidget::fileSelected, this, &OptionsNtopFileWidget::onValueChanged);
	connect(maxFPSWidget, &IntegerWidget::valueChanged, this, &OptionsNtopFileWidget::onValueChanged);
	connect(firstSceneWidget, &FileSelectorWidget::fileSelected, this, &OptionsNtopFileWidget::onValueChanged);
	connect(startProfilingWidget, &BooleanWidget::stateChanged, this, &OptionsNtopFileWidget::onValueChanged);
}

void OptionsNtopFileWidget::onValueChanged() {
	QObject* senderWidget = sender();
	if (senderWidget == windowIconImageWidget) {
		std::string iconImagePath = windowIconImageWidget->filePathButton->path;
		if (!iconImagePath.empty()) {
			std::replace(iconImagePath.begin(), iconImagePath.end(), '\\', '/');
			if (m_globalInfo.projectDirectory != "") {
				if (std::filesystem::path(iconImagePath).is_absolute()) {
					if (iconImagePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
						iconImagePath = iconImagePath.substr(m_globalInfo.projectDirectory.size() + 1);
					}
				}
			}
			windowIconImageWidget->filePathButton->path = iconImagePath;
			windowIconImageWidget->filePathButton->setText(QString::fromStdString(iconImagePath.substr(iconImagePath.rfind('/') + 1)));
			windowIconImageWidget->filePathButton->setToolTip(QString::fromStdString(iconImagePath));
		}
	}
	else if (senderWidget == firstSceneWidget) {
		std::string firstScenePath = firstSceneWidget->filePathButton->path;
		if (!firstScenePath.empty()) {
			std::replace(firstScenePath.begin(), firstScenePath.end(), '\\', '/');
			if (m_globalInfo.projectDirectory != "") {
				if (std::filesystem::path(firstScenePath).is_absolute()) {
					if (firstScenePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
						firstScenePath = firstScenePath.substr(m_globalInfo.projectDirectory.size() + 1);
					}
				}
			}
			firstSceneWidget->filePathButton->path = firstScenePath;
			firstSceneWidget->filePathButton->setText(QString::fromStdString(firstScenePath.substr(firstScenePath.rfind('/') + 1)));
			firstSceneWidget->filePathButton->setToolTip(QString::fromStdString(firstScenePath));
		}
	}

	SaveTitleChanger::change(this);
}

void OptionsNtopFileWidget::save() {
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
	if (maxFPSWidget->value > 0) {
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
	if (j.empty()) {
		optionsFile << "{\n}";
	}
	else {
		optionsFile << j.dump(1, '\t');
	}

	SaveTitleChanger::reset(this);
}
