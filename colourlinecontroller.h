/*
 * File:    colourlinecontroller.h
 * Author:  Rachel Bood 100088769
 * Date:    2014 (?)
 * Version: 1.2
 *
 * Purpose:
 *
 * Modification history:
 * Jul 9, 2020 (IC V1.1)
 *  (a) #include defuns.h to get BUTTON_STYLE.
 * Oct 18, 2020 (JD V1.2)
 *  (a) Spelling corrections, fix incorrect #ifndef token.
 */

#ifndef COLOURLINECONTROLLER_H
#define COLOURLINECONTROLLER_H

#include "edge.h"
#include "node.h"
#include "defuns.h"

#include <QPushButton>
#include <QObject>

class ColourLineController: public QObject
{
    Q_OBJECT

public:
    ColourLineController(Edge * anEdge, QPushButton * aButton);
    ColourLineController(Node * anNode, QPushButton * aButton);

private slots:
    void setEdgeLineColour();
    void setNodeOutlineColour();
    void deleteButton();

private:
    Edge * edge;
    Node * node;
    QPushButton * button;
};

#endif // COLOURLINECONTROLLER_H
