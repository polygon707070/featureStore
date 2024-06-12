/*
 * File:    sizecontroller.h
 * Author:  Rachel Bood
 * Date:    2014/??/??
 * Version: 1.2
 *
 * Modification history:
 * Jul 15, 2020 (IC V1.1)
 *  (a) Updated the node sizecontroller to take two spinboxes as parameters,
 *      one for node penwidth and the other for node diameter.
 *  (b) Added setNodeSize2 to handle the new thickness box and a node specific
 *      delete function.
 * Jul 30, 2020 (JD V 1.2)
 *  (a) Fix the V1.1 comment.  Duh.
 */

#ifndef EDGESIZECONTROLLER_H
#define EDGESIZECONTROLLER_H

#include "edge.h"
#include "node.h"

#include <QObject>
#include <QDoubleSpinBox>

class SizeController : QObject
{
    Q_OBJECT

public:
    SizeController(Edge * anEdge, QDoubleSpinBox * aBox);
    SizeController(Node * aNode, QDoubleSpinBox * diamBox,
		   QDoubleSpinBox * thicknessBox);


private slots:
    void setEdgeSize(double value);
    void setNodeSize(double value);
    void setNodeSize2(double value);
    void deletedEdgeBox();
    void deletedNodeBoxes();

private:
    Edge * edge;
    Node * node;
    QDoubleSpinBox * box1;
    QDoubleSpinBox * box2;
};

#endif // EDGESIZECONTROLLER_H
