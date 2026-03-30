#include "text_edit_line_number.h"
#include "code_editor_widget.h"

TextEditLineNumber::TextEditLineNumber(QTextEdit* textEdit) : m_textEdit(textEdit) {
	setParent(m_textEdit);
}

QSize TextEditLineNumber::sizeHint() const {
	return QSize(static_cast<CodeEditorWidget*>(m_textEdit)->lineNumberAreaWidth(), 0);
}

void TextEditLineNumber::paintEvent(QPaintEvent* event) {
	static_cast<CodeEditorWidget*>(m_textEdit)->lineNumberAreaPaintEvent(event);
}