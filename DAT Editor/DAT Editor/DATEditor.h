#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DATEditor.h"
#include "DATData.h"

class DATEditor : public QMainWindow
{
	Q_OBJECT

public:
	DATEditor(QWidget *parent = Q_NULLPTR);

private slots:
	void on_Action_Open();
	void on_Action_Save();
	void on_Action_SaveAs();
	void on_Action_Close();
	void on_Action_Quit();
	void on_Action_Import();
	void on_Action_Export();

	void on_Action_MLineEdit();
	void on_Action_Find();
	void on_Action_FindNext();
	void on_Action_Replace();
	void on_Action_ReplaceNext();
	void on_Action_ReplaceAll();
	void on_Action_JumpToRow();
	void on_Action_JumpToColumn();

	void on_Action_About();

	void on_Data_Changed(QStandardItem *item);

private:
	void setupTable();
	void clearTable();

	bool saveScene(QString path);
	bool searchNext();
	void replaceCurrent();

	Ui::DATEditorClass ui;
	QStandardItemModel* tableModel = nullptr;
	QLabel* statusText = nullptr;
	QLabel* statusInfo = nullptr;

	QString curPath, curName;
	DATData *m_data = nullptr, *m_orindata = nullptr;

	QString searchStr, replaceStr;

	const QStringList m_supdats = {
		"bulletdata_c.dat",
		"bulletstatudata_c.dat",
		"designation_c.dat",
		"designation_lan_c.dat",
		"hero_c.dat",
		"heroskin_c.dat",
		"item_item_c.dat",
		"item_item_lan_c.dat",
		"monster_c.dat",
		"monster_lan_c.dat",
		"skill_skillbase_c.dat",
		"skill_skilllevel_c.dat",
		"skill_skilllevel_lan_c.dat",
		"skill_statuslist_c.dat",
		"string_client_c.dat",
		"sound_actsound_c.dat"
	};

	/*	0 [0, int]
		1 [0, int] [1, int]
		2 [0, int] [2, int]
		3 [0, string]
	*/
	const int m_indexmethod[16] = {
		0, 0, 0, 0, 0, 1, /*heroskin*/
		0, 0, 0, 0, 1, 1, 2, 1, 3, 0
	};
};
