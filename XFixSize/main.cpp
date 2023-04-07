#include "JumpX.h"
#pragma execution_character_set("utf-8")

QTextStream qout(stdout, QIODevice::WriteOnly);

bool process(QFile& input, QFile& output) {
	XScene* scene = new XScene();
	try {
		qout << QString("加载输入文件...") << endl << endl;

		scene->loadFromFile(input);
		while (scene->haveWarning())
			qout << scene->nextWarning() << endl;

		qout << QString("加载文件完毕") << endl;
		qout << QString("X文件版本 %1\n动画总帧数 %2\n%3 个贴图\n%4 个材质\n%5 个网格\n%6 个骨骼\n%7 个尾迹\n%8 个粒子\n%9 个动画")
			.arg(scene->m_head.fileVersion).arg(scene->m_head.frameCount).arg(scene->m_head.ntex).arg(scene->m_head.nmtl)
			.arg(scene->m_head.ngeo).arg(scene->m_head.nbon).arg(scene->m_head.nrib).arg(scene->m_head.nprt).arg(scene->m_head.nact) << endl;
		qout << endl;

		qout << QString("内部数据大小：%1 %2 10M，%3 %4 10M").arg(scene->m_head.dataSize).arg(scene->m_head.dataSize >= g_limit ? ">" : "<")
			.arg(scene->m_head.indexSize).arg(scene->m_head.indexSize >= g_limit ? ">" : "<") << endl;
		bool needFix = scene->m_head.dataSize >= g_limit || scene->m_head.indexSize >= g_limit;
		qout << QString(needFix ? "需要修复" : "无需修复") << endl << endl;

		if (needFix) {
			qout << QString("保存输出文件到：") << output.fileName() << endl << endl;

			scene->saveToFile(output);
			while (scene->haveWarning())
				qout << scene->nextWarning() << endl;

			qout << QString("修复后数据大小：%1 %2 10M，%3 %4 10M").arg(scene->m_head.dataSize).arg(scene->m_head.dataSize >= g_limit ? ">" : "<")
				.arg(scene->m_head.indexSize).arg(scene->m_head.indexSize >= g_limit ? ">" : "<") << endl;
			if (scene->m_head.dataSize >= g_limit || scene->m_head.indexSize >= g_limit)
				throw QString("文件太大无法修复，请手动优化模型");
			qout << QString("修复成功") << endl << endl;

			qout << QString("保存文件完毕") << endl;
		}
	}
	catch (QString msg) {
		qout << QString("错误：") << msg << endl;
	}

	delete scene;
	getchar();
	return true;
}

void seh_handler(unsigned int u, EXCEPTION_POINTERS* pExp) {
	throw QString("SEH 异常 ") + QString::number(pExp->ExceptionRecord->ExceptionCode);
}

int main(int argc, char *argv[]) {
	_set_se_translator(seh_handler);

    QCoreApplication a(argc, argv);
	qout << QString("JumpX 文件大小修复 V1.1") << endl;
	qout << QString("作者：黑の秋风") << endl << endl;

	if (argc < 2) {
		qout << QString("使用方法：XFixSize 输入文件 [输出文件]") << endl;
		qout << QString("或者直接将X文件拖动到此exe图标上") << endl;
		getchar();
		return 0;
	}

	QString input = argv[1];
	QString output;
	if (argc < 3) {
		QFileInfo info(input);
		output = info.absoluteDir().filePath(info.baseName() + "_fixed.x");
	} else output = argv[2];
	process(QFile(input), QFile(output));

    return 0;
}
