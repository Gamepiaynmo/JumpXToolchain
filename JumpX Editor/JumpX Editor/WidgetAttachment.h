#pragma once

#include "ui_WidgetAttachment.h"

class WidgetAttachment : public QWidget {
	Q_OBJECT

public:
	WidgetAttachment(XAttachment &att, QListWidgetItem *item, QWidget *parent = Q_NULLPTR);

private:
	Ui::WidgetAttachment ui;
	XAttachment &m_att;
	QListWidgetItem *m_item;
};

