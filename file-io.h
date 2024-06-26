/*
 * File:	file-io.h
 * Author:	Jim Diamond
 * Date:	2020-10-22
 * Version:	1.0
 *
 * Purpose:	This class holds all the functions which read or write
 *		files (except for the settings, which is taken care of
 *		elsewhere).
 *
 * Notes:	Many functions are "static" so that they can be called
 *		from other modules (required since currently there is
 *		no File_IO object).  Note that C++ does not require
 *		these to be declared static in the .cpp file, and, in
 *		fact, that won't work.
 *
 * Modification history:
 * Oct 22, 2020 (JD V1.0)
 *  (a) Initial revision.  Functions and structs extracted from
 *	mainwindow.cpp and mainwindow.h.
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include <QTextStream>

#include "node.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#define GRAPHiCS_FILE_EXTENSION "grphc"
#define GRAPHiCS_SAVE_FILE	"Graph-ic (*." GRAPHiCS_FILE_EXTENSION ")"
#define GRAPHiCS_SAVE_SUBDIR	"graph-ic"

class File_IO
{
public:
    static bool saveTikZ(QTextStream &outfile, QVector<Node *> nodes);
    static bool saveEdgelist(QTextStream &outfile, QVector<Node *> nodes);
    static bool saveGraphIc(QTextStream &outfile, QVector<Node *> nodes, 
			    bool outputExtra);
    static bool saveGraph(bool * promptSave, QWidget * parent,
			   Ui::MainWindow * ui);
    static bool loadGraphicFile(QWidget * parent, Ui::MainWindow * ui);
    static void loadGraphicLibrary(Ui::MainWindow * ui);
    static void inputCustomGraph(bool prependDirPath, QString graphName,
				 Ui::MainWindow * ui);
    static void setFileDirectory(QWidget * parent);

protected:

private:
    typedef struct
    {
	int fillR, fillG, fillB;
	int lineR, lineG, lineB;
	qreal nodeDiameter;	// inches
	qreal penSize;		// pixels (!); thickness of line.
	qreal labelSize;	// points; See Node::setNodeLabelSize()
    } nodeInfo;

    typedef struct
    {
	int lineR, lineG, lineB;
	qreal penSize;		// pixels (!); thickness of line.
	qreal labelSize;	// points
    } edgeInfo;

    static void findDefaults(QVector<Node *> nodes, nodeInfo * nodeDefaults_p,
			     edgeInfo * edgeDefaults_p);
    static QString lookupColour(QColor colour);
    static void inputCustomGraphOriginal(QString graphFileName,
					 Ui::MainWindow * ui);
};

#endif // FILE_IO_H
