#pragma once
#include "../common/common.h"
#include "asset_list.h"
#include <QWidget>
#include <QLabel>

class ResourcePanel : public QWidget {
	Q_OBJECT
public:
	ResourcePanel(GlobalInfo& globalInfo);

private slots:
	void onDirectoryChanged(const std::string& directory);

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* assetsLabel;
	AssetList* assetList;
};