#pragma once

#include <QDialog>
#include "ui_WidgetTexture.h"

class CharWindow;
class JumpXEditor;

class WidgetTexture : public QWidget {
	Q_OBJECT

public:
	WidgetTexture(XTexture &tex, QListWidgetItem *item, CharWindow *charWind, JumpXEditor *parent = Q_NULLPTR);

private:
	Ui::WidgetTexture ui;
	XTexture &m_tex;
	QListWidgetItem *m_item;
	CharWindow *m_charWind;
	JumpXEditor *m_parent;
};
