#include "combo_box_no_scroll_widget.h"

ComboBoxNoScrollWidget::ComboBoxNoScrollWidget() : QComboBox() {}

void ComboBoxNoScrollWidget::wheelEvent(QWheelEvent* event) {
	event->ignore();
}
