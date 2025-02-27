#pragma once
#include "../common/global_info.h"
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>

class DeleteScriptWidget : public QDialog {
	Q_OBJECT
public:
	DeleteScriptWidget(GlobalInfo& globalInfo, const std::string& scriptName);

private slots:
	void onOkButtonClicked();
	void onCancelButtonClicked();

	void keyPressEvent(QKeyEvent* event);

private:
	GlobalInfo& m_globalInfo;

	std::string m_scriptName = "";

public:
	QPushButton* okButton;
	QPushButton* cancelButton;
};