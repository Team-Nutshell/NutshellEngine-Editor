#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QMouseEvent>
#include <string>
#include <limits>

class ScalarSliderWidget : public QWidget {
	Q_OBJECT
public:
	ScalarSliderWidget(GlobalInfo& globalInfo, const std::string& name, float min, float max);

	void setValue(float value);
	float getValue();

	void setTemporaryValue(float value);
	float getTemporaryValue();

	void setMin(float min);
	void setMax(float max);

signals:
	void valueChanged(float);
	void temporaryValueChanged(float);

private slots:
	void onEditingFinished();
	void onSliderReleased();
	void onSliderMoved();
	void onSliderValueChanged();

private:
	GlobalInfo& m_globalInfo;

	float m_value = 0.0f;
	float m_temporaryValue = 0.0f;

	float m_min = std::numeric_limits<float>::lowest();
	float m_max = std::numeric_limits<float>::max();

public:
	QLabel* nameLabel;
	QLineEdit* valueLineEdit;
	QSlider* slider;
};