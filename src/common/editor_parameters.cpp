#pragma once
#include "editor_parameters.h"
#include "../../external/nlohmann/json.hpp"
#include <QKeySequence>
#include <fstream>

EditorParameters::EditorParameters() {
	std::fstream optionsFile("assets/options.json", std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			return;
		}
	}
	else {
		return;
	}

	optionsFile = std::fstream("assets/options.json", std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("renderer")) {
		if (j["renderer"].contains("cameraForwardKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraForwardKey"]));
			if (!sequence.isEmpty()) {
				renderer.cameraForwardKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraBackwardKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraBackwardKey"]));
			if (!sequence.isEmpty()) {
				renderer.cameraBackwardKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraLeftKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraLeftKey"]));
			if (!sequence.isEmpty()) {
				renderer.cameraLeftKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraRightKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraRightKey"]));
			if (!sequence.isEmpty()) {
				renderer.cameraRightKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraUpKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraUpKey"]));
			if (!sequence.isEmpty()) {
				renderer.cameraUpKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraDownKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraDownKey"]));
			if (!sequence.isEmpty()) {
				renderer.cameraDownKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("translateEntityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["translateEntityKey"]));
			if (!sequence.isEmpty()) {
				renderer.translateEntityKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("rotateEntityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["rotateEntityKey"]));
			if (!sequence.isEmpty()) {
				renderer.rotateEntityKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("scaleEntityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["scaleEntityKey"]));
			if (!sequence.isEmpty()) {
				renderer.scaleEntityKey = sequence[0].key();
			}
		}
	}

	if (j.contains("build")) {
		if (j["build"].contains("cMakePath")) {
			build.cMakePath = j["build"]["cMakePath"];
			std::replace(build.cMakePath.begin(), build.cMakePath.end(), '\\', '/');
		}
	}

	if (j.contains("code")) {
		if (j["code"].contains("codeEditorCommand")) {
			code.codeEditorCommand = j["code"]["codeEditorCommand"];
		}
	}
}
