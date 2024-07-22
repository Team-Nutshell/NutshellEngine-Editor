#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class OptionsMenu : public QMenu {
	Q_OBJECT
public:
	OptionsMenu(GlobalInfo& globalInfo);

private:
	void openEditorParameters();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_openEditorParametersAction;
};