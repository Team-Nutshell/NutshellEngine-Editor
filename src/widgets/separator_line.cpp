#include "separator_line.h"

SeparatorLine::SeparatorLine() {
	setFrameShape(QFrame::Shape::HLine);
	setLineWidth(1);
	setStyleSheet("color: rgba(255, 255, 255, 120);");
}