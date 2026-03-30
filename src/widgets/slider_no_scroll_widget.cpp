#include "slider_no_scroll_widget.h"

SliderNoScrollWidget::SliderNoScrollWidget() : QSlider() {}

void SliderNoScrollWidget::wheelEvent(QWheelEvent* event) {
	event->ignore();
}
