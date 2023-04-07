#include "stdafx.h"
#include "WidgetAttachment.h"

WidgetAttachment::WidgetAttachment(XAttachment &att, QListWidgetItem *item, QWidget *parent) : m_att(att), m_item(item), QWidget(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());

	ui.name->setText(m_att.name);
	connect(ui.name, &QLineEdit::editingFinished, [this]() { m_att.name = ui.name->text(); m_item->setText(m_att.name); });
}
