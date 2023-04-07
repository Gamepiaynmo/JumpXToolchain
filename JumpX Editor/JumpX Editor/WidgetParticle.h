#pragma once

#include <QDialog>
#include "ui_WidgetParticle.h"

class WidgetParticle : public QWidget {
	Q_OBJECT

public:
	WidgetParticle(XParticle &prt, QListWidgetItem *item, QList<QString> &textures, QList<QString> &bones, QWidget *parent);

private:
	Ui::WidgetParticle ui;
	XParticle &m_prt;
	QListWidgetItem *m_item;

	QIntValidator m_intVal;
	QDoubleValidator m_doubleVal;
};
