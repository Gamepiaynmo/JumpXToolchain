#pragma once

#include <QtWidgets/QDialog>
#include "ui_WidgetAction.h"

class WidgetAction : public QWidget {
	Q_OBJECT

public:
	WidgetAction(XAction &act, QListWidgetItem *item, QWidget *parent);

private:
	Ui::WidgetAction ui;
	XAction &m_act;
	QListWidgetItem *m_item;

	QIntValidator m_intVal;
};