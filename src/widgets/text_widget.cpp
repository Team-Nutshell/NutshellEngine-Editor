#include "text_widget.h"

void TextWidget::focusOutEvent(QFocusEvent* event) {
	emit valueChanged(toPlainText().toStdString());
	QTextEdit::focusOutEvent(event);
}