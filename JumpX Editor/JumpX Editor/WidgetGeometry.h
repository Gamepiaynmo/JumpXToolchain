#pragma once

#include <QDialog>
#include "ui_WidgetGeometry.h"

class WidgetGeometry : public QWidget {
	Q_OBJECT

public:
	WidgetGeometry(XGeometry &geo, QListWidgetItem *item, QList<QString> &materials, QList<QString> &bones, QWidget *parent);

private:
	Ui::WidgetGeometry ui;
	XGeometry &m_geo;
	QListWidgetItem *m_item;
};
