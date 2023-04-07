#pragma once

#include <QDialog>
#include "ui_DialogAbout.h"

class DialogAbout : public QDialog
{
	Q_OBJECT

public:
	DialogAbout(QWidget *parent = Q_NULLPTR);
	~DialogAbout();

private:
	Ui::DialogAbout ui;
};
