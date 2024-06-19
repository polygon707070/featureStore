/*
 * File:    labelcontroller.h
 * Author:  Rachel Bood
 * Date:    2014/11/07 (?)
 * Version: 1.5
 *
 * Purpose: 
 *
 * Modification history:
 *  Nov 13, 2019 (JD, V1.1)
 *   (a) Rename "Weight" to "Label" for edge function names.
 *  Nov 13, 2019 (JD, V1.2)
 *   (a) Fix incorrect #ifndef string.
 *  Jun 18, 2020 (IC, V1.3)
 *   (a) Added setEdgeLabel2() and setNodeLabel2() for updating the edit tab
 *       line edits when labels are changed via edit mode on the canvas.
 *  Jun 23, 2020 (IC, V1.4)
 *   (a) Rename two functions.
 *  Jul 14, 2020 (IC, V1.5)
 *   (a) Update ...EditLabel slot names to more meaningful values.
 */


#ifndef LABELCONTROLLER_H
#define LABELCONTROLLER_H
#include "edge.h"
#include "node.h"

#include <QLineEdit>
#include <QObject>

class LabelController: public QObject
{
    Q_OBJECT
public:
    LabelController(Edge * anEdge, QLineEdit * anEdit);
    LabelController(Node * aNode, QLineEdit * anEdit);

private slots:
    void setEdgeLabel(QString string);
    void setEdgeEditLabel();
    void setNodeLabel(QString string);
    void setNodeEditLabel();
    void deletedLineEdit();

private:
    Edge * edge;
    Node * node;
    QLineEdit * edit;
};

#endif // LABELCONTROLLER_H
