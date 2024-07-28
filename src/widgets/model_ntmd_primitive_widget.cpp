#include "model_ntmd_primitive_widget.h"
#include "separator_line.h"
#include <QVBoxLayout>

ModelNtmdPrimitiveWidget::ModelNtmdPrimitiveWidget(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	removePrimitiveButton = new QPushButton("X");
	removePrimitiveButton->setFixedWidth(20);
	layout()->addWidget(removePrimitiveButton);
	layout()->setAlignment(removePrimitiveButton, Qt::AlignmentFlag::AlignRight);
	primitiveMeshWidget = new FileSelectorWidget(m_globalInfo, "Mesh", "No mesh selected", "");
	layout()->addWidget(primitiveMeshWidget);
	primitiveMaterialWidget = new FileSelectorWidget(m_globalInfo, "Material", "No material selected", "");
	layout()->addWidget(primitiveMaterialWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(removePrimitiveButton, &QPushButton::clicked, this, &ModelNtmdPrimitiveWidget::onRemovePrimitiveButtonClicked);
	connect(primitiveMeshWidget, &FileSelectorWidget::fileSelected, this, &ModelNtmdPrimitiveWidget::onValueChanged);
	connect(primitiveMaterialWidget, &FileSelectorWidget::fileSelected, this, &ModelNtmdPrimitiveWidget::onValueChanged);
}

nlohmann::json ModelNtmdPrimitiveWidget::toJson() const {
	nlohmann::json j;
	if (primitiveMeshWidget->filePathButton->path != "") {
		std::string meshPath = primitiveMeshWidget->filePathButton->path;
		std::replace(meshPath.begin(), meshPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(meshPath).is_absolute()) {
				if (meshPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					meshPath = meshPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["meshPath"] = meshPath;
	}
	if (primitiveMaterialWidget->filePathButton->path != "") {
		std::string materialPath = primitiveMaterialWidget->filePathButton->path;
		std::replace(materialPath.begin(), materialPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(materialPath).is_absolute()) {
				if (materialPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					materialPath = materialPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["materialPath"] = materialPath;
	}

	return j;
}

void ModelNtmdPrimitiveWidget::onValueChanged() {
	emit valueChanged();
}

void ModelNtmdPrimitiveWidget::onRemovePrimitiveButtonClicked() {
	emit removePrimitive();
}