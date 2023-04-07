#include "stdafx.h"
#include "SplashScreen.h"

SplashScreen::SplashScreen(const QPixmap &pixmap)
	: QSplashScreen(pixmap)
{
	setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SplashScreen | Qt::FramelessWindowHint);

	setAttribute(Qt::WA_NoBackground, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_TranslucentBackground, true);
}

SplashScreen::~SplashScreen()
{
}

void SplashScreen::paintEvent(QPaintEvent *e) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	return QSplashScreen::paintEvent(e);
};