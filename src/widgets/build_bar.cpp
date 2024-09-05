#include "build_bar.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../common/scene_manager.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <sstream>
#include <filesystem>
#include <thread>
#include <regex>
#include <algorithm>
#if defined(NTSHENGN_OS_WINDOWS)
#include <windows.h>
#elif defined(NTSHENGN_OS_LINUX)
#include <stdio.h>
#endif
#include <cctype>

BuildBar::BuildBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 5, 0, 0);
	layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);
	buildAndRunButton = new QPushButton("Build and Run");
	layout()->addWidget(buildAndRunButton);
	std::vector<std::string> buildTypeList{ "Debug", "Release" };
	buildTypeComboBox = new ComboBoxWidget(m_globalInfo, "Build Type", buildTypeList);
	layout()->addWidget(buildTypeComboBox);
	exportButton = new QPushButton("Export");
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
	fileDialog.setWindowTitle("NutshellEngine - Export To...");
	fileDialog.setWindowIcon(QIcon("assets/icon.png"));
	fileDialog.setFileMode(QFileDialog::FileMode::Directory);
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
	const std::string buildType = buildTypeComboBox->comboBox->currentText().toStdString();
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Launching " + buildType + " build.");

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build");
	}

	// Set current path
	const std::string previousCurrentPath = std::filesystem::current_path().string();
	std::filesystem::current_path(m_globalInfo.projectDirectory + "/editor_build");

	if (!std::filesystem::exists(buildType)) {
		std::filesystem::create_directory(buildType);
	}

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

	const std::string cMakeCommand = m_globalInfo.editorParameters.build.cMakePath + " " + m_globalInfo.projectDirectory + " -DNTSHENGN_COMMON_PATH=" + m_globalInfo.projectDirectory + "/Common";
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Launching CMake with command: " + cMakeCommand);
	bool cMakeSuccess = true;
	if (CreateProcessA(NULL, const_cast<char*>(cMakeCommand.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		m_globalInfo.logger.addLog(LogLevel::Info, "[Build] CMake logs:");
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
			m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Successfully launched the project\'s CMakeLists.txt.");
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Error, "[Build] Error with the project\'s CMakeLists.txt.");

			cMakeSuccess = false;
		}
	}
	else {
		CloseHandle(pipeWrite);
		CloseHandle(pipeRead);
		m_globalInfo.logger.addLog(LogLevel::Error, "[Build] Cannot launch CMake (CMake not installed?).");

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
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Launching " + buildType + " build with command: " + cMakeBuildCommand);
	if (CreateProcessA(NULL, const_cast<char*>(cMakeBuildCommand.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Build logs:");
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
			m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Successfully built the project.");
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Error, "[Build] Error while building the project.");

			buildSuccess = false;
		}
	}
	else {
		CloseHandle(pipeWrite);
		CloseHandle(pipeRead);
		m_globalInfo.logger.addLog(LogLevel::Error, "[Build] Cannot launch CMake (CMake not installed?).");

		buildSuccess = false;
	}

	if (buildSuccess) {
		CloseHandle(pipeRead);
	}
#elif defined(NTSHENGN_OS_LINUX)
	// CMake
	const std::string cMakeCommand = m_globalInfo.editorParameters.build.cMakePath + " " + m_globalInfo.projectDirectory + " -DNTSHENGN_COMMON_PATH=" + m_globalInfo.projectDirectory + "/Common -DCMAKE_BUILD_TYPE=" + buildType + " 2>&1";
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Launching CMake with command: " + cMakeCommand);
	bool cMakeSuccess = true;
	FILE* fp = popen(cMakeCommand.c_str(), "r");
	if (fp == NULL) {
		m_globalInfo.logger.addLog(LogLevel::Error, "[Build] Cannot launch CMake (CMake not installed?).");

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return false;
	}
	
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] CMake logs:");
	char stdOutBuffer[4096];
	while (fgets(stdOutBuffer, 4096, fp) != NULL) {
		addLog(std::string(stdOutBuffer));
	}

	if (pclose(fp) == 0) {
		m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Successfully launched the project\'s CMakeLists.txt.");
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Error, "[Build] Error with the project\'s CMakeLists.txt.");

		cMakeSuccess = false;
	}

	if (!cMakeSuccess) {
		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return false;
	}

	// Build
	const std::string cMakeBuildCommand = m_globalInfo.editorParameters.build.cMakePath + " --build . --config " + buildType + " 2>&1";
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Launching " + buildType + " build with command: " + cMakeBuildCommand);
	fp = popen(cMakeBuildCommand.c_str(), "r");
	if (fp == NULL) {
		m_globalInfo.logger.addLog(LogLevel::Error, "[Build] Cannot launch CMake (CMake not installed?).");

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return false;
	}
	
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Build logs:");
	while (fgets(stdOutBuffer, 4096, fp) != NULL) {
		addLog(std::string(stdOutBuffer));
	}

	if (pclose(fp) == 0) {
		m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Successfully built the project.");
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Error, "[Build] Error while building the project.");

		buildSuccess = false;
	}

	std::string scriptsLibrary = "libNutshellEngine-Scripts.so";
	std::filesystem::copy(scriptsLibrary, buildType, std::filesystem::copy_options::overwrite_existing);
	std::string buildAssetsDirectory = buildType + "/assets";
	if (!std::filesystem::exists(buildAssetsDirectory)) {
		std::filesystem::create_directory(buildAssetsDirectory);
	}
	std::filesystem::copy("assets", buildAssetsDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
#endif

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);

	return buildSuccess;
}

void BuildBar::run() {
	const std::string buildType = buildTypeComboBox->comboBox->currentText().toStdString();
	m_globalInfo.logger.addLog(LogLevel::Info, "[Run] Running the application.");

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build");
		m_globalInfo.logger.addLog(LogLevel::Error, "[Run] There is no build to run.");

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
	m_globalInfo.logger.addLog(LogLevel::Info, "[Run] Launching application with command: " + runCommand);
	if (CreateProcessA(NULL, const_cast<char*>(runCommand.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		m_globalInfo.logger.addLog(LogLevel::Info, "[Run] Application Logs:");
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
			m_globalInfo.logger.addLog(LogLevel::Info, "[Run] Successfully closed the application.");
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Error, "[Run] Error when closing the application.");
		}
	}
	else {
		CloseHandle(pipeWrite);
		CloseHandle(pipeRead);
		m_globalInfo.logger.addLog(LogLevel::Error, "[Run] Cannot launch NutshellEngine\'s runtime executable.");

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return;
	}
	CloseHandle(pipeRead);
#elif defined(NTSHENGN_OS_LINUX)
	std::filesystem::current_path(buildType);
	const std::string runCommand = "./NutshellEngine 2>&1";
	m_globalInfo.logger.addLog(LogLevel::Info, "[Run] Launching application with command: " + runCommand);
	FILE* fp = popen(runCommand.c_str(), "r");
	if (fp == NULL) {
		m_globalInfo.logger.addLog(LogLevel::Error, "[Run] Cannot launch NutshellEngine\'s runtime executable.");

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);
	}
	
	m_globalInfo.logger.addLog(LogLevel::Info, "[Run] Application logs:");
	char stdOutBuffer[4096];
	while (fgets(stdOutBuffer, 4096, fp) != NULL) {
		std::string log = std::string(stdOutBuffer);

		std::stringstream syntaxSugarRegexResult;
		std::regex_replace(std::ostream_iterator<char>(syntaxSugarRegexResult), log.begin(), log.end(), syntaxSugarRegex, "");

		addLog(syntaxSugarRegexResult.str());
	}

	if (pclose(fp) == 0) {
		m_globalInfo.logger.addLog(LogLevel::Info, "[Run] Successfully closed the application.");
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Error, "[Run] Error when closing the application.");
	}
#endif

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);
}

void BuildBar::exportApplication(const std::string& exportDirectory) {
	const std::string buildType = buildTypeComboBox->comboBox->currentText().toStdString();
	m_globalInfo.logger.addLog(LogLevel::Info, "[Export] Exporting the application.");

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build");
		m_globalInfo.logger.addLog(LogLevel::Error, "[Export] There is no build to export.");

		return;
	}

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/editor_build/export_tmp")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/editor_build/export_tmp");
	}

	// Copy runtime
	std::filesystem::copy("assets/runtime/" + buildType, m_globalInfo.projectDirectory + "/editor_build/" + buildType, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	// Copy to another directory
	const std::string tmpExportDirectory = m_globalInfo.projectDirectory + "/editor_build/export_tmp/" + m_globalInfo.projectName;
	std::filesystem::create_directory(tmpExportDirectory);
	std::filesystem::copy(m_globalInfo.projectDirectory + "/editor_build/" + buildType, tmpExportDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	// Set current path
	const std::string previousCurrentPath = std::filesystem::current_path().string();
	std::filesystem::current_path(m_globalInfo.projectDirectory + "/editor_build");

#if defined(NTSHENGN_OS_WINDOWS)
	const std::string exportedFileName = m_globalInfo.projectName + "_" + buildType + ".zip";
	const std::string exportedFullPath = exportDirectory + "/" + exportedFileName;

	// Rename executable
	std::filesystem::rename(tmpExportDirectory + "/NutshellEngine.exe", tmpExportDirectory + "/" + m_globalInfo.projectName + ".exe");

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

	const std::string exportCommand = "powershell Compress-Archive -Path export_tmp/" + m_globalInfo.projectName + " -DestinationPath " + exportedFullPath + " -Force";
	m_globalInfo.logger.addLog(LogLevel::Info, "[Export] Export application with command: " + exportCommand);
	if (CreateProcessA(NULL, const_cast<char*>(exportCommand.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		m_globalInfo.logger.addLog(LogLevel::Info, "[Export] Export logs:");
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
			m_globalInfo.logger.addLog(LogLevel::Info, "[Export] Successfully exported the application at " + exportedFullPath + ".");
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Error, "[Export] Error when exporting the application.");
		}
	}
	else {
		CloseHandle(pipeWrite);
		CloseHandle(pipeRead);
		m_globalInfo.logger.addLog(LogLevel::Error, "[Export] Cannot export the application.");

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return;
	}
	CloseHandle(pipeRead);
#elif defined(NTSHENGN_OS_LINUX)
	const std::string exportedFileName = m_globalInfo.projectName + "_" + buildType + ".tar.gz";
	const std::string exportedFullPath = exportDirectory + "/" + exportedFileName;

	// Rename executable
	std::filesystem::rename(tmpExportDirectory + "/NutshellEngine", tmpExportDirectory + "/" + m_globalInfo.projectName);
	
	const std::string exportCommand = "tar -zcvf " + exportedFullPath + " export_tmp/" + m_globalInfo.projectName;
	m_globalInfo.logger.addLog(LogLevel::Info, "[Export] Launching export with command: " + exportCommand);
	FILE* fp = popen(exportCommand.c_str(), "r");
	if (fp == NULL) {
		m_globalInfo.logger.addLog(LogLevel::Error, "[Export] Cannot export the application.");

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);
	}

	m_globalInfo.logger.addLog(LogLevel::Info, "[Export] Export logs:");
	char stdOutBuffer[4096];
	while (fgets(stdOutBuffer, 4096, fp) != NULL) {
		addLog(std::string(stdOutBuffer));
	}

	if (pclose(fp) == 0) {
		m_globalInfo.logger.addLog(LogLevel::Info, "[Export] Successfully exported the application at " + exportedFullPath + ".");
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Error, "[Export] Error when exporting the application.");
	}
#endif

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);

	// Destroy temporary directory
	std::filesystem::remove_all(tmpExportDirectory);
	std::filesystem::remove(tmpExportDirectory);
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
