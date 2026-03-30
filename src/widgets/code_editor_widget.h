#pragma once
#include "text_edit_line_number.h"
#include <QTextEdit>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QResizeEvent>

class CodeEditorWidget : public QTextEdit {
	Q_OBJECT
public:
	CodeEditorWidget(QWidget* parent = nullptr);

	int getFirstVisibleBlock();
	void lineNumberAreaPaintEvent(QPaintEvent* event);
	int lineNumberAreaWidth();

protected:
	void focusOutEvent(QFocusEvent* event);

public slots:
	void keyPressEvent(QKeyEvent* event);
	void resizeEvent(QResizeEvent* event);

private slots:
	void updateLineNumberAreaWidth();
	void updateLineNumberArea();

signals:
	void valueChanged(const std::string&);

private:
	TextEditLineNumber* m_textEditLineNumber;
};