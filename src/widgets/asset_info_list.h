#pragma once
#include "../common/global_info.h"
#include "material_ntml_file_widget.h"
#include "model_ntmd_file_widget.h"
#include "options_ntop_file_widget.h"
#include "sampler_ntsp_file_widget.h"
#include "image_file_widget.h"
#include "text_file_widget.h"
#include "project_ntpj_file_widget.h"
#include <QTreeView>
#include <QWidget>
#include <string>

class AssetInfoList : public QWidget {
	Q_OBJECT
public:
	AssetInfoList(GlobalInfo& globalInfo);

private slots:
	void onAssetSelected(const std::string&);

private:
	GlobalInfo& m_globalInfo;

public:
	MaterialNtmlFileWidget* materialNtmlFileWidget;
	ModelNtmdFileWidget* modelNtmdFileWidget;
	OptionsNtopFileWidget* optionsNtopFileWidget;
	SamplerNtspFileWidget* samplerNtspFileWidget;
	ImageFileWidget* imageFileWidget;
	QTreeView* jsonFileWidget;
	TextFileWidget* textFileWidget;
	ProjectNtpjFileWidget* projectNtpjFileWidget;

	std::string currentAssetPath = "";
};