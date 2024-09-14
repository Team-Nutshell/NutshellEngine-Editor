#include "delete_asset_widget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <filesystem>

DeleteAssetWidget::DeleteAssetWidget(GlobalInfo& globalInfo, const std::string& path) : m_globalInfo(globalInfo), m_path(path) {
	setWindowTitle("NutshellEngine - Delete - " + QString::fromStdString(path));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);
	setModal(true);

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignHCenter);
	layout()->addWidget(new QLabel("Do you want to delete"));
	layout()->addWidget(new QLabel(QString::fromStdString(m_path) + "?"));
	layout()->addWidget(new QLabel("<b>This action cannot be undone.</b>"));
	QWidget* buttonLayoutWidget = new QWidget();
	buttonLayoutWidget->setLayout(new QHBoxLayout());
	okButton = new QPushButton("OK");
	buttonLayoutWidget->layout()->addWidget(okButton);
	cancelButton = new QPushButton("Cancel");
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
		emit m_globalInfo.signalEmitter.selectAssetSignal("");
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