#pragma once
#include "../common/global_info.h"
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>

class DeleteAssetWidget : public QDialog {
	Q_OBJECT
public:
	DeleteAssetWidget(GlobalInfo& globalInfo, const std::string& path);

private slots:
	void onOkButtonClicked();
	void onCancelButtonClicked();

	void keyPressEvent(QKeyEvent* event);

private:
	GlobalInfo& m_globalInfo;

	std::string m_path = "";

public:
	QPushButton* okButton;
	QPushButton* cancelButton;
};