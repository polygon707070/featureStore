/*
 * File:    preview.cpp
 * Author:  Rachel Bood 100088769
 * Date:    2014/11/07
 * Version: 1.17
 *
 * Purpose: Initializes a QGraphicsView that is used to house the QGraphicsScene
 *
 * Modification history:
 * Nov 13, 2019 (JD, V1.1):
 *  (a) Rename setWeightLabelSize() to setLabelSize().
 * Nov 13, 2019 (JD, V1.2)
 *  (a) Rename "Weight" to "Label" for edge function names.
 * Nov 13, 2019 (JD, V1.3)
 *  (a) Some formatting and comment tweaks.
 *  (b) Added Ctrl-= as a way to zoom in the preview pane.
 * Dec 1, 2019 (JD, V1.4)
 *  (a) Added qDeb() / DEBUG stuff.
 *  (b) Set edges' source radius correctly (fwiw) in Style_Graph().
 *  (c) Added some debug outputs and some comment additions/mods.
 *  (d) In Style_Graph() call setNodeLabelSize() regardless of
 *	whether or not the node is getting a label.  Simplify the
 *	related code in that part of the function.
 *  (e) Also in Style_Graph() call edge->setSourceRadius() with a value
 *	which seems to be more sensible to me.
 * Dec 7, 2019 (JD V1.5)
 *  (a) Use the currentGraphTypeIndex field in Create_Graph() to
 *	decide whether we have to create a new graph or just modify
 *	the one we already have.
 *  (b) Remove node->edgeWeight, which is used nowhere.
 *  (c) Redesign code so that when a graph is created it is designed
 *	for a 1"x1" region, and only when it is styled are the node
 *	positions set as per the size widgets.  This is a step toward
 *	allowing library graphs to be scaled (and styled in other ways).
 *  (c) With that, redesign code so that a graph is not re-created
 *	every time a style parameter is changed.  (That is, create a
 *	new graph only when the graph type, number of nodes, or
 *	whether to draw edges changes.)
 *  (d) Yet more comment additions and formatting changes.
 *      In particular rename "complete" to "drawEdges", since
 *	this checkBox refers to a different notion than that of a
 *	complete graph.
 *  (e) Obsessively alphabetize the cases in Create_Graph().
 * Dec 12, 2019 (JD V1.6)
 *  (a) #include defuns.h and remove debug #ifdef stuff.
 *  (b) Another batch of comments, debug stmts, identifier improvements.
 *  (c) Along with changes in mainwindow.cpp, implement the ability to
 *	style library graphs.  Part of this involves changing
 *	Style_Graph() so that individual styles can be applied,
 *	rather than the former "apply everything" approach.
 * May 11, 2020 (IC V1.7)
 *  (a) Changed the width/height values of the preview window from
 *	logicalDotsPerInch to physicalDotsPerInch to correct scaling issues
 *	(Only reliable with Qt V5.14.2 or higher)
 * May 25, 2020 (IC V1.8)
 *  (a) Added numStart param to Style_Graph() to allow numbering of nodes
 *	to start at a specified value instead of only 0.
 *	Check for that widget being changed in Style_Graph().
 * Jul 3, 2020 (IC V1.9)
 *  (a) Added nodeThickness param to Style_Graph() to allow adjusting
 *      thickness of nodes.
 *  (b) Display the zoom amount.  Fixed keyPressEvent to require Ctrl+
 *	and Ctrl- to zoom out/in instead of just + and - keys.
 * Jul 7, 2020 (IC V1.10)
 *  (a) Updated scaleView to accomodate for the new visible zoomDisplay on the
 *      preview pane ui.
 * Aug 11, 2020 (IC V1.11)
 *  (a) Added wheelEvent to allow for zooming using the mouse wheel.
 *  (b) Update names for (the former) node size and edge thickness.
 * Aug 12, 2020 (IC V1.12)
 *  (a) Updated Style_Graph() to use global physicalDPI values for xDPI and
 *      yDPI.
 *  (b) Created macros to be used for zoom level min and max for clarity.
 * Aug 14, 2020 (IC V1.13)
 *  (a) Add new param to setRotation().
 * Aug 21, 2020 (IC V1.14)
 *  (a) Added the ability to number edge labels similar to nodes;
 *	Style_Graph() was updated to accomodate the numbering, and
 *	some other variables were renamed to avoid confusion.
 * Aug 24, 2020 (IC V1.15)
 *  (a) New basicGraphs category circulant added; create_Basic_Graph() takes
 *      a new argument, offsets, and has a new case for generate_circulant().
 *  (b) For circulant graphs added the offsets param and related code
 *	to Create_Basic_Graph().
 * Aug 25, 2020 (IC V1.16)
 *  (a) Minor clean-up of yesterday's additions.
 * Oct 18, 2020 (JD V1.17)
 *  (a) Fix spurious error message when no graph is selected.
 *  (b) Fix spelling of "colour" throughout, where possible.
 */

#include "basicgraphs.h"
#include "defuns.h"
#include "edge.h"
#include "node.h"
#include "graph.h"
#include "graphmimedata.h"
#include "preview.h"

#include <math.h>
#include <QKeyEvent>
#include <QDrag>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>
#include <QMessageBox>
#include <QFileDialog>
#include <QShortcut>
#include <QtGui>
#include <qmath.h>


// This is the factor by which the preview pane is zoomed for each
// zoom in or zoom out operation.
#define SCALE_FACTOR    1.1
static qreal zoomValue = 100;
static QString zoomDisplayText = "Zoom: " + QString::number(zoomValue) + "%";

// Empirically chosen values, modify as you see fit:
#define MIN_ZOOM_LEVEL  0.07
#define MAX_ZOOM_LEVEL  10.0



/*
 * Name:        PreView
 * Purpose:     Constructor for the PreView class.
 * Arguments:   QWidget *
 * Outputs:     Nothing.
 * Modifies:    The graphics context.
 * Returns:     Nothing.
 * Assumptions: ?
 * Bugs:        ?
 * Notes:       None so far.
 */

PreView::PreView(QWidget * parent)
    : QGraphicsView(parent)
{
    PV_Scene = new QGraphicsScene();
    PV_Scene->setSceneRect(0, 0, this->width(), this->height());
    
    qDeb() << "PV::PV() just set the scene rectangle to 0, 0, "
	   << this->width() << ", " << this->height();

    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate); // Updates the canvas
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setScene(PV_Scene);
}



/*
 * Name:        keyPressEvent
 * Purpose:     Perform the appropriate action for known key presses.
 * Arguments:   QKeyEvent
 * Output:      Nothing.
 * Modifies:    The scale of the preview window for the zoom operations.
 * Returns:     Nothing.
 * Assumptions: ?
 * Bugs:        ?
 * Notes:       Unhandled key events are passed on to QGraphicsView.
 */

void
PreView::keyPressEvent(QKeyEvent * event)
{
    qDeb() << "PV:keyPressEvent(" << event->key() << ") called.";

    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
        switch (event->key())
        {
          case Qt::Key_Equal:
            zoomIn();
            break;
          case Qt::Key_Minus:
            zoomOut();
            break;
          default:
            QGraphicsView::keyPressEvent(event);
	}
    }
    else
        QGraphicsView::keyPressEvent(event);
}



/*
 * Name:        wheelEvent
 * Purpose:     Perform the appropriate action for wheel scroll.
 * Arguments:   A QWheelEvent.
 * Output:      Nothing.
 * Modifies:    The scale of the preview window for the zoom operations.
 * Returns:     Nothing.
 * Assumptions: ?
 * Bugs:        ?
 * Notes:       Unhandled key events are passed on to QGraphics View
 */

void
PreView::wheelEvent(QWheelEvent * event)
{
    qDeb() << "PV:wheelEvent(" << event->angleDelta() << ") called.";

    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
        if (event->angleDelta().y() > 0)
            zoomIn();
        else if (event->angleDelta().y() < 0)
            zoomOut();
        else
            QGraphicsView::wheelEvent(event);
    }
    else
        QGraphicsView::wheelEvent(event);
}



/*
 * Name:        scaleView()
 * Purpose:     Scales the view of the QGraphicsScene
 * Arguments:   A qreal
 * Output:      Nothing.
 * Modifies:    The scale view of the QGraphicsScene
 * Returns:     Nothing.
 * Assumptions: None.
 * Bugs:        ?
 * Notes:       None.
 */

void
PreView::scaleView(qreal scaleFactor)
{
    qDeb() << "PV::scaleView(" << scaleFactor << ") called";

    qreal factor = transform().scale(scaleFactor, scaleFactor)
                .mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < MIN_ZOOM_LEVEL || factor > MAX_ZOOM_LEVEL)
        return;

    scale(scaleFactor, scaleFactor);

    // Determine how displayed zoom value needs to update
    qreal afterFactor = transform().scale(scaleFactor, scaleFactor)
            .mapRect(QRectF(0, 0, 1, 1)).width();
    if (afterFactor > factor)
        zoomValue = zoomValue * SCALE_FACTOR;
    else
        zoomValue = zoomValue / SCALE_FACTOR;

    // Update and show the current zoom
    zoomDisplayText = "Zoom: " + QString::number(zoomValue, 'f', 0) + "%";
    emit zoomChanged(zoomDisplayText);
}



/*
 * Name:	mousePressEvent()
 * Purpose:	Handle the dragging of the preview graph to the main canvas.
 * Arguments:	A mouse event.
 * Outputs:	Nothing.
 * Modifies:	The canvas and the preview window.
 * Returns:	Nothing.
 * Assumptions:	?
 * Bugs:	?
 * Notes:	Passes the mouse press on to QGraphicsView() after it is done.
 */

void
PreView::mousePressEvent(QMouseEvent * event)
{
    qDeb() << "PV::mousePressEvent(" << event << ") called";

    if (event->button() == Qt::LeftButton)
    {
        QList<QGraphicsItem *> itemList = this->scene()->items(
	    this->mapToScene(event->pos()),
	    Qt::IntersectsItemShape,
	    Qt::DescendingOrder, QTransform());

        foreach (QGraphicsItem * item, itemList)
        {
	    qDeb() << "\tlooking at a graphics item of type " << item->type();
            if (item->type() == Graph::Type)
            {
                Graph * graph = qgraphicsitem_cast<Graph *>(item);
		GraphMimeData * data = new GraphMimeData(graph);
		QDrag * drag = new QDrag(this->scene());
		drag->setMimeData(data);

		QPixmap * image = new QPixmap(
		    graph->childrenBoundingRect().size().toSize());
		image->fill(Qt::white);
		QPainter painter(image);
		painter.setRenderHints(QPainter::Antialiasing |
				       QPainter::HighQualityAntialiasing |
				       QPainter::NonCosmeticDefaultPen,
				       true);
		scene()->render(&painter,
				QRectF(0, 0,
				       scene()->itemsBoundingRect().width(),
				       scene()->itemsBoundingRect().height()),
				scene()->itemsBoundingRect(),
				Qt::IgnoreAspectRatio);
		painter.end();

		drag->setPixmap(*image);
		drag->exec();
		break;
            }
        }
    }
    QGraphicsView::mousePressEvent(event);
}



/*
 * Name:        zoomIn()
 * Purpose:     Zoom in the preview pane.
 * Arguments:   None.
 * Output:      Nothing.
 * Modifies:    The scale of the preview QGraphicsScene.
 * Returns:     Nothing.
 * Assumptions: None.
 * Bugs:        None.
 * Notes:       None.
 */

void
PreView::zoomIn()
{
    scaleView(qreal(SCALE_FACTOR));
}



/*
 * Name:        zoomOut()
 * Purpose:     Zoom out the preview pane.
 * Arguments:   None.
 * Output:      Nothing.
 * Modifies:    The scale of the preview QGraphicsScene.
 * Returns:     Nothing.
 * Assumptions: None.
 * Bugs:        None.
 * Notes:       None.
 */

void
PreView::zoomOut()
{
    scaleView(1. / qreal(SCALE_FACTOR));
}



/*
 * Name:	Create_Basic_Graph
 * Purpose:	Create a "basic graph" and add it to the preview scene.
 * Arguments:	The graph type index, the node count(s), and a flag
 *		indicating whether edges should be added to the graph
 *		(otherwise just the nodes are drawn).
 * Outputs:	Nothing.
 * Modifies:	The preview scene, erasing any previous graph there.
 * Returns:	Nothing.
 * Assumptions:	?
 * Bugs:	?
 * Notes:	Creates a graph where the centers of the nodes occupy
 *		a 1" by 1" square (or a 1" line in the case of
 *		1-dimensional graphs such as paths).  However, the
 *		width and height stored in the graph struct have been
 *		converted into PIXELS.  When the graph is subsequently
 *		styled the current node diameter must be taken into
 *		account when positioning the nodes.
 */

void
PreView::Create_Basic_Graph(int graphType, int numOfNodes1, int numOfNodes2,
			    qreal nodeDiameter, bool drawEdges,
			    QString offsets)
{
    // This param is here for the day when some basic graph drawing
    // algorithm needs this.  Dutch Windmill comes to mind, as does prism.
    Q_UNUSED(nodeDiameter);

    qDeb() << "PV::Create_Basic_Graph(): the preview scene currently has "
	   << this->scene()->items().size() << " items";

    // We are making a new graph: away with the old one.
    this->scene()->clear();

    Graph * g = new Graph();
    BasicGraphs * basicG = new BasicGraphs();

    switch (graphType)
    {
      case 0:
	// We get this when the drop-down menu just shows the menu title.
	delete basicG;
	break;

      case BasicGraphs::Antiprism:
        basicG->generate_antiprism(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::BBTree:
        basicG->generate_balanced_binary_tree(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Bipartite:
        basicG->generate_bipartite(g, numOfNodes1, numOfNodes2, drawEdges);
        break;

      case BasicGraphs::Circulant:
        basicG->generate_circulant(g, numOfNodes1, offsets, drawEdges);
        break;

      case BasicGraphs::Complete:
        basicG->generate_complete(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Crown:
        basicG->generate_crown(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Cycle:
        basicG->generate_cycle(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Dutch_Windmill:
        basicG->generate_dutch_windmill(g, numOfNodes1, numOfNodes2, drawEdges);
        break;

      case BasicGraphs::Gear:
	basicG->generate_gear(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Grid:
        basicG->generate_grid(g, numOfNodes1, numOfNodes2, drawEdges);
        break;

      case BasicGraphs::Helm:
        basicG->generate_helm(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Path:
        basicG->generate_path(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Petersen:
        basicG->generate_petersen(g, numOfNodes1, numOfNodes2, drawEdges);
        break;

      case BasicGraphs::Prism:
        basicG->generate_prism(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Star:
        basicG->generate_star(g, numOfNodes1, drawEdges);
        break;

      case BasicGraphs::Wheel:
        basicG->generate_wheel(g, numOfNodes1, drawEdges);
        break;

      default:
        // This should never happen!  Do not change to qDeb().
	qDebug() << "PV::Create_Graph(): unknown/invalid graph index "
		 << graphType;
        break;
    }

    this->scene()->addItem(g);
}



/*
 * Name:	Style_Graph()
 * Purpose:	Given a previously-created graph, apply the given
 *		drawing parameters to get the desired drawing.
 * Arguments:	The graph object and all the drawing info.
 *		Note that the width and height arguments specify the
 *		bounding box of the graph drawing (in inches), so the
 *		node diameters must be taken into account when
 *		positioning node centers.
 * Outputs:	Nothing.
 * Modifies:	The drawing of the graph.
 * Returns:	Nothing.
 * Assumptions:	?
 * Bugs:	?
 * Notes:	Each node of the graph has previewX and previewY
 *		values which locate the node in a 1"*1" region,
 *		centered on (0, 0).  When styling the graph, we use
 *		this information, the width and height, and the DPI
 *		information to scale the node location accordingly.
 */

#define GUARD(x) if ((what_changed == ALL_WGT) || ((x) == what_changed))

void
PreView::Style_Graph(Graph * graph,		    int graphType,
		     enum widget_ID what_changed,   qreal nodeDiameter,
		     QString topNodeLabels,	    QString bottomNodeLabels,
		     bool nodeLabelsNumbered,	    qreal nodeLabelSize,
		     QColor nodeFillColour,	    QColor nodeOutlineColour,
		     qreal edgeSize,		    QString edgeLabel,
		     qreal edgeLabelSize,	    QColor edgeLineColour,
		     qreal totalWidth,		    qreal totalHeight,
		     qreal rotation,		    qreal nodeNumStart,
		     qreal nodeThickness,	    bool edgeLabelsNumbered,
		     qreal edgeNumStart)
{
    qDeb() << "PV::Style_Graph(wid:" << what_changed << ") called.";

    int i = nodeNumStart;
    int j = nodeNumStart;
    int k = edgeNumStart;

    // The w & h args are *total* w & h for the graph, but we need to
    // locate the center of the nodes.  So first calculate the
    // nodecenter-to-nodecenter dimensions, then calculate the scale
    // factors, and finally factor in the inch->screen mapping; this
    // will be used to set the position of the nodes.
    qreal centerWidth = totalWidth - nodeDiameter;
    if (centerWidth < 0.1)
	centerWidth = 0.1;
    qreal widthScaleFactor = centerWidth * currentPhysicalDPI_X;
    qreal centerHeight = totalHeight - nodeDiameter;
    if (centerHeight < 0.1)
	centerHeight = 0.1;
    qreal heightScaleFactor = centerHeight * currentPhysicalDPI_Y;

    qDeb() << "    Desired total width: " << totalWidth
	   << "; desired center width " << centerWidth
	   << "\n\twidthScaleFactor: " << widthScaleFactor;
    qDeb() << "    Desired total height: " << totalHeight
	   << "; desired center height " << centerHeight
	   << "\n\theightScaleFactor: " << heightScaleFactor;

    foreach (QGraphicsItem * item, graph->childItems())
    {
        if (item->type() == Node::Type)
        {
	    Node * node = qgraphicsitem_cast<Node *>(item);
	    node->setParentItem(nullptr);	    // ?? Eh?

	    node->physicalDotsPerInchX = currentPhysicalDPI_X;

	    GUARD(nodeThickness_WGT) node->setPenWidth(nodeThickness);
	    GUARD(nodeDiam_WGT) node->setDiameter(nodeDiameter);
	    GUARD(nodeFillColour_WGT) node->setFillColour(nodeFillColour);
	    GUARD(nodeOutlineColour_WGT) node->setLineColour(nodeOutlineColour);
	    GUARD(nodeLabelSize_WGT) node->setNodeLabelSize(nodeLabelSize);
	    node->setPos(node->getPreviewX() * widthScaleFactor,
			 node->getPreviewY() * heightScaleFactor);

	    if (what_changed == ALL_WGT
		|| what_changed == nodeLabel1_WGT
		|| what_changed == nodeLabel2_WGT
		|| what_changed == nodeNumLabelCheckBox_WGT
		|| what_changed == nodeNumLabelStart_WGT)
	    {
		// Clear the node label, in case it was set previously.
		node->setNodeLabel("");
		if (nodeLabelsNumbered)
		    node->setNodeLabel(i++);
		else if (graphType == BasicGraphs::Bipartite)
		{
		    // Special case for labeling bipartite graphs.
		    if (bottomNodeLabels.length() != 0
			&& graph->nodes.bipartite_bottom.contains(node))
			node->setNodeLabel(bottomNodeLabels, j++);
		    else if (topNodeLabels.length() != 0
			     && graph->nodes.bipartite_top.contains(node))
			node->setNodeLabel(topNodeLabels, i++);
		    else if (topNodeLabels.length() != 0
			     && graph->nodes.bipartite_bottom.contains(node))
			node->setNodeLabel(topNodeLabels, i++);
		}
		else if (topNodeLabels.length() != 0)
		    node->setNodeLabel(topNodeLabels, i++);
	    }

	    qDeb() << "    nodes[" << node->getLabel()
		   << "] coords: screen (" << node->x() << ", " << node->y()
		   << "); preview (" << node->getPreviewX()
		   << ", " << node->getPreviewY() << ")";
	    node->setParentItem(graph);
        }
        else if (item->type() == Edge::Type)
        {
	    Edge * edge = qgraphicsitem_cast<Edge *>(item);
	    edge->setParentItem(nullptr);	// ?? Eh?
	    GUARD(edgeThickness_WGT) edge->setPenWidth(edgeSize);
	    GUARD(edgeLineColour_WGT) edge->setColour(edgeLineColour);
	    GUARD(edgeLabelSize_WGT)
		edge->setEdgeLabelSize((edgeLabelSize > 0) ? edgeLabelSize : 1);
	    if (what_changed == ALL_WGT
		|| what_changed == edgeLabel_WGT
		|| what_changed == edgeNumLabelCheckBox_WGT
		|| what_changed == edgeNumLabelStart_WGT)
	    {
		// Clear the edge label, in case it was set previously.
		edge->setEdgeLabel("");
		if (edgeLabelsNumbered)
		    edge->setEdgeLabel(k++);
		else if (edgeLabel.length() != 0)
		    edge->setEdgeLabel(edgeLabel, k++);
	    }
	    GUARD(nodeDiam_WGT) edge->setDestRadius(nodeDiameter / 2.);
	    // Q: why did RB do this?  It gives a bizarre value.
	    // edge->setSourceRadius(edge->sourceNode()->getDiameter() / 2.);
	    GUARD(nodeDiam_WGT) edge->setSourceRadius(nodeDiameter / 2.);
	    edge->setParentItem(graph);
        }
    }
    qDeb() << "   graph currently located at " << graph->x() << ", "
	   << graph->y(); 
    graph->setPos(mapToScene(viewport()->rect().center()));
    qDeb() << "   graph NOW located at " << graph->x() << ", "
	   << graph->y(); 
    graph->setRotation(-1 * rotation, false);
}
