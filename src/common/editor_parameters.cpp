#include "editor_parameters.h"
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

	fromJson(j);
}

void EditorParameters::fromJson(const nlohmann::json& j) {
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
		if (j["renderer"].contains("switchCameraProjectionKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["switchCameraProjectionKey"]));
			if (!sequence.isEmpty()) {
				renderer.switchCameraProjectionKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("resetCameraKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["resetCameraKey"]));
			if (!sequence.isEmpty()) {
				renderer.resetCameraKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("orthographicCameraToXMKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["orthographicCameraToXMKey"]));
			if (!sequence.isEmpty()) {
				renderer.orthographicCameraToXMKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("orthographicCameraToXPKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["orthographicCameraToXPKey"]));
			if (!sequence.isEmpty()) {
				renderer.orthographicCameraToXPKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("orthographicCameraToYMKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["orthographicCameraToYMKey"]));
			if (!sequence.isEmpty()) {
				renderer.orthographicCameraToYMKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("orthographicCameraToYPKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["orthographicCameraToYPKey"]));
			if (!sequence.isEmpty()) {
				renderer.orthographicCameraToYPKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("orthographicCameraToZMKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["orthographicCameraToZMKey"]));
			if (!sequence.isEmpty()) {
				renderer.orthographicCameraToZMKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("orthographicCameraToZPKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["orthographicCameraToZPKey"]));
			if (!sequence.isEmpty()) {
				renderer.orthographicCameraToZPKey = sequence[0].key();
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
		if (j["renderer"].contains("toggleCurrentEntityVisibilityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["toggleCurrentEntityVisibilityKey"]));
			if (!sequence.isEmpty()) {
				renderer.toggleCurrentEntityVisibilityKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("toggleGridVisibilityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["toggleGridVisibilityKey"]));
			if (!sequence.isEmpty()) {
				renderer.toggleGridVisibilityKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("toggleBackfaceCullingKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["toggleBackfaceCullingKey"]));
			if (!sequence.isEmpty()) {
				renderer.toggleBackfaceCullingKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("toggleCamerasVisibilityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["toggleCamerasVisibilityKey"]));
			if (!sequence.isEmpty()) {
				renderer.toggleCamerasVisibilityKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("toggleLightingKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["toggleLightingKey"]));
			if (!sequence.isEmpty()) {
				renderer.toggleLightingKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("toggleCollidersVisibilityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["toggleCollidersVisibilityKey"]));
			if (!sequence.isEmpty()) {
				renderer.toggleCollidersVisibilityKey = sequence[0].key();
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

nlohmann::json EditorParameters::toJson() const {
	nlohmann::json j;
	j["renderer"]["cameraForwardKey"] = QKeySequence(renderer.cameraForwardKey).toString().toStdString();
	j["renderer"]["cameraBackwardKey"] = QKeySequence(renderer.cameraBackwardKey).toString().toStdString();
	j["renderer"]["cameraLeftKey"] = QKeySequence(renderer.cameraLeftKey).toString().toStdString();
	j["renderer"]["cameraRightKey"] = QKeySequence(renderer.cameraRightKey).toString().toStdString();
	j["renderer"]["cameraUpKey"] = QKeySequence(renderer.cameraUpKey).toString().toStdString();
	j["renderer"]["cameraDownKey"] = QKeySequence(renderer.cameraDownKey).toString().toStdString();
	j["renderer"]["switchCameraProjectionKey"] = QKeySequence(renderer.switchCameraProjectionKey).toString().toStdString();
	j["renderer"]["resetCameraKey"] = QKeySequence(renderer.resetCameraKey).toString().toStdString();
	j["renderer"]["orthographicCameraToXMKey"] = QKeySequence(renderer.orthographicCameraToXMKey).toString().toStdString();
	j["renderer"]["orthographicCameraToXPKey"] = QKeySequence(renderer.orthographicCameraToXPKey).toString().toStdString();
	j["renderer"]["orthographicCameraToYMKey"] = QKeySequence(renderer.orthographicCameraToYMKey).toString().toStdString();
	j["renderer"]["orthographicCameraToYPKey"] = QKeySequence(renderer.orthographicCameraToYPKey).toString().toStdString();
	j["renderer"]["orthographicCameraToZMKey"] = QKeySequence(renderer.orthographicCameraToZMKey).toString().toStdString();
	j["renderer"]["orthographicCameraToZPKey"] = QKeySequence(renderer.orthographicCameraToZPKey).toString().toStdString();
	j["renderer"]["translateEntityKey"] = QKeySequence(renderer.translateEntityKey).toString().toStdString();
	j["renderer"]["rotateEntityKey"] = QKeySequence(renderer.rotateEntityKey).toString().toStdString();
	j["renderer"]["scaleEntityKey"] = QKeySequence(renderer.scaleEntityKey).toString().toStdString();
	j["renderer"]["toggleCurrentEntityVisibilityKey"] = QKeySequence(renderer.toggleCurrentEntityVisibilityKey).toString().toStdString();
	j["renderer"]["toggleGridVisibilityKey"] = QKeySequence(renderer.toggleGridVisibilityKey).toString().toStdString();
	j["renderer"]["toggleBackfaceCullingKey"] = QKeySequence(renderer.toggleBackfaceCullingKey).toString().toStdString();
	j["renderer"]["toggleCamerasVisibilityKey"] = QKeySequence(renderer.toggleCamerasVisibilityKey).toString().toStdString();
	j["renderer"]["toggleLightingKey"] = QKeySequence(renderer.toggleLightingKey).toString().toStdString();
	j["renderer"]["toggleCollidersVisibilityKey"] = QKeySequence(renderer.toggleCollidersVisibilityKey).toString().toStdString();

	j["build"]["cMakePath"] = build.cMakePath;

	j["code"]["codeEditorCommand"] = code.codeEditorCommand;

	return j;
}
