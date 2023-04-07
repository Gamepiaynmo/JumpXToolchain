#pragma once

#include <QDialog>
#include "ui_WidgetBone.h"

class WidgetBone : public QWidget {
	Q_OBJECT

public:
	WidgetBone(XBone &bon, QTreeWidgetItem *item, QWidget *parent = Q_NULLPTR);

private:
	Ui::WidgetBone ui;
	XBone &m_bon;
	QTreeWidgetItem *m_item;
};
