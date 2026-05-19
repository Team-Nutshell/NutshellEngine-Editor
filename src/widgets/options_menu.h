#pragma once
#include "../common/global_info.h"
#include "editor_parameters_widget.h"
#include <QMenu>
#include <QAction>

class OptionsMenu : public QMenu {
	Q_OBJECT
public:
	OptionsMenu(GlobalInfo& globalInfo);

private:
	void openEditorParameters();
	void openProjectOptions();

private slots:
	void onEditorParametersWidgetClose();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_openEditorParametersAction;
	QAction* m_openProjectOptionsAction;

	EditorParametersWidget* m_editorParametersWidget = nullptr;
};