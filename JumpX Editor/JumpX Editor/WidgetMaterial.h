#pragma once

#include <QDialog>
#include "ui_WidgetMaterial.h"

class WidgetMaterial : public QWidget {
	Q_OBJECT

public:
	WidgetMaterial(XMaterial &mtl, QListWidgetItem *item, QList<QString> &textures, QWidget *parent);

	QColorSlider *colorBar() { return ui.colorBar; }
	XMaterial &mtl() { return m_mtl; }

private:
	Ui::WidgetMaterial ui;
	XMaterial &m_mtl;
	QListWidgetItem *m_item;

	QIntValidator m_intVal;
	QDoubleValidator m_doubleVal;
};
