/*
 * File:    main.cpp
 * Author:  Rachel Bood 100088769
 * Date:    2014/11/07
 * Version: 1.6
 *
 * Purpose: executes the mainwindow.ui.
 *
 * Modification history:
 * May 8, 2020 (IC/JD V1.1)
 *  (a) Added calls to addApplicationFont(), since newer versions of Qt
 *	(apparently) need this to use the included fonts.
 * May 12, 2020 (IC V1.2)
 *  (a) Added another font (arimo.ttf) to be embedded, so that Graphic
 *      can look as similar as possible on different systems.
 * May 13, 2020 (JD V1.3)
 *  (a) Update comments.
 * Jun 6, 2020 (IC V1.4)
 *  (a) Call set_Interface_Sizes() after show() to get accurate sizehints
 *      when resizing the window.
 * Aug 18, 2020 (IC V1.5)
 *  (a) Added cmtt10 and cmsy10 fonts to be embedded.
 * Sep 11, 2020 (JD V1.6)
 *  (a) Replace (most of) cmr10, cmmi10 and cmsy10 with cmzsd10.
 *      See corresponding simplifications of html-label.cpp
 *	and comment about why I still embed cmr10.
 */

#include "mainwindow.h"
#include <QApplication>
#include <QFileSystemModel>
#include <QTreeView>
#include <QFontDatabase>

int
main(int argc, char * argv[])
{
    QApplication a(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/arimo.ttf");
    QFontDatabase::addApplicationFont(":/fonts/cmr10.ttf");
    QFontDatabase::addApplicationFont(":/fonts/cmtt10.ttf");
    QFontDatabase::addApplicationFont(":/fonts/cmzsd10.ttf");

    MainWindow w;
    w.show();
    w.set_Interface_Sizes();

    return a.exec();
}
