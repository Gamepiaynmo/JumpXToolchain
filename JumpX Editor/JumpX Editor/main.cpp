#include "stdafx.h"
#include "JumpXEditor.h"
#include <QtWidgets/QApplication>
#include "SplashScreen.h"
#include <fstream>

void seh_handler(unsigned int u, EXCEPTION_POINTERS* pExp) {
	throw QString("SEH Òì³£ ") + QString::number(pExp->ExceptionRecord->ExceptionCode);
}

void iterateXModels() {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	QFile inputFile("x_path.txt");
	inputFile.open(QIODevice::ReadOnly);
	QTextStream stream(&inputFile);
	QString line;
	int cnt = 0;
	while (stream.readLineInto(&line)) {
		XScene scene(false);
		cnt++;
		try {
			scene.loadFromFile(QFile(line));
		} catch (QString msg) {
			while (scene.haveWarning())
				msg += "\n" + scene.nextWarning();
			qDebug() << cnt << 15137 << line << msg;
			continue;
		}
		if (scene.m_head.nprt > 4)
			qDebug() << cnt << line << scene.m_head.nprt;
		//for (XBone &bon : scene.m_bones) {
		//	if (bon.saveFlag != 0 && bon.saveFlag != 3)
		//		qDebug() << cnt << 15137 << line << hex << bon.saveFlag;
		//}
		//for (XMaterial &mtl : scene.m_materials) {
		//	if (mtl.exData.flag & EFFECT_DISSOLVE)
		//		qDebug() << cnt << line << mtl.textureId;
		//}
		//for (int i = 0; i < scene.m_head.nprt; i++) {
		//	XParticle &prt = scene.m_particles[i];
		//	if (prt.seqLoopTimes != 0)
		//		qDebug() << cnt << line << i << prt.seqLoopTimes;
		//	//if ((prt.row > 1 || prt.col > 1) && (prt.lifeSpanHeadUVAnim[0] > 0 || prt.lifeSpanHeadUVAnim[1] > 0 || prt.decayHeadUVAnim[0] > 0 || prt.decayHeadUVAnim[1] > 0))
		//	//	qDebug() << cnt << line << i << prt.row << prt.col << prt.lifeSpanHeadUVAnim[0] << prt.lifeSpanHeadUVAnim[1] << prt.lifeSpanHeadUVAnim[2]
		//	//		<< prt.decayHeadUVAnim[0] << prt.decayHeadUVAnim[1] << prt.decayHeadUVAnim[2];
		//}
	}
	qDebug() << "finished";
}

int main(int argc, char *argv[]) {
	_set_se_translator(seh_handler);
#ifndef NDEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif
	QApplication a(argc, argv);

#ifdef NDEBUG
//	iterateXModels(); return a.exec();
#endif

#ifdef NDEBUG
	QPixmap pixmap("splash.png");
	SplashScreen screen(pixmap);
	screen.show();
#endif

	JumpXEditor w;
	w.show();
#ifdef NDEBUG
	screen.finish(&w);
#endif

	return a.exec();
}
