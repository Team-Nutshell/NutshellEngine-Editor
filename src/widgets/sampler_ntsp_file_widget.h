#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include "integer_widget.h"
#include "scalar_widget.h"
#include <string>

struct SamplerNtsp {
	std::string magFilter = "Nearest";
	std::string minFilter = "Nearest";
	std::string mipmapFilter = "Nearest";
	std::string addressModeU = "ClampToEdge";
	std::string addressModeV = "ClampToEdge";
	std::string addressModeW = "ClampToEdge";
	std::string borderColor = "IntOpaqueBlack";
	float minLod = 0.0f;
	float maxLod = 1000.0f;
	float maxAnisotropy = 0.0f;

	bool operator==(const SamplerNtsp& rhs) {
		return (magFilter == rhs.magFilter) &&
			(minFilter == rhs.minFilter) &&
			(mipmapFilter == rhs.mipmapFilter) &&
			(addressModeU == rhs.addressModeU) &&
			(addressModeV == rhs.addressModeV) &&
			(addressModeW == rhs.addressModeW) &&
			(borderColor == rhs.borderColor) &&
			(minLod == rhs.minLod) &&
			(maxLod == rhs.maxLod) &&
			(maxAnisotropy == rhs.maxAnisotropy);
	}

	bool operator!=(const SamplerNtsp& rhs) {
		return !(*this == rhs);
	}
};

class SamplerNtspFileWidget : public QWidget {
	Q_OBJECT
public:
	SamplerNtspFileWidget(GlobalInfo& globalInfo);

	void setPath(const std::string& path);
	std::string getPath();

	void updateWidgets();
	void save();

private:
	std::string filterToType(const std::string& filterName);
	std::string typeToFilter(const std::string& type);
	std::string addressModeToType(const std::string& addressModeName);
	std::string typeToAddressMode(const std::string& type);
	std::string borderColorToType(const std::string& borderColorName);
	std::string typeToBorderColor(const std::string& type);

private slots:
	void onValueChanged();

private:
	GlobalInfo& m_globalInfo;

	std::string m_samplerFilePath;

public:
	SamplerNtsp samplerNtsp;

	ComboBoxWidget* magFilterWidget;
	ComboBoxWidget* minFilterWidget;
	ComboBoxWidget* mipmapFilterWidget;
	ComboBoxWidget* addressModeUWidget;
	ComboBoxWidget* addressModeVWidget;
	ComboBoxWidget* addressModeWWidget;
	ComboBoxWidget* borderColorWidget;
	ScalarWidget* minLodWidget;
	ScalarWidget* maxLodWidget;
	IntegerWidget* maxAnisotropyWidget;
};

class ChangeSamplerNtspFile : public QUndoCommand {
public:
	ChangeSamplerNtspFile(GlobalInfo& globalInfo, SamplerNtsp newSamplerNtsp, const std::string& filePath);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	SamplerNtspFileWidget* m_samplerNtspFileWidget;

	std::string m_filePath;
	SamplerNtsp m_oldSamplerNtsp;
	SamplerNtsp m_newSamplerNtsp;
};