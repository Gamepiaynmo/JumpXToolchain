#include "stdafx.h"
#include "WidgetBone.h"

WidgetBone::WidgetBone(XBone &bon, QTreeWidgetItem *item, QWidget *parent) : m_bon(bon), m_item(item), QWidget(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());

	ui.name->setText(m_bon.name);
	connect(ui.name, &QLineEdit::editingFinished, [this]() { m_bon.name = ui.name->text(); m_item->setText(0, m_bon.name); });
}
