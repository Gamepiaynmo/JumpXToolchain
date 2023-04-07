#include "stdafx.h"
#include "DATEditor.h"
#include <QtWidgets/QApplication>

void seh_handler(unsigned int u, EXCEPTION_POINTERS* pExp) {
	throw QString("SEH Òì³£ ") + QString::number(pExp->ExceptionRecord->ExceptionCode);
}

int main(int argc, char *argv[])
{
	_set_se_translator(seh_handler);
#ifndef NDEBUG
	AllocConsole();
#endif

	QApplication a(argc, argv);
	DATEditor w;
	w.show();

	return a.exec();
}
