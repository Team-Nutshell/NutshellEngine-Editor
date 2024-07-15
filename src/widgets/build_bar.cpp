#include "build_bar.h"
#include <QHBoxLayout>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <regex>
#if defined(NTSHENGN_OS_WINDOWS)
#include <windows.h>
#elif defined(NTSHENGN_OS_LINUX)
#endif

BuildBar::BuildBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);
	buildButton = new QPushButton("Build");
	layout()->addWidget(buildButton);
	std::vector<std::string> buildTypeList{ "Debug", "Release" };
	buildTypeComboBox = new ComboBoxWidget(m_globalInfo, "Build Type", buildTypeList);
	layout()->addWidget(buildTypeComboBox);

	connect(buildButton, &QPushButton::clicked, this, &BuildBar::launchBuild);
}

void BuildBar::launchBuild() {
	if (m_isBuilding) {
		return;
	}

	std::thread buildThread([this]() {
		m_isBuilding = true;
		if (build()) {
			run();
		}
		m_isBuilding = false;
	});
	buildThread.detach();
}

bool BuildBar::build() {
	const std::string buildType = buildTypeComboBox->comboBox->currentText().toStdString();
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Launching " + buildType + " build.");

	std::string cMakePath = "cmake";

	std::fstream optionsFile("assets/options.json", std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"assets/options.json\" is not a valid JSON file.");
			buildButton->setEnabled(false);

			return false;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"assets/options.json\" cannot be opened.");
		buildButton->setEnabled(false);

		return false;
	}

	optionsFile = std::fstream("assets/options.json", std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("build")) {
		if (j["build"].contains("cMakePath")) {
			cMakePath = j["build"]["cMakePath"];
			std::replace(cMakePath.begin(), cMakePath.end(), '\\', '/');
		}
	}

	if (!std::filesystem::exists("editor_build")) {
		std::filesystem::create_directory("editor_build");
	}

	// Set current path
	const std::string previousCurrentPath = std::filesystem::current_path().string();
	std::filesystem::current_path("editor_build");

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

	const std::string cMakeCommand = cMakePath + " " + m_globalInfo.projectDirectory + " -DNTSHENGN_COMMON_PATH=" + m_globalInfo.projectDirectory + "/Common";
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Launching CMake with command: " + cMakeCommand);
	bool cMakeSuccess = true;
	if (CreateProcessA(NULL, const_cast<char*>(cMakeCommand.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		m_globalInfo.logger.addLog(LogLevel::Info, "[Build] CMake logs:");
		CHAR stdoutBuffer[4096];
		DWORD bytesRead;
		while (ReadFile(pipeRead, stdoutBuffer, 4096, &bytesRead, NULL)) {
			m_globalInfo.logger.addLog(LogLevel::Info, std::string(stdoutBuffer, bytesRead));
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

		return cMakeSuccess;
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

	const std::string cMakeBuildCommand = cMakePath + " --build . --config " + buildType;
	m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Launching " + buildType + " build with command: " + cMakeBuildCommand);
	if (CreateProcessA(NULL, const_cast<char*>(cMakeBuildCommand.c_str()), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation)) {
		CloseHandle(pipeWrite);

		m_globalInfo.logger.addLog(LogLevel::Info, "[Build] Build logs:");
		CHAR stdoutBuffer[4096];
		DWORD bytesRead;
		while (ReadFile(pipeRead, stdoutBuffer, 4096, &bytesRead, NULL)) {
			m_globalInfo.logger.addLog(LogLevel::Info, std::string(stdoutBuffer, bytesRead));
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
#endif

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);

	return buildSuccess;
}

void BuildBar::run() {
	const std::string buildType = buildTypeComboBox->comboBox->currentText().toStdString();
	m_globalInfo.logger.addLog(LogLevel::Info, "[Run] Running the application.");

	if (!std::filesystem::exists("editor_build")) {
		std::filesystem::create_directory("editor_build");
		m_globalInfo.logger.addLog(LogLevel::Error, "[Run] There is no build to run.");

		return;
	}

	// Copy runtime
	std::filesystem::copy("assets/runtime/" + buildType, "editor_build/" + buildType, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	// Set current path
	const std::string previousCurrentPath = std::filesystem::current_path().string();
	std::filesystem::current_path("editor_build");

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
		CHAR stdoutBuffer[4096];
		DWORD bytesRead;
		while (ReadFile(pipeRead, stdoutBuffer, 4096, &bytesRead, NULL)) {
			std::string log = std::string(stdoutBuffer, bytesRead);

			std::stringstream syntaxSugarRegexResult;
			std::regex_replace(std::ostream_iterator<char>(syntaxSugarRegexResult), log.begin(), log.end(), syntaxSugarRegex, "");

			m_globalInfo.logger.addLog(LogLevel::Info, syntaxSugarRegexResult.str());
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
		m_globalInfo.logger.addLog(LogLevel::Error, "[Run] Cannot find NutshellEngine\'s runtime executable.");

		// Reset current path
		std::filesystem::current_path(previousCurrentPath);

		return;
	}
	CloseHandle(pipeRead);
#elif defined(NTSHENGN_OS_LINUX)
#endif

	// Reset current path
	std::filesystem::current_path(previousCurrentPath);
}
