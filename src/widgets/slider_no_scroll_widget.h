#pragma once
#include <QSlider>
#include <QWheelEvent>

class SliderNoScrollWidget : public QSlider {
	Q_OBJECT
public:
	SliderNoScrollWidget();

private slots:
	void wheelEvent(QWheelEvent* event);
};