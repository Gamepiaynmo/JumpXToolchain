#include "stdafx.h"
#include "DialogAbout.h"

DialogAbout::DialogAbout(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());
	ui.label->setText(ui.label->text() + EDITOR_VERSION);
}

DialogAbout::~DialogAbout()
{
}
