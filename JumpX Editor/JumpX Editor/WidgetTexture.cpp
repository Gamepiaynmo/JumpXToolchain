#include "stdafx.h"
#include "WidgetTexture.h"
#include "CharWindow.h"
#include "JumpXEditor.h"

WidgetTexture::WidgetTexture(XTexture &tex, QListWidgetItem *item, CharWindow *charWind, JumpXEditor *parent) : m_tex(tex), m_item(item), m_charWind(charWind), m_parent(parent), QWidget(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());

	ui.name->setText(m_tex.name);
	connect(ui.name, &QLineEdit::editingFinished, [this]() { m_tex.name = ui.name->text(); m_item->setText(m_tex.name);
		m_charWind->updateTextures(); m_parent->updateTexturePreview(); });
}
