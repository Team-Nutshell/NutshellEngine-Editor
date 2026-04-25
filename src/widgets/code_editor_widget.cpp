#include "code_editor_widget.h"
#include <QScrollBar>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QPainter>

CodeEditorWidget::CodeEditorWidget(QWidget* parent) : QTextEdit(parent), m_textEditLineNumber(new TextEditLineNumber(this)) {
	setTabStopDistance(QFontMetricsF(font()).horizontalAdvance(' ') * 4.0f);
	setAcceptRichText(false);

	connect(document(), &QTextDocument::blockCountChanged, this, &CodeEditorWidget::updateLineNumberAreaWidth);
	connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &CodeEditorWidget::updateLineNumberArea);
	connect(this, &QTextEdit::textChanged, this, &CodeEditorWidget::updateLineNumberArea);
	connect(this, &QTextEdit::cursorPositionChanged, this, &CodeEditorWidget::updateLineNumberArea);

	updateLineNumberAreaWidth();
}

int CodeEditorWidget::getFirstVisibleBlock() {
	QTextCursor cursor = textCursor();
	cursor.movePosition(QTextCursor::MoveOperation::Start);
	for (int i = 0; i < document()->blockCount(); i++) {
		QTextBlock block = cursor.block();
		QRect rect1 = viewport()->geometry();
		QRect rect2 = document()->documentLayout()->blockBoundingRect(block).translated(viewport()->geometry().x(), viewport()->geometry().y() - verticalScrollBar()->sliderPosition()).toRect();
		if (rect1.contains(rect2, true)) {
			return i;
		}

		cursor.movePosition(QTextCursor::NextBlock);
	}

	return 0;
}

void CodeEditorWidget::lineNumberAreaPaintEvent(QPaintEvent* event) {
	verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition());

	QPainter painter(m_textEditLineNumber);
	painter.fillRect(event->rect(), QColor::fromRgbF(0.1f, 0.1f, 0.1f));
	int blockNumber = getFirstVisibleBlock();

	QTextBlock block = document()->findBlockByNumber(blockNumber);
	QTextBlock prevBlock = (blockNumber > 0) ? document()->findBlockByNumber(blockNumber - 1) : block;
	int translateY = (blockNumber > 0) ? -verticalScrollBar()->sliderPosition() : 0;

	int top = viewport()->geometry().top();

	int additionalMargin;
	if (blockNumber == 0) {
		additionalMargin = document()->documentMargin() - 1 - verticalScrollBar()->sliderPosition();
	}
	else {
		additionalMargin = document()->documentLayout()->blockBoundingRect(prevBlock).translated(0, translateY).intersected(viewport()->geometry()).height();
	}
	top += additionalMargin;

	int bottom = top + document()->documentLayout()->blockBoundingRect(prevBlock).height();

	QColor colorCurrentLine(QColor::fromRgbF(0.5f, 0.5f, 0.5f));
	QColor colorOtherLines(QColor::fromRgbF(0.4f, 0.4f, 0.4f));

	while (block.isValid() && (top <= event->rect().bottom())) {
		if (block.isVisible() && (bottom >= event->rect().top())) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen((textCursor().blockNumber() == blockNumber) ? colorCurrentLine : colorOtherLines);
			QFont font = painter.font();
			font.setBold((textCursor().blockNumber() == blockNumber));
			painter.setFont(font);
			painter.drawText(-5, top, m_textEditLineNumber->width(), fontMetrics().height(), Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + document()->documentLayout()->blockBoundingRect(block).height();
		blockNumber++;
	}
}

int CodeEditorWidget::lineNumberAreaWidth() {
	int digits = 1;
	int max = std::max(1, document()->blockCount());
	while (max >= 10) {
		max /= 10;
		digits++;
	}

	int space = 13 + fontMetrics().horizontalAdvance('0') * digits;

	return space;
}

void CodeEditorWidget::focusOutEvent(QFocusEvent* event) {
	emit valueChanged(toPlainText().toStdString());
	QTextEdit::focusOutEvent(event);
}

void CodeEditorWidget::keyPressEvent(QKeyEvent* event) {
	if (textCursor().hasSelection()) {
		if (event->key() == Qt::Key_Tab) {
			QTextCursor cursor = textCursor();
			cursor.beginEditBlock();
			int cursorStartPosition = textCursor().selectionStart();
			int cursorEndPosition = textCursor().selectionEnd();

			// Before cursor
			size_t newlinePos = toPlainText().toStdString().substr(0, cursorStartPosition).find_last_of('\n');
			if (newlinePos != std::string::npos) {
				cursor.setPosition(static_cast<int>(newlinePos) + 1);
				cursor.insertText("\t");
				cursorStartPosition++;
				cursorEndPosition++;
			}
			else {
				cursor.setPosition(0);
				cursor.insertText("\t");
				cursorStartPosition++;
				cursorEndPosition++;
			}

			// After cursor
			newlinePos = toPlainText().toStdString().find('\n', cursor.position());
			while ((newlinePos != std::string::npos) && ((static_cast<int>(newlinePos) + 1) <= cursorEndPosition)) {
				cursor.setPosition(static_cast<int>(newlinePos) + 1);
				cursor.insertText("\t");
				cursorEndPosition++;
				newlinePos = toPlainText().toStdString().find('\n', cursor.position());
			}

			cursor.setPosition(cursorStartPosition);
			cursor.setPosition(cursorEndPosition, QTextCursor::MoveMode::KeepAnchor);
			cursor.endEditBlock();
			setTextCursor(cursor);
		}
		else if (event->key() == Qt::Key_Backtab) {
			QTextCursor cursor = textCursor();
			cursor.beginEditBlock();
			int cursorStartPosition = textCursor().selectionStart();
			int cursorEndPosition = textCursor().selectionEnd();
			
			// Before cursor
			size_t newlinePos = toPlainText().toStdString().substr(0, cursorStartPosition).find_last_of('\n');
			if (newlinePos != std::string::npos) {
				if (toPlainText().toStdString()[newlinePos + 1] == '\t') {
					cursor.setPosition(static_cast<int>(newlinePos) + 1);
					cursor.deleteChar();
					if (cursorStartPosition > (static_cast<int>(newlinePos) + 1)) {
						cursorStartPosition--;
					}
					cursorEndPosition--;
				}
			}
			else {
				if (toPlainText().toStdString()[0] == '\t') {
					cursor.setPosition(0);
					cursor.deleteChar();
					if (cursorStartPosition > 0) {
						cursorStartPosition--;
					}
					cursorEndPosition--;
				}
			}
			cursor.setPosition(cursorStartPosition);
			newlinePos = toPlainText().toStdString().find('\n', cursor.position());
			while ((newlinePos != std::string::npos) && ((static_cast<int>(newlinePos) + 1) < cursorEndPosition)) {
				cursor.setPosition(static_cast<int>(newlinePos) + 1);
				if (toPlainText().toStdString()[cursor.position()] == '\t') {
					cursor.deleteChar();
					cursorEndPosition--;
				}
				newlinePos = toPlainText().toStdString().find('\n', cursor.position());
			}

			cursor.setPosition(cursorStartPosition);
			cursor.setPosition(cursorEndPosition, QTextCursor::MoveMode::KeepAnchor);
			cursor.endEditBlock();
			setTextCursor(cursor);
		}
		else if (event->key() == Qt::Key_ParenLeft) {
			QTextCursor cursor = textCursor();
			cursor.beginEditBlock();
			int cursorStartPosition = textCursor().selectionStart();
			int cursorEndPosition = textCursor().selectionEnd();

			cursor.setPosition(cursorStartPosition);
			cursor.insertText("(");
			cursorEndPosition++;
			cursor.setPosition(cursorEndPosition);
			cursor.insertText(")");
			cursorEndPosition++;

			cursor.setPosition(cursorEndPosition);
			cursor.endEditBlock();
			setTextCursor(cursor);
		}
		else if (event->key() == Qt::Key_Apostrophe) {
			QTextCursor cursor = textCursor();
			cursor.beginEditBlock();
			int cursorStartPosition = textCursor().selectionStart();
			int cursorEndPosition = textCursor().selectionEnd();

			cursor.setPosition(cursorStartPosition);
			cursor.insertText("\'");
			cursorEndPosition++;
			cursor.setPosition(cursorEndPosition);
			cursor.insertText("\'");
			cursorEndPosition++;

			cursor.setPosition(cursorEndPosition);
			cursor.endEditBlock();
			setTextCursor(cursor);
		}
		else if (event->key() == Qt::Key_QuoteDbl) {
			QTextCursor cursor = textCursor();
			cursor.beginEditBlock();
			int cursorStartPosition = textCursor().selectionStart();
			int cursorEndPosition = textCursor().selectionEnd();

			cursor.setPosition(cursorStartPosition);
			cursor.insertText("\"");
			cursorEndPosition++;
			cursor.setPosition(cursorEndPosition);
			cursor.insertText("\"");
			cursorEndPosition++;

			cursor.setPosition(cursorEndPosition);
			cursor.endEditBlock();
			setTextCursor(cursor);
		}
		else {
			QTextEdit::keyPressEvent(event);
		}
	}
	else {
		if (event->key() == Qt::Key_Backtab) {
			QTextCursor cursor = textCursor();
			cursor.beginEditBlock();
			if (cursor.position() != 0) {
				if (toPlainText().toStdString()[(cursor.position() - 1)] == '\t') {
					cursor.deletePreviousChar();
				}
			}
			cursor.endEditBlock();
			setTextCursor(cursor);
		}
		else {
			QTextEdit::keyPressEvent(event);
		}
	}
}

void CodeEditorWidget::resizeEvent(QResizeEvent* event) {
	QTextEdit::resizeEvent(event);

	QRect rect = contentsRect();
	m_textEditLineNumber->setGeometry(QRect(rect.left(), rect.top(), lineNumberAreaWidth(), rect.height()));
}

void CodeEditorWidget::updateLineNumberAreaWidth() {
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditorWidget::updateLineNumberArea() {
	verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition());

	QRect rect = contentsRect();
	m_textEditLineNumber->update(0, rect.y(), m_textEditLineNumber->width(), rect.height());
	updateLineNumberAreaWidth();
	int dy = verticalScrollBar()->sliderPosition();
	if (dy > -1) {
		m_textEditLineNumber->scroll(0, dy);
	}

	int firstBlockID = getFirstVisibleBlock();
	if ((firstBlockID == 0) || (textCursor().block().blockNumber() == firstBlockID - 1)) {
		verticalScrollBar()->setSliderPosition(dy - document()->documentMargin());
	}
}