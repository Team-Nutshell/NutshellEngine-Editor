#include "build_bar.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../common/scene_manager.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <regex>
#include <algorithm>
#include <vector>
#include <utility>
#include <tuple>
#if defined(NTSHENGN_OS_WINDOWS)
#include <windows.h>
#elif defined(NTSHENGN_OS_LINUX) || defined(NTSHENGN_OS_FREEBSD)
#include <stdio.h>
#endif
#include <cctype>

BuildBar::BuildBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 5, 0, 0);
	layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);
	buildAndRunButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("header_project_build_and_run")));
	layout()->addWidget(buildAndRunButton);
	std::vector<std::string> buildTypeList{ "Debug", "Release" };
	buildTypeComboBox = new ComboBoxWidget(m_globalInfo, m_globalInfo.localization.getString("build_type"), buildTypeList);
	layout()->addWidget(buildTypeComboBox);
	exportButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("build_export")));
	layout()->addWidget(exportButton);

	connect(buildAndRunButton, &QPushButton::clicked, this, &BuildBar::launchBuild);
	connect(exportButton, &QPushButton::clicked, this, &BuildBar::launchExport);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::startBuildAndRunSignal, this, &BuildBar::onBuildRunExportStarted);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::endBuildAndRunSignal, this, &BuildBar::onBuildRunExportEnded);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::startExportSignal, this, &BuildBar::onBuildRunExportStarted);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::endExportSignal, this, &BuildBar::onBuildRunExportEnded);
}

void BuildBar::launchBuild() {
	if (!m_globalInfo.currentScenePath.empty()) {
		SceneManager::saveScene(m_globalInfo, m_globalInfo.currentScenePath);
		SaveTitleChanger::reset(m_globalInfo.mainWindow);
	}
	std::thread buildThread([this]() {
		emit m_globalInfo.signalEmitter.startBuildAndRunSignal();
		if (build()) {
			run();
		}
		emit m_globalInfo.signalEmitter.endBuildAndRunSignal();
		});
	buildThread.detach();
}

void BuildBar::launchExport() {
	QFileDialog fileDialog = QFileDialog();
	fileDialog.setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("build_export")));
	fileDialog.setWindowIcon(QIcon("assets/icon.png"));
	fileDialog.setFileMode(QFileDialog::FileMode::Directory);
	fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory));
	if (fileDialog.exec()) {
		if (!m_globalInfo.currentScenePath.empty()) {
			SceneManager::saveScene(m_globalInfo, m_globalInfo.currentScenePath);
			SaveTitleChanger::reset(m_globalInfo.mainWindow);
		}

		std::string exportDirectory = fileDialog.selectedFiles()[0].toStdString();

		std::thread buildThread([this, exportDirectory]() {
			emit m_globalInfo.signalEmitter.startExportSignal();
			if (build()) {
				exportApplication(exportDirectory);
			}
			emit m_globalInfo.signalEmitter.endExportSignal();
			});
		buildThread.detach();
	}
}

bool BuildBar::build() {
	generateScriptManager();

	const std::string buildType = buildTypeComboBox->comboBox->currentText().toStdString();
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_launching_build", { buildType }));

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build");
	}

	// Clear assets directory
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build/" + buildType + "/assets")) {
		std::filesystem::remove_all(m_globalInfo.projectDirectory + "/editor_build/" + buildType + "/assets");
	}

	// Clear modules directory
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build/" + buildType + "/modules")) {
		std::filesystem::remove_all(m_globalInfo.projectDirectory + "/editor_build/" + buildType + "/modules");
	}

	// Set current path
	const std::string previousCurrentPath = std::filesystem::current_path().string();
	std::filesystem::current_path(m_globalInfo.projectDirectory + "/editor_build");

	bool buildSuccess = true;
#if defined(NTSHENGN_OS_WINDOWS)
	HANDLE pipeRead = NULL;
	HANDLE pipeWrite = NULL;
	DWORD exitCode;

	// CMake
	SECURITY_ATTRIBUTES securityAttributes;
	ZeroMemory(&securityAttributes, sizeof(SECURITY_ATTRIBUTES));
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;

	CreatePipe(&pipeRead, &pipeWrite, &securityAttributes, 0);
	SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA startupInfo;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdOutput = pipeWrite;
	startupInfo.hStdError = pipeWrite;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION processInformation;
	ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));

	const std::string cMakeCommand = m_globalInfo.editorParameters.build.cMakePath + " " + m_globalInfo.projectDirectory + " -DNTSHENGN_COMMON_PATH=" + m_globalInfo.projectDirectory + "/Common -DNTSHENGN_BUILD_IN_EDITOR=ON";
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_launching_cmake_command", { cMakeCommand }));
	bool cMakeSuccess = true;
	if (CreateProcessA(NULL, const_cast<char*>(cMakeCommand.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_cmake_logs"));
		CHAR stdOutBuffer[4096];
		DWORD bytesRead;
		while (ReadFile(pipeRead, stdOutBuffer, 4096, &bytesRead, NULL)) {
			addLog(std::string(stdOutBuffer, bytesRead));
		}

		WaitForSingleObject(processInformation.hProcess, INFINITE);

		GetExitCodeProcess(processInformation.hProcess, &exitCode);

		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);

		if (exitCode == 0) {
			m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_cmake_success"));
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_build_cmake_error"));

			cMakeSuccess = false;
		}
	}
	else {
		CloseHandle(pipeWrite);
		CloseHandle(pipeRead);
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_build_cmake_cannot_launch"));

		cMakeSuccess = false;
	}

	if (!cMakeSuccess) {
		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return false;
	}

	CloseHandle(pipeRead);

	// Build
	ZeroMemory(&securityAttributes, sizeof(SECURITY_ATTRIBUTES));
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;

	CreatePipe(&pipeRead, &pipeWrite, &securityAttributes, 0);
	SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0);

	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdOutput = pipeWrite;
	startupInfo.hStdError = pipeWrite;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_HIDE;

	ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));

	const std::string cMakeBuildCommand = m_globalInfo.editorParameters.build.cMakePath + " --build . --config " + buildType;
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_launching_build_command", { buildType, cMakeBuildCommand }));
	if (CreateProcessA(NULL, const_cast<char*>(cMakeBuildCommand.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_build_logs"));
		CHAR stdOutBuffer[4096];
		DWORD bytesRead;
		while (ReadFile(pipeRead, stdOutBuffer, 4096, &bytesRead, NULL)) {
			addLog(std::string(stdOutBuffer, bytesRead));
		}

		WaitForSingleObject(processInformation.hProcess, INFINITE);

		GetExitCodeProcess(processInformation.hProcess, &exitCode);

		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);

		if (exitCode == 0) {
			m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_build_success"));
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_build_build_error"));

			buildSuccess = false;
		}
	}
	else {
		CloseHandle(pipeWrite);
		CloseHandle(pipeRead);
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_build_cmake_cannot_launch"));

		buildSuccess = false;
	}

	if (buildSuccess) {
		CloseHandle(pipeRead);
	}
#elif defined(NTSHENGN_OS_LINUX) || defined(NTSHENGN_OS_FREEBSD)
	// CMake
	const std::string cMakeCommand = m_globalInfo.editorParameters.build.cMakePath + " " + m_globalInfo.projectDirectory + " -DNTSHENGN_COMMON_PATH=" + m_globalInfo.projectDirectory + "/Common -DCMAKE_BUILD_TYPE=" + buildType + " -DNTSHENGN_BUILD_IN_EDITOR=ON 2>&1";
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_launching_cmake_command", { cMakeCommand }));
	bool cMakeSuccess = true;
	FILE* fp = popen(cMakeCommand.c_str(), "r");
	if (fp == NULL) {
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_build_cmake_cannot_launch"));

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return false;
	}
	
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_cmake_logs"));
	char stdOutBuffer[4096];
	while (fgets(stdOutBuffer, 4096, fp) != NULL) {
		addLog(std::string(stdOutBuffer));
	}

	if (pclose(fp) == 0) {
		m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_cmake_success"));
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_build_cmake_error"));

		cMakeSuccess = false;
	}

	if (!cMakeSuccess) {
		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return false;
	}

	// Build
	const std::string cMakeBuildCommand = m_globalInfo.editorParameters.build.cMakePath + " --build . --config " + buildType + " 2>&1";
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_launching_build_command", { buildType, cMakeBuildCommand }));
	fp = popen(cMakeBuildCommand.c_str(), "r");
	if (fp == NULL) {
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_build_cmake_cannot_launch"));

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return false;
	}

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);
	
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_build_logs"));
	while (fgets(stdOutBuffer, 4096, fp) != NULL) {
		addLog(std::string(stdOutBuffer));
	}

	if (pclose(fp) == 0) {
		m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_build_build_success"));
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_build_build_error"));

		buildSuccess = false;
	}

	std::string scriptsLibrary = "libNutshellEngine-Scripts.so";
	std::filesystem::copy(m_globalInfo.projectDirectory + "/editor_build/" + scriptsLibrary, m_globalInfo.projectDirectory + "/editor_build/" + buildType, std::filesystem::copy_options::overwrite_existing);
	std::string buildAssetsDirectory = buildType + "/assets";
	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build/" + buildAssetsDirectory)) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build/" + buildAssetsDirectory);
	}
	std::filesystem::copy(m_globalInfo.projectDirectory + "/editor_build/assets", m_globalInfo.projectDirectory + "/editor_build/" + buildAssetsDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);
#endif

	return buildSuccess;
}

void BuildBar::run() {
	const std::string buildType = buildTypeComboBox->comboBox->currentText().toStdString();
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_launching_run"));

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build");
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_run_no_build_to_run"));

		return;
	}

	// Copy runtime
	std::filesystem::copy("assets/runtime/" + buildType, m_globalInfo.projectDirectory + "/editor_build/" + buildType, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	// Set current path
	const std::string previousCurrentPath = std::filesystem::current_path().string();
	std::filesystem::current_path(m_globalInfo.projectDirectory + "/editor_build");

	const std::regex syntaxSugarRegex(R"(\x1B\[[0-9]*?m)");

#if defined(NTSHENGN_OS_WINDOWS)
	HANDLE pipeRead = NULL;
	HANDLE pipeWrite = NULL;
	DWORD exitCode;

	SECURITY_ATTRIBUTES securityAttributes;
	ZeroMemory(&securityAttributes, sizeof(SECURITY_ATTRIBUTES));
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;

	CreatePipe(&pipeRead, &pipeWrite, &securityAttributes, 0);
	SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA startupInfo;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdOutput = pipeWrite;
	startupInfo.hStdError = pipeWrite;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_SHOW;

	PROCESS_INFORMATION processInformation;
	ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));

	std::filesystem::current_path(buildType);
	const std::string runCommand = "NutshellEngine.exe";
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_launching_run_command", { runCommand }));
	if (CreateProcessA(NULL, const_cast<char*>(runCommand.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_application_logs"));
		CHAR stdOutBuffer[4096];
		DWORD bytesRead;
		while (ReadFile(pipeRead, stdOutBuffer, 4096, &bytesRead, NULL)) {
			std::string log = std::string(stdOutBuffer, bytesRead);

			std::stringstream syntaxSugarRegexResult;
			std::regex_replace(std::ostream_iterator<char>(syntaxSugarRegexResult), log.begin(), log.end(), syntaxSugarRegex, "");

			addLog(syntaxSugarRegexResult.str());
		}

		WaitForSingleObject(processInformation.hProcess, INFINITE);

		GetExitCodeProcess(processInformation.hProcess, &exitCode);

		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);

		if (exitCode == 0) {
			m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_close_success"));
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_run_close_error"));
		}
	}
	else {
		CloseHandle(pipeWrite);
		CloseHandle(pipeRead);
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_run_cannot_launch"));

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return;
	}
	CloseHandle(pipeRead);
#elif defined(NTSHENGN_OS_LINUX) || defined(NTSHENGN_OS_FREEBSD)
	std::filesystem::current_path(buildType);
	const std::string runCommand = "./NutshellEngine 2>&1";
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_launching_run_command", { runCommand }));
	FILE* fp = popen(runCommand.c_str(), "r");
	if (fp == NULL) {
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_run_cannot_launch"));
	}

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);
	
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_application_logs"));
	char stdOutBuffer[4096];
	while (fgets(stdOutBuffer, 4096, fp) != NULL) {
		std::string log = std::string(stdOutBuffer);

		std::stringstream syntaxSugarRegexResult;
		std::regex_replace(std::ostream_iterator<char>(syntaxSugarRegexResult), log.begin(), log.end(), syntaxSugarRegex, "");

		addLog(syntaxSugarRegexResult.str());
	}

	if (pclose(fp) == 0) {
		m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_close_success"));
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_run_close_error"));
	}
#endif
}

void BuildBar::exportApplication(const std::string& exportDirectory) {
	const std::string buildType = buildTypeComboBox->comboBox->currentText().toStdString();
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_export_launching_export"));

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build");
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_run_no_build_to_export"));

		return;
	}

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build/export_tmp")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build/export_tmp");
	}

	std::string projectNameNoSpace = m_globalInfo.projectName;
	std::replace(projectNameNoSpace.begin(), projectNameNoSpace.end(), ' ', '_');

	// Copy runtime
	std::filesystem::copy("assets/runtime/" + buildType, m_globalInfo.projectDirectory + "/editor_build/" + buildType, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	// Copy to another directory
	const std::string tmpExportDirectory = m_globalInfo.projectDirectory + "/editor_build/export_tmp/" + projectNameNoSpace;
	std::filesystem::create_directory(tmpExportDirectory);
	std::filesystem::copy(m_globalInfo.projectDirectory + "/editor_build/" + buildType, tmpExportDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	// Set current path
	const std::string previousCurrentPath = std::filesystem::current_path().string();
	std::filesystem::current_path(m_globalInfo.projectDirectory + "/editor_build");

#if defined(NTSHENGN_OS_WINDOWS)
	// Remove some files
	const std::string scriptsExp = tmpExportDirectory + "/NutshellEngine-Scripts.exp";
	if (std::filesystem::exists(scriptsExp)) {
		std::filesystem::remove(scriptsExp);
	}
	const std::string scriptsLib = tmpExportDirectory + "/NutshellEngine-Scripts.lib";
	if (std::filesystem::exists(scriptsLib)) {
		std::filesystem::remove(scriptsLib);
	}
	const std::string scriptsPdb = tmpExportDirectory + "/NutshellEngine-Scripts.pdb";
	if (std::filesystem::exists(scriptsPdb)) {
		std::filesystem::remove(scriptsPdb);
	}

	const std::string exportedFileName = projectNameNoSpace + "_" + buildType + ".zip";
	const std::string exportedFullPath = exportDirectory + "/" + exportedFileName;

	// Rename executable
	std::filesystem::rename(tmpExportDirectory + "/NutshellEngine.exe", tmpExportDirectory + "/" + projectNameNoSpace + ".exe");

	HANDLE pipeRead = NULL;
	HANDLE pipeWrite = NULL;
	DWORD exitCode;

	SECURITY_ATTRIBUTES securityAttributes;
	ZeroMemory(&securityAttributes, sizeof(SECURITY_ATTRIBUTES));
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;

	CreatePipe(&pipeRead, &pipeWrite, &securityAttributes, 0);
	SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA startupInfo;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdOutput = pipeWrite;
	startupInfo.hStdError = pipeWrite;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION processInformation;
	ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));

	const std::string exportCommand = "powershell Compress-Archive -Path export_tmp/" + projectNameNoSpace + " -DestinationPath " + exportedFullPath + " -Force";
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_no_build_to_export", { exportCommand }));
	if (CreateProcessA(NULL, const_cast<char*>(exportCommand.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_export_export_logs"));
		CHAR stdOutBuffer[4096];
		DWORD bytesRead;
		while (ReadFile(pipeRead, stdOutBuffer, 4096, &bytesRead, NULL)) {
			addLog(std::string(stdOutBuffer, bytesRead));
		}
		
		WaitForSingleObject(processInformation.hProcess, INFINITE);

		GetExitCodeProcess(processInformation.hProcess, &exitCode);

		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);

		if (exitCode == 0) {
			m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_export_export_success", { exportedFullPath }));
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_export_export_error"));
		}
	}
	else {
		CloseHandle(pipeWrite);
		CloseHandle(pipeRead);
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_export_cannot_export"));

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return;
	}
	CloseHandle(pipeRead);
#elif defined(NTSHENGN_OS_LINUX) || defined(NTSHENGN_OS_FREEBSD)
	const std::string exportedFileName = projectNameNoSpace + "_" + buildType + ".tar.gz";
	const std::string exportedFullPath = exportDirectory + "/" + exportedFileName;

	std::filesystem::current_path(m_globalInfo.projectDirectory + "/editor_build/export_tmp");

	// Rename executable
	std::filesystem::rename(tmpExportDirectory + "/NutshellEngine", tmpExportDirectory + "/" + projectNameNoSpace);
	
	const std::string exportCommand = "tar -zcvf " + exportedFullPath + " " + projectNameNoSpace;
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_run_no_build_to_export", { exportCommand }));
	FILE* fp = popen(exportCommand.c_str(), "r");
	if (fp == NULL) {
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_export_cannot_export"));

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);
	}

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);

	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_export_export_logs"));
	char stdOutBuffer[4096];
	while (fgets(stdOutBuffer, 4096, fp) != NULL) {
		addLog(std::string(stdOutBuffer));
	}

	if (pclose(fp) == 0) {
		m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_export_export_success", { exportedFullPath }));
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_export_export_error"));
	}
#endif

	// Destroy temporary directory
	std::filesystem::remove_all(tmpExportDirectory);
}

void BuildBar::addLog(std::string log) {
	log.erase(log.begin(), std::find_if(log.begin(), log.end(), [](unsigned char c) {
		return !std::isspace(c);
		}));
	log.erase(std::find_if(log.rbegin(), log.rend(), [](unsigned char c) {
		return !std::isspace(c);
		}).base(), log.end());

	LogLevel logLevel = LogLevel::Info;
	const std::string infoString = "info";
	const std::string warningString = "warning";
	const std::string errorString = "error";

	std::string::iterator infoSearch = std::search(log.begin(), log.end(), infoString.begin(), infoString.end(), [](unsigned char a, unsigned char b) {
		return std::toupper(a) == std::toupper(b);
		});
	std::string::iterator warningSearch = std::search(log.begin(), log.end(), warningString.begin(), warningString.end(), [](unsigned char a, unsigned char b) {
		return std::toupper(a) == std::toupper(b);
		});
	std::string::iterator errorSearch = std::search(log.begin(), log.end(), errorString.begin(), errorString.end(), [](unsigned char a, unsigned char b) {
		return std::toupper(a) == std::toupper(b);
		});

	if (infoSearch <= warningSearch) {
		if (infoSearch <= errorSearch) {
			logLevel = LogLevel::Info;
		}
		else {
			logLevel = LogLevel::Error;
		}
	}
	else {
		if (warningSearch <= errorSearch) {
			logLevel = LogLevel::Warning;
		}
		else {
			logLevel = LogLevel::Error;
		}
	}

	if (!log.empty()) {
		m_globalInfo.logger.addLog(logLevel, log);
	}
}

std::pair<std::string, std::string> BuildBar::parseVariableLineTokens(const std::vector<std::string>& tokens, bool usingNamespaceStd, bool usingNamespaceNtshEngnMath) {
	if (tokens.size() < 2) {
		return { "" , "Unknown" };
	}
	
	std::string type = tokens[0];
	std::string name = tokens[1];
	if (type == "bool") {
		return { name, "Boolean" };
	}
	else if (type == "int8_t") {
		return { name, "Int8" };
	}
	else if (type == "int16_t") {
		return { name, "Int16" };
	}
	else if ((type == "int") || (type == "int32_t")) {
		return { name, "Int32" };
	}
	else if (type == "int64_t") {
		return { name, "Int64" };
	}
	else if (type == "uint8_t") {
		return { name, "Uint8" };
	}
	else if (type == "uint16_t") {
		return { name, "Uint16" };
	}
	else if (type == "uint32_t") {
		return { name, "Uint32" };
	}
	else if ((type == "uint64_t") || (type == "size_t")) {
		return { name, "Uint64" };
	}
	else if (type == "float") {
		return { name, "Float32" };
	}
	else if (type == "double") {
		return { name, "Float64" };
	}
	else if ((usingNamespaceStd && (type == "string")) || (!usingNamespaceStd && (type == "std::string"))) {
		return { name, "String" };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "vec2")) || (!usingNamespaceNtshEngnMath && (type == "Math::vec2"))) {
		return { name, "Vector2" };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "vec3")) || (!usingNamespaceNtshEngnMath && (type == "Math::vec3"))) {
		return { name, "Vector3" };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "vec4")) || (!usingNamespaceNtshEngnMath && (type == "Math::vec4"))) {
		return { name, "Vector4" };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "quat")) || (!usingNamespaceNtshEngnMath && (type == "Math::quat"))) {
		return { name, "Quaternion" };
	}

	return { "" , "Unknown" };
}

void BuildBar::generateScriptManager() {
	const std::string scriptDefine = "NTSHENGN_SCRIPT";
	const std::string editableVariableDefine = "NTSHENGN_EDITABLE_VARIABLE";

	std::vector<std::tuple<std::string, std::string, std::vector<std::pair<std::string, std::string>>>> scriptEntries;
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
		for (const auto& entry : std::filesystem::directory_iterator(m_globalInfo.projectDirectory + "/scripts/")) {
			if (entry.is_directory()) {
				continue;
			}

			std::fstream scriptFile(entry.path().string(), std::ios::in);
			if (scriptFile.is_open()) {
				std::string scriptContent((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
				size_t scriptNameStartPos = scriptContent.find(scriptDefine + "(");
				if (scriptNameStartPos != std::string::npos) {
					scriptNameStartPos += 16;
					size_t scriptNameLength = scriptContent.substr(scriptNameStartPos).find(")");
					if (scriptNameLength != std::string::npos) {
						std::string scriptName = scriptContent.substr(scriptNameStartPos, scriptNameLength);
						std::string scriptPath = entry.path().string().substr((m_globalInfo.projectDirectory + "/scripts/").length());
						std::vector<std::pair<std::string, std::string>> editableScriptVariables;

						bool usingNamespaceStd = scriptContent.find("using namespace std;") != std::string::npos;
						bool usingNamespaceNtshEngnMath = scriptContent.find("using namespace NtshEngn::Math;") != std::string::npos;
						std::string currentParsing = scriptContent;
						size_t editableVariableIndex;
						while ((editableVariableIndex = currentParsing.find(editableVariableDefine)) != std::string::npos) {
							currentParsing = currentParsing.substr(editableVariableIndex + editableVariableDefine.length() + 1);
							std::string variableLine = currentParsing.substr(0, currentParsing.find(';'));
							std::vector<std::string> variableLineTokens;
							size_t spacePosition = 0;
							while ((spacePosition = variableLine.find(' ')) != std::string::npos) {
								std::string variableLineToken = variableLine.substr(0, spacePosition);
								if (!variableLineToken.empty()) {
									variableLineTokens.push_back(variableLineToken);
								}
								variableLine.erase(0, spacePosition + 1);
							}
							if (!variableLine.empty()) {
								variableLineTokens.push_back(variableLine);
							}
							editableScriptVariables.push_back(parseVariableLineTokens(variableLineTokens, usingNamespaceStd, usingNamespaceNtshEngnMath));
						}

						scriptEntries.push_back({ scriptName, scriptPath, editableScriptVariables });
					}
				}
			}
		}
	}

	std::string scriptManagerFileContent = "// This file is automatically generated by the editor\n\n#include \"ntshengn_script_manager.h\"\n\n";

	for (const auto& scriptEntry : scriptEntries) {
		scriptManagerFileContent += "#include \"../" + std::get<1>(scriptEntry) + "\"\n";
	}
	if (!scriptEntries.empty()) {
		scriptManagerFileContent += "\n";
	}

	scriptManagerFileContent += "NtshEngn::Scriptable NtshEngn::ScriptManager::createScriptable(const std::string& scriptName) {\n";
	if (scriptEntries.empty()) {
		scriptManagerFileContent += "\tNTSHENGN_UNUSED(scriptName);\n\n";
	}
	scriptManagerFileContent += "\tScriptable scriptable;\n\n";
	for (size_t i = 0; i < scriptEntries.size(); i++) {
		scriptManagerFileContent += "\t";
		if (i != 0) {
			scriptManagerFileContent += "else ";
		}
		scriptManagerFileContent += "if (scriptName == \"" + std::get<0>(scriptEntries[i]) + "\") {\n\t\t" + std::get<0>(scriptEntries[i]) + "* script = createScript<" + std::get<0>(scriptEntries[i]) + ">();\n\t\tscript->createEditableScriptVariableMap();\n\t\tscriptable.script = script;\n\t}\n";
	}
	if (!scriptEntries.empty()) {
		scriptManagerFileContent += "\n";
	}
	scriptManagerFileContent += "\treturn scriptable;\n}\n\n";

	for (size_t i = 0; i < scriptEntries.size(); i++) {
		scriptManagerFileContent += "void " + std::get<0>(scriptEntries[i]) + "::createEditableScriptVariableMap() {\n";
		const std::vector<std::pair<std::string, std::string>>& editableVariables = std::get<2>(scriptEntries[i]);
		for (const auto& editableVariable : editableVariables) {
			if (editableVariable.second != "Unknown") {
				scriptManagerFileContent += "\teditableScriptVariables[\"" + editableVariable.first + "\"] = { NtshEngn::EditableScriptVariableType::" + editableVariable.second + ", &" + editableVariable.first + " };\n";
			}
		}
		scriptManagerFileContent += "}\n\n";
	}

	scriptManagerFileContent += "extern \"C\" NTSHENGN_SCRIPT_MANAGER_API NtshEngn::ScriptManagerInterface* createScriptManager() {\n\treturn new NtshEngn::ScriptManager;\n}\n\nextern \"C\" NTSHENGN_SCRIPT_MANAGER_API void destroyScriptManager(NtshEngn::ScriptManagerInterface* sm) {\n\tdelete sm;\n}";

	std::fstream scriptManagerFile(m_globalInfo.projectDirectory + "/scripts/script_manager/ntshengn_script_manager.cpp", std::ios::out | std::ios::trunc);
	scriptManagerFile << scriptManagerFileContent;
}

void BuildBar::onBuildRunExportStarted() {
	buildAndRunButton->setEnabled(false);
	buildTypeComboBox->comboBox->setEnabled(false);
	exportButton->setEnabled(false);
}

void BuildBar::onBuildRunExportEnded() {
	buildAndRunButton->setEnabled(true);
	buildTypeComboBox->comboBox->setEnabled(true);
	exportButton->setEnabled(true);
}
