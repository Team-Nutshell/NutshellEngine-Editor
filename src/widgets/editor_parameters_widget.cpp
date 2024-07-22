#include "editor_parameters_widget.h"
#include "main_window.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <fstream>

EditorParametersWidget::EditorParametersWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(1000, 360);
	setWindowTitle("NutshellEngine - Editor Parameters");
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	QMargins contentMargins = contentsMargins();
	contentMargins.setTop(contentMargins.top() + 10);
	setContentsMargins(contentMargins);

	rendererParametersTab = new QTabWidget();
	QGridLayout* layout = new QGridLayout();
	rendererParametersTab->setLayout(layout);

	QKeySequence cameraForwardKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraForwardKey);
	cameraForwardKeySelect = new KeySelectWidget(m_globalInfo, "Camera Forward", cameraForwardKeySequence.toString().toStdString());
	cameraForwardKeySelect->layout()->setAlignment(cameraForwardKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(cameraForwardKeySelect, 0, 0);

	QKeySequence cameraBackwardKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraBackwardKey);
	cameraBackwardKeySelect = new KeySelectWidget(m_globalInfo, "Camera Backward", cameraBackwardKeySequence.toString().toStdString());
	cameraBackwardKeySelect->layout()->setAlignment(cameraBackwardKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(cameraBackwardKeySelect, 1, 0);

	QKeySequence cameraLeftKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraLeftKey);
	cameraLeftKeySelect = new KeySelectWidget(m_globalInfo, "Camera Left", cameraLeftKeySequence.toString().toStdString());
	cameraLeftKeySelect->layout()->setAlignment(cameraLeftKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(cameraLeftKeySelect, 2, 0);

	QKeySequence cameraRightKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraRightKey);
	cameraRightKeySelect = new KeySelectWidget(m_globalInfo, "Camera Right", cameraRightKeySequence.toString().toStdString());
	cameraRightKeySelect->layout()->setAlignment(cameraRightKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(cameraRightKeySelect, 3, 0);

	QKeySequence cameraUpKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraUpKey);
	cameraUpKeySelect = new KeySelectWidget(m_globalInfo, "Camera Up", cameraUpKeySequence.toString().toStdString());
	cameraUpKeySelect->layout()->setAlignment(cameraUpKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(cameraUpKeySelect, 4, 0);

	QKeySequence cameraDownKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraDownKey);
	cameraDownKeySelect = new KeySelectWidget(m_globalInfo, "Camera Down", cameraDownKeySequence.toString().toStdString());
	cameraDownKeySelect->layout()->setAlignment(cameraDownKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(cameraDownKeySelect, 5, 0);

	QKeySequence switchCameraProjectionKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.switchCameraProjectionKey);
	switchCameraProjectionKeySelect = new KeySelectWidget(m_globalInfo, "Switch Camera Projection", switchCameraProjectionKeySequence.toString().toStdString());
	switchCameraProjectionKeySelect->layout()->setAlignment(switchCameraProjectionKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(switchCameraProjectionKeySelect, 6, 0);

	QKeySequence resetCameraKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.resetCameraKey);
	resetCameraKeySelect = new KeySelectWidget(m_globalInfo, "Switch Camera Projection", resetCameraKeySequence.toString().toStdString());
	resetCameraKeySelect->layout()->setAlignment(resetCameraKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(resetCameraKeySelect, 7, 0);

	QKeySequence orthographicCameraToXMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey);
	orthographicCameraToXMKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To X-", orthographicCameraToXMKeySequence.toString().toStdString());
	orthographicCameraToXMKeySelect->layout()->setAlignment(orthographicCameraToXMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(orthographicCameraToXMKeySelect, 0, 1);

	QKeySequence orthographicCameraToXPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey);
	orthographicCameraToXPKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To X+", orthographicCameraToXPKeySequence.toString().toStdString());
	orthographicCameraToXPKeySelect->layout()->setAlignment(orthographicCameraToXPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(orthographicCameraToXPKeySelect, 1, 1);

	QKeySequence orthographicCameraToYMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey);
	orthographicCameraToYMKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To Y-", orthographicCameraToYMKeySequence.toString().toStdString());
	orthographicCameraToYMKeySelect->layout()->setAlignment(orthographicCameraToYMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(orthographicCameraToYMKeySelect, 2, 1);

	QKeySequence orthographicCameraToYPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey);
	orthographicCameraToYPKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To Y+", orthographicCameraToYPKeySequence.toString().toStdString());
	orthographicCameraToYPKeySelect->layout()->setAlignment(orthographicCameraToYPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(orthographicCameraToYPKeySelect, 3, 1);

	QKeySequence orthographicCameraToZMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey);
	orthographicCameraToZMKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To Z-", orthographicCameraToZMKeySequence.toString().toStdString());
	orthographicCameraToZMKeySelect->layout()->setAlignment(orthographicCameraToZMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(orthographicCameraToZMKeySelect, 4, 1);

	QKeySequence orthographicCameraToZPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);
	orthographicCameraToZPKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To Z+", orthographicCameraToZPKeySequence.toString().toStdString());
	orthographicCameraToZPKeySelect->layout()->setAlignment(orthographicCameraToZPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(orthographicCameraToZPKeySelect, 5, 1);

	QKeySequence translateEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.translateEntityKey);
	translateEntityKeySelect = new KeySelectWidget(m_globalInfo, "Translate Entity", translateEntityKeySequence.toString().toStdString());
	translateEntityKeySelect->layout()->setAlignment(translateEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(translateEntityKeySelect, 6, 1);

	QKeySequence rotateEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.rotateEntityKey);
	rotateEntityKeySelect = new KeySelectWidget(m_globalInfo, "Rotate Entity", rotateEntityKeySequence.toString().toStdString());
	rotateEntityKeySelect->layout()->setAlignment(rotateEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(rotateEntityKeySelect, 7, 1);

	QKeySequence scaleEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.scaleEntityKey);
	scaleEntityKeySelect = new KeySelectWidget(m_globalInfo, "Scale Entity", scaleEntityKeySequence.toString().toStdString());
	scaleEntityKeySelect->layout()->setAlignment(scaleEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(scaleEntityKeySelect, 0, 2);

	QKeySequence toggleCurrentEntityVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey);
	toggleCurrentEntityVisibilityKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Current Entity Visibility", toggleCurrentEntityVisibilityKeySequence.toString().toStdString());
	toggleCurrentEntityVisibilityKeySelect->layout()->setAlignment(toggleCurrentEntityVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(toggleCurrentEntityVisibilityKeySelect, 1, 2);

	QKeySequence toggleGridVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey);
	toggleGridVisibilityKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Grid Visibility", toggleGridVisibilityKeySequence.toString().toStdString());
	toggleGridVisibilityKeySelect->layout()->setAlignment(toggleGridVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(toggleGridVisibilityKeySelect, 2, 2);

	QKeySequence toggleBackfaceCullingKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey);
	toggleBackfaceCullingKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Backface Culling", toggleBackfaceCullingKeySequence.toString().toStdString());
	toggleBackfaceCullingKeySelect->layout()->setAlignment(toggleBackfaceCullingKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(toggleBackfaceCullingKeySelect, 3, 2);

	QKeySequence toggleCamerasVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey);
	toggleCamerasVisibilityKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Cameras Visibility", toggleCamerasVisibilityKeySequence.toString().toStdString());
	toggleCamerasVisibilityKeySelect->layout()->setAlignment(toggleCamerasVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(toggleCamerasVisibilityKeySelect, 4, 2);

	QKeySequence toggleLightingKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleLightingKey);
	toggleLightingKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Lighting", toggleLightingKeySequence.toString().toStdString());
	toggleLightingKeySelect->layout()->setAlignment(toggleLightingKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(toggleLightingKeySelect, 5, 2);

	QKeySequence toggleCollidersVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
	toggleCollidersVisibilityKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Colliders Visibility", toggleCollidersVisibilityKeySequence.toString().toStdString());
	toggleCollidersVisibilityKeySelect->layout()->setAlignment(toggleCollidersVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	layout->addWidget(toggleCollidersVisibilityKeySelect, 6, 2);

	addTab(rendererParametersTab, "Renderer");

	buildParametersTab = new QTabWidget();
	buildParametersTab->setLayout(new QVBoxLayout());
	cMakePathWidget = new StringWidget(m_globalInfo, "CMake executable");
	cMakePathWidget->valueLineEdit->setText(QString::fromStdString(m_globalInfo.editorParameters.build.cMakePath));
	cMakePathWidget->valueLineEdit->setFixedWidth(400);
	cMakePathWidget->layout()->setAlignment(cMakePathWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	cMakePathWidget->layout()->setAlignment(cMakePathWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	buildParametersTab->layout()->addWidget(cMakePathWidget);

	addTab(buildParametersTab, "Build");

	codeParametersTab = new QTabWidget();
	codeParametersTab->setLayout(new QVBoxLayout());
	codeEditorCommandWidget = new StringWidget(m_globalInfo, "Code editor command");
	codeEditorCommandWidget->valueLineEdit->setText(QString::fromStdString(m_globalInfo.editorParameters.code.codeEditorCommand));
	codeEditorCommandWidget->valueLineEdit->setFixedWidth(400);
	codeEditorCommandWidget->layout()->setAlignment(codeEditorCommandWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	codeEditorCommandWidget->layout()->setAlignment(codeEditorCommandWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	codeParametersTab->layout()->addWidget(codeEditorCommandWidget);

	addTab(codeParametersTab, "Code");

	connect(cameraForwardKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraBackwardKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraLeftKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraRightKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraUpKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraDownKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(switchCameraProjectionKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(resetCameraKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToXMKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToXPKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToYMKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToYPKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToZMKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToZPKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(translateEntityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(rotateEntityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(scaleEntityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleCurrentEntityVisibilityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleGridVisibilityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleBackfaceCullingKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleCamerasVisibilityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleLightingKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleCollidersVisibilityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cMakePathWidget, &StringWidget::valueChanged, this, &EditorParametersWidget::onCMakePathChanged);
	connect(codeEditorCommandWidget, &StringWidget::valueChanged, this, &EditorParametersWidget::onCodeEditorCommandChanged);
}

void EditorParametersWidget::onKeyChanged(const std::string& key) {
	QObject* senderWidget = sender();
	if (senderWidget == cameraForwardKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.cameraForwardKey = sequence[0].key();
		}
	}
	else if (senderWidget == cameraBackwardKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.cameraBackwardKey = sequence[0].key();
		}
	}
	else if (senderWidget == cameraLeftKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.cameraLeftKey = sequence[0].key();
		}
	}
	else if (senderWidget == cameraRightKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.cameraRightKey = sequence[0].key();
		}
	}
	else if (senderWidget == cameraUpKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.cameraUpKey = sequence[0].key();
		}
	}
	else if (senderWidget == cameraDownKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.cameraDownKey = sequence[0].key();
		}
	}
	else if (senderWidget == switchCameraProjectionKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.switchCameraProjectionKey = sequence[0].key();
		}
	}
	else if (senderWidget == resetCameraKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.resetCameraKey = sequence[0].key();
		}
	}
	else if (senderWidget == orthographicCameraToXMKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->orthographicCameraToXMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey);
		}
	}
	else if (senderWidget == orthographicCameraToXPKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->orthographicCameraToXPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey);
		}
	}
	else if (senderWidget == orthographicCameraToYMKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->orthographicCameraToYMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey);
		}
	}
	else if (senderWidget == orthographicCameraToYPKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->orthographicCameraToYPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey);
		}
	}
	else if (senderWidget == orthographicCameraToZMKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->orthographicCameraToZMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey);
		}
	}
	else if (senderWidget == orthographicCameraToZPKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->orthographicCameraToZPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);
		}
	}
	else if (senderWidget == translateEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.translateEntityKey = sequence[0].key();
		}
	}
	else if (senderWidget == rotateEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.rotateEntityKey = sequence[0].key();
		}
	}
	else if (senderWidget == scaleEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.scaleEntityKey = sequence[0].key();
		}
	}
	else if (senderWidget == toggleCurrentEntityVisibilityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->toggleCurrentEntityVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey);
		}
	}
	else if (senderWidget == toggleGridVisibilityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->toggleGridVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey);
		}
	}
	else if (senderWidget == toggleBackfaceCullingKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->toggleBackfaceCullingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey);
		}
	}
	else if (senderWidget == toggleCamerasVisibilityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->toggleCamerasVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey);
		}
	}
	else if (senderWidget == toggleLightingKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.toggleLightingKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->toggleLightingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleLightingKey);
		}
	}
	else if (senderWidget == toggleCollidersVisibilityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty()) {
			m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey = sequence[0].key();
			reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow)->viewMenu->toggleCollidersVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
		}
	}

	save();
}

void EditorParametersWidget::onCMakePathChanged(const std::string& cMakePath) {
	m_globalInfo.editorParameters.build.cMakePath = cMakePath;

	save();
}

void EditorParametersWidget::onCodeEditorCommandChanged(const std::string& codeEditorCommandChanged) {
	m_globalInfo.editorParameters.code.codeEditorCommand = codeEditorCommandChanged;

	save();
}

void EditorParametersWidget::save() {
	std::fstream optionsFile("assets/options.json", std::ios::out | std::ios::trunc);
	nlohmann::json j = m_globalInfo.editorParameters.toJson();
	optionsFile << j.dump(1, '\t');
}