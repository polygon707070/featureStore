/*
 * File:    colourfillcontroller.h
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
 *  (a) Spelling corrections.
 */

#ifndef COLOURFILLCONTROLLER_H
#define COLOURFILLCONTROLLER_H

#include "node.h"
#include "defuns.h"

#include <QPushButton>
#include <QObject>

class ColourFillController : public QObject
{
    Q_OBJECT
public:
    ColourFillController(Node * aNode, QPushButton * aButton);

private slots:
    void setNodeFillColour();
    void deleteButton();

private:
    Node * node;
    QPushButton * button;
};

#endif // COLOURFILLCONTROLLER_H
