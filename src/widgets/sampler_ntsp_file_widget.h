#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include "integer_widget.h"
#include <string>

struct SamplerNtsp {
	std::string magFilter = "Linear";
	std::string minFilter = "Linear";
	std::string mipmapFilter = "Linear";
	std::string addressModeU = "Repeat";
	std::string addressModeV = "Repeat";
	std::string addressModeW = "Repeat";
	std::string borderColor = "FloatTransparentBlack";
	int anisotropyLevel = 0;

	bool operator==(const SamplerNtsp& rhs) {
		return (magFilter == rhs.magFilter) &&
			(minFilter == rhs.minFilter) &&
			(mipmapFilter == rhs.mipmapFilter) &&
			(addressModeU == rhs.addressModeU) &&
			(addressModeV == rhs.addressModeV) &&
			(addressModeW == rhs.addressModeW) &&
			(borderColor == rhs.borderColor) &&
			(anisotropyLevel == rhs.anisotropyLevel);
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
	IntegerWidget* anisotropyLevelWidget;
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