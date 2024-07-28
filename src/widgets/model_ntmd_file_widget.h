#pragma once
#include "../common/global_info.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <string>

class ModelNtmdFileWidget : public QWidget {
	Q_OBJECT
public:
	ModelNtmdFileWidget(GlobalInfo& globalInfo, const std::string& modelFilePath);

private slots:
	void onAddPrimitiveButtonClicked();
	void onRemovePrimitiveButtonClicked();
	void onValueChanged();
	void save();

private:
	GlobalInfo& m_globalInfo;

	std::string m_modelFilePath;

	QMenuBar* m_menuBar;
	QMenu* m_fileMenu;

	QAction* m_fileSaveAction;

public:
	QWidget* primitivesWidget;
	QPushButton* addPrimitiveButton;
};