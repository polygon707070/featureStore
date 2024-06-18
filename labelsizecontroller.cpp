/*
 * File:    labelsizecontroller.cpp
 * Author:  Rachel Bood
 * Date:    2014/11/07 (?)
 * Version: 1.5
 *
 * Purpose: Initializes a QGraphicsView that is used to house the QGraphicsScene
 *
 * Modification history:
 * Nov 13, 2019 (JD, V1.1):
 *  (a) Renamed setWeightLabelSize() -> setEdgeLabelSize().
 *  (b) Removed apparently redundant "|| edge != 0" in setEdgeLabelSize().
 * Nov 24, 2019 (JD, V1.2):
 *  (a) Fixed bugs in the (node and edge) constructors which
 *	incorrectly set the text size box value.
 *  (b) Minor formatting tweaks, comment additions.
 *  (c) Removed apparently redundant "|| box != 0" tests.
 * Dec 1, 2019 (JD, V1.3)
 *  (a) Previously if there was no label, the label size in the "Edit
 *	Graph" tab would get sizes of 12, rather than whatever size
 *	was stored in the edge/node object.  Henceforth recognize the
 *	size in that field even when there is currently no label.
 *	(Assumption: the label sizes have been set to meaningful
 *	values.)
 *  (b) Make the font sizes integers strictly larger than 0.  See note
 *	below in LabelSizeController(edge, box) comment.
 * June 9, 2020 (IC V1.4)
 *  (a) Changed QDoubleSpinBox to QSpinBox and Double to Int where applicable.
 * Aug 21, 2020 (IC V1.5)
 *  (a) Use new name of the function which sets an edge label.
 */

#include "labelsizecontroller.h"


/*
 * Name:	LabelSizeController()
 * Purpose:	
 * Arguments:	
 * Outputs:	
 * Modifies:	
 * Returns:	
 * Assumptions:	
 * Bugs:	
 * Notes:	It would seem that, at least in Qt 5.9.8 on Linux,
 *		fractional parts of font sizes are ignored (or rounded down?)
 *		and setting a font size of 0 causes Qt to spit out a
 *		complaint. So make the font size spin boxes show no
 *		decimal places, and also start at 1 and go up from there.
 *		UPDATE: No longer doublespinboxes so no need to set decimals.
 */

LabelSizeController::LabelSizeController(Edge * anEdge, QSpinBox * aBox)
{
    edge = anEdge;
    box = aBox;
    if (box != nullptr)
    {
	box->setMinimum(1);
	box->setValue(edge->getLabelSize());

        connect(box, SIGNAL(valueChanged(int)),
                this, SLOT(setEdgeLabelSize(int)));
        connect(anEdge, SIGNAL(destroyed(QObject*)),
                this, SLOT(deletedSpinBox()));
        connect(anEdge, SIGNAL(destroyed(QObject*)),
                this, SLOT(deleteLater()));
    }
}



LabelSizeController::LabelSizeController(Node * aNode, QSpinBox * aBox)
{
    node = aNode;
    box = aBox;
    if (box != nullptr)
    {
	box->setMinimum(1);
	box->setValue(node->getLabelSize());

        connect(box, SIGNAL(valueChanged(int)),
                this, SLOT(setNodeLabelSize(int)));
        connect(aNode, SIGNAL(destroyed(QObject*)),
                this, SLOT(deletedSpinBox()));
        connect(aNode, SIGNAL(destroyed(QObject*)),
                this, SLOT(deleteLater()));
    }
}



/*
 * Name:	setNodeLabelSize()
 * Purpose:	Set the size of the node label.
 * Arguments:	The size, in points.
 * Outputs:	Nothing.
 * Modifies:	The node label size.
 * Returns:	Nothing.
 * Assumptions:	None.
 * Bugs:	?
 * Notes:	Can node possibly be null?
 */

void LabelSizeController::setNodeLabelSize(int ptSize)
{
    if (node != nullptr)
        node->setNodeLabelSize(ptSize);
}



void LabelSizeController::deletedSpinBox()
{
    delete box;
}



/*
 * Name:	setEdgeLabelSize()
 * Purpose:	Set the size of the edge label.
 * Arguments:	The size, in points.
 * Outputs:	Nothing.
 * Modifies:	The edge label size.
 * Returns:	Nothing.
 * Assumptions:	None.
 * Bugs:	?
 * Notes:	Can edge possibly be null?
 */

void
LabelSizeController::setEdgeLabelSize(int ptSize)
{
    if (edge != nullptr)
        edge->setEdgeLabelSize(ptSize);
}
