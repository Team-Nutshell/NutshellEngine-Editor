#pragma once
#include "../common/global_info.h"
#include <QLineEdit>
#include <string>

class AssetInfoNameWidget : public QLineEdit {
	Q_OBJECT
public:
	AssetInfoNameWidget(GlobalInfo& globalInfo);

private slots:
	void onAssetSelected(const std::string& path);
	void onEditingFinished();
	void onFileRenamed(const std::string& oldFilename, const std::string& newFilename);

private:
	GlobalInfo& m_globalInfo;

	std::string m_previousName;
	std::string m_directory;
};