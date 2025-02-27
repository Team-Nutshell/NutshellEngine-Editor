#pragma once
#include "../common/global_info.h"
#include "asset_list.h"
#include <QWidget>
#include <QLabel>

class AssetPanel : public QWidget {
	Q_OBJECT
public:
	AssetPanel(GlobalInfo& globalInfo);

private slots:
	void onDirectoryChanged(const std::string& directory);

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* assetsLabel;
	AssetList* assetList;
};