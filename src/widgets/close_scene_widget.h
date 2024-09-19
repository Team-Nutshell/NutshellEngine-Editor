#pragma once
#include "../common/global_info.h"
#include <QDialog>
#include <QPushButton>

class CloseSceneWidget : public QDialog {
	Q_OBJECT
public:
	CloseSceneWidget(GlobalInfo& globalInfo);

private slots:
	void onSaveSceneButtonClicked();
	void onDontSaveSceneButtonClicked();
	void onCancelButtonClicked();

signals:
	void confirmSignal();

private:
	GlobalInfo& m_globalInfo;

	std::string m_scenePath;

	QPushButton* m_saveSceneButton;
	QPushButton* m_dontSaveSceneButton;
	QPushButton* m_cancelButton;
};