#include "model_ntmd_primitive_widget.h"
#include "separator_line.h"
#include "../common/asset_helper.h"
#include <QVBoxLayout>

ModelNtmdPrimitiveWidget::ModelNtmdPrimitiveWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	removePrimitiveButton = new QPushButton("X");
	removePrimitiveButton->setFixedWidth(20);
	layout()->addWidget(removePrimitiveButton);
	layout()->setAlignment(removePrimitiveButton, Qt::AlignmentFlag::AlignRight);
	primitiveMeshWidget = new FileSelectorWidget(m_globalInfo, m_globalInfo.localization.getString("assets_model_primitive_mesh"), m_globalInfo.localization.getString("assets_model_primitive_no_mesh_selected"), "");
	layout()->addWidget(primitiveMeshWidget);
	primitiveMaterialWidget = new FileSelectorWidget(m_globalInfo, m_globalInfo.localization.getString("assets_model_primitive_material"), m_globalInfo.localization.getString("assets_model_primitive_no_material_selected"), "");
	layout()->addWidget(primitiveMaterialWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(removePrimitiveButton, &QPushButton::clicked, this, &ModelNtmdPrimitiveWidget::onRemovePrimitiveButtonClicked);
	connect(primitiveMeshWidget, &FileSelectorWidget::fileSelected, this, &ModelNtmdPrimitiveWidget::onValueChanged);
	connect(primitiveMaterialWidget, &FileSelectorWidget::fileSelected, this, &ModelNtmdPrimitiveWidget::onValueChanged);
}

nlohmann::json ModelNtmdPrimitiveWidget::toJson() const {
	nlohmann::json j;
	if (!primitiveMeshWidget->getPath().empty()) {
		std::string meshPath = AssetHelper::absoluteToRelative(primitiveMeshWidget->getPath(), m_globalInfo.projectDirectory);
		j["meshPath"] = meshPath;
	}
	if (!primitiveMaterialWidget->getPath().empty()) {
		std::string materialPath = AssetHelper::absoluteToRelative(primitiveMaterialWidget->getPath(), m_globalInfo.projectDirectory);
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