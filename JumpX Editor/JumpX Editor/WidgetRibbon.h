#pragma once

#include <QDialog>
#include "ui_WidgetRibbon.h"

class WidgetRibbon : public QWidget {
	Q_OBJECT

public:
	WidgetRibbon(XRibbon &rib, QListWidgetItem *item, QList<QString> &textures, QList<QString> &bones, QWidget *parent);

private:
	Ui::WidgetRibbon ui;
	XRibbon &m_rib;
	QListWidgetItem *m_item;

	QIntValidator m_intVal;
};
