#include "build_bar.h"
#include <QHBoxLayout>
#include <fstream>

BuildBar::BuildBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);
	buildButton = new QPushButton("Build");
	layout()->addWidget(buildButton);

	std::fstream optionsFile("assets/options.json", std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"assets/options.json\" is not a valid JSON file.");
			buildButton->setEnabled(false);
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"assets/options.json\" cannot be opened.");
		buildButton->setEnabled(false);
		return;
	}

	optionsFile = std::fstream("assets/options.json", std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("build")) {
		if (j["build"].contains("compilerPath")) {
			m_globalInfo.compilerPath = j["build"]["compilerPath"];
		}
	}

	if (m_globalInfo.compilerPath.empty()) {
		buildButton->setEnabled(false);
		m_globalInfo.logger.addLog(LogLevel::Warning, "No compiler path has been specified.");
	}
	else {
		buildButton->setEnabled(true);
	}
}