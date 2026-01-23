#include "delete_asset_widget.h"
#include "../common/asset_helper.h"
#include "../undo_commands/select_asset_entities_command.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <filesystem>

DeleteAssetWidget::DeleteAssetWidget(GlobalInfo& globalInfo, const std::string& path) : m_globalInfo(globalInfo), m_path(path) {
	setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("assets_delete")) + " - " + QString::fromStdString(path));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);
	setModal(true);

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignHCenter);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_confirm_delete", { AssetHelper::absoluteToRelative(m_path, m_globalInfo.projectDirectory) }))));
	layout()->addWidget(new QLabel("<b>" + QString::fromStdString(m_globalInfo.localization.getString("assets_delete_cannot_undo")) + "</b>"));
	QWidget* buttonLayoutWidget = new QWidget();
	buttonLayoutWidget->setLayout(new QHBoxLayout());
	okButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("ok", { m_path })));
	buttonLayoutWidget->layout()->addWidget(okButton);
	cancelButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("cancel", { m_path })));
	buttonLayoutWidget->layout()->addWidget(cancelButton);
	layout()->addWidget(buttonLayoutWidget);

	connect(okButton, &QPushButton::clicked, this, &DeleteAssetWidget::onOkButtonClicked);
	connect(cancelButton, &QPushButton::clicked, this, &DeleteAssetWidget::onCancelButtonClicked);
}

void DeleteAssetWidget::onOkButtonClicked() {
	if (std::filesystem::exists(m_path)) {
		if (std::filesystem::is_directory(m_path)) {
			std::filesystem::remove_all(m_path);
		}
		else {
			std::filesystem::remove(m_path);
		}
		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, "", NO_ENTITY, {}));
	}
	close();
}

void DeleteAssetWidget::onCancelButtonClicked() {
	close();
}

void DeleteAssetWidget::keyPressEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == Qt::Key_Escape) {
		close();
	}
}