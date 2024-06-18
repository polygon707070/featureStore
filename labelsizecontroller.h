/*
 * File:    labelsizecontroller.h
 * Author:  Rachel Bood
 * Date:    2014/11/07 (?)
 * Version: 1.2
 *
 * Purpose: Declare the node class.
 * 
 * Modification history:
 * Nov 13, 2019 (JD V1.1)
 *  (a) Rename setEdgeWeightSize() -> setEdgeLabelSize().
 *  (b) Fix incorrect #ifndef token name.
 * June 9, 2020 (IC V1.2)
 *  (a) Changed QDoubleSpinBox to QSpinBox and Double to Int where applicable.
 */


#ifndef LABELSIZECONTROLLER_H
#define LABELSIZECONTROLLER_H
#include "edge.h"
#include "node.h"

#include <QSpinBox>
#include <QObject>

class LabelSizeController : public QObject
{
    Q_OBJECT
public:
    LabelSizeController(Edge * anEdge, QSpinBox * aBox);
    LabelSizeController(Node * aNode, QSpinBox * aBox);

private slots:
    void setEdgeLabelSize(int value);
    void setNodeLabelSize(int value);
    void deletedSpinBox();

private:
    Edge * edge;
    Node * node;
    QSpinBox * box;

};

#endif // LABELSIZECONTROLLER_H
