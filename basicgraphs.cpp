/*
 * File:	basicgraphs.cpp
 * Author:	Rachel Bood
 * Date:	Dec 31, 2015 (?)
 * Version:	1.9
 *
 * Purpose:	Implement functions which draw all the "known" graph types.
 *
 * Modification history:
 * Feb 5, 2016 (JD V1.1):
 *  (a) Fixed scaling of internal polygon of antiprisms so that it scales
 *      according to both height and width.
 *  (b) (drive-by) made formatting more consistent.
 * Dec 7, 2019 (JD V1.2):
 *  (a) Add improved version of DEBUG / qDeb() stuff.
 	Clean up some comments, formatting, ...
 *  (b) When creating a graph, rather than the former setting of the
 *	node (scene) position, use node->setPreviewCoords() to record
 *	where the node is positioned in a graph of "standard" size,
 *	as currently defined in preview.cpp.  This allows subsequent
 *	styling to scale it as desired.  Remove height and width from
 *	all of the pre-defined graph types.
 *  (c) Modify generate_antiprism() so that the internal cycle gets
 *	bigger as the number of nodes increases, for aesthetic appeal.
 *  (d) Replace "complete" (when used in the "draw edges" sense) with
 *	the identifier drawEdges.
 *  (e) Replace "item" with "g" when the item is a graph.
 *      Rename topNodes and bottomNodes to more suggestive names for
 *	graphs other than bipartite.
 *  (f) In a fit of obsessiveness re-order the generate_...() functions
 *	into alphabetical order.  Other sundry tidying.
 * Dec 10, 2019 (JD V1.3):
 *  (a) Add decl of Graph_Type_Name so that I can access getGraphName()
 *      from other classes.
 * Dec 10, 2019 (JD V1.4):
 *  (a) Having put the debug defns in defuns.h (included by basicgraphs.h),
 *      remove them from here.
 * Dec 14, 2019 (JD V1.5):
 *  (a) Modify create_cycle() so that the cycle fills up as much of
 *	the given rectangle as possible, while scaling X and Y equally.
 *  (b) Modify generate_antiprism() to create two separate cycles,
 *	since the former method of creating one cycle and then moving
 *	some vertices did not play well with the new create_cycle().
 *	(And, as a side benefit, not only are the sizes closer to the
 *	requested box, but the numbering of the vertices is, arguably,
 *	more pleasing.).
 *  (c) Modify generate_prism() so that the number of nodes parameter
 *      specifies the total nodes, not the nodes in the outer (and
 *	inner) cycle.  This makes it consistent with generate_antiprism().
 *	Also make the inner cycle shrink size a function of the number of
 *	nodes.
 *  (d) Modify generate_dutch_windmill() so that the windmill is as
 *	large as possible while still fitting inside the bounding box.
 *  (e) Modify generate_gear() so that the gear is as large as
 *	possible while still fitting inside the bounding box (as with
 *	all the above, while scaling X and Y equally).
 * Aug 24, 2020 (IC + JD V1.6):
 *  (a) Added a new basicGraphs category, circulant graph, which creates
 *      a cycle along with edges based on a list of node offsets.
 * Aug 25, 2020 (IC + JD V1.7):
 *  (a) Improve the code checking circulant graph offsets.
 * Aug 27, 2020 (JD V1.8)
 *  (a) Modified the function which creates circulant graphs to remove
 *	need for regexps and tweak the code a bit.
 *	Added function documentation.
 * Aug 28, 2020 (IC V1.9)
 *  (a) Remove spurious #include <QRegularExpression>.
 */

#include "basicgraphs.h"
#include "defuns.h"
#include "node.h"
#include "edge.h"
#include <qmath.h>
#include <QDebug>


static const double PI = 3.14159265358979323846264338327950288419717;

// TODO: Use golden ratio in graphs with two cycles
// TODO: make create_cycle() better honour height and width.  Some
//	 trigonometric thinking will be required.

QVector<QString> BasicGraphs::Graph_Type_Name;

BasicGraphs::BasicGraphs()
{
    // This must agree with the Graph_Type enum defined in basicgraphs.h.
    Graph_Type_Name = { "None", "Antiprism", "Balanced Binary Tree",
			"Bipartite", "Circulant", "Complete", "Crown",
			"Cycle", "Dutch Windmill", "Gear (generalized)",
			"Grid", "Helm", "Path", "Petersen (generalized)",
			"Prism", "Star", "Wheel"
    };
}



/*
 * Name:	create_cycle()
 * Purpose:	Create a list of nodes uniformly distributed on a
 *		width*height ellipse centered at (0,0).
 *		Make these children of the given graph.
 * Arguments:	A graph, the number of nodes, the height and width of
 *		the ellipse, and, optionally, an angle which specifies
 *		the location of the first node.
 * Outputs:	Nothing.
 * Modifies:	The graph's child list, and, indirectly, the graph's scene.
 * Returns:	The list of nodes.
 * Assumptions:	?
 * Bugs:	?
 * Notes:	Depending on the number of nodes, we can't necessarily
 *		fill the width*height ellipse.  However, maximize the
 *		cycle in that space.  I do a brute-force calculation,
 *		but probably some clever trigonometry would be better.
 */


QList<Node *>
BasicGraphs::create_cycle(Graph * g, qreal width, qreal height,
			  int numOfNodes, qreal radians)
{
    QList<Node *> nodes;
    qreal spacing = (2 * PI) / numOfNodes;

    qDebu("BG::create_cycle(w = %.3f, h = %.3f, n = %d, a = %.3f",
	      width, height, numOfNodes, radians);

    // Calculate the amount by which we should scale up w and h.
    qreal minX = 1E10, maxX = -1E10, minY = 1E10, maxY = -1E10;
    qreal rads = radians;
    for (int i = 0; i < numOfNodes; i++)
    {
        qreal x = width * qSin(rads);
        qreal y = height * -qCos(rads);
	qDebu("\ti = %d: a = %.3f; x = %.3f, y = %.3f", i, rads, x, y);
	if (x < minX)
	    minX = x;
	if (x > maxX)
	    maxX = x;
	if (y < minY)
	    minY = y;
	if (y > maxY)
	    maxY = y;
        rads += spacing;
    }

    // Find out which dimension allows the smallest scaling up.
    qreal xslop = 2 * width / (maxX - minX);
    qreal yslop = 2 * height / (maxY - minY);
    qreal minSlop = xslop < yslop ? xslop : yslop;
    qDebu("\tX [%.3f, %.3f] Y [%.3f, %.3f]",
	  minX, maxX, minY, maxY);
    qDebu("\txslop %.3f; yslop %.3f; minSlop %.3f", xslop, yslop, minSlop);
    width *= minSlop;
    height *= minSlop;
    qDebu("\tnew width %.3f, new height %.3f", width, height);

    for (int i = 0; i < numOfNodes; i++)
    {
        qreal x = width * qSin(radians);
        qreal y = height * -qCos(radians);

        Node * node = new Node();
	qDeb() << "\tputting a node at (" << x << ", " << y << ")";
        node->setPreviewCoords(x, y);
        nodes.append(node);
        node->setParentItem(g);
        radians += spacing;
    }

    return nodes;
}



/*
 * Name:	generate_antiprism()
 * Purpose:	Generate an antiprism graph.
 * Arguments:	The graph g, the number of nodes, and whether to draw edges.
 * Outputs:	Nothing.
 * Modifies:	g
 * Returns:	Nothing.
 * Assumptions:	numOfNodes is even and >= 6.  (If odd, it is "rounded" down.)
 * Bugs:	?
 * Notes:	Aesthetic positioning of the interior nodes depends on
 *		both the number of nodes and the size of the nodes.
 *		In the future it might make sense to add the node
 *		diameter to this function for better aesthetics.  The
 *		shrink factors below were empirically found to be
 *		reasonably pleasing for a node diameter of 0.20 inch.
 *		TODO: make this better.
 */

void
BasicGraphs::generate_antiprism(Graph * g, int numOfNodes, bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;
    int halfNumNodes = numOfNodes / 2;

    // These numbers arbitrarily picked while looking at 2.5" x 2.5"
    // graphs with 0.20" nodes.
    qreal shrink_factor = 4;
    if (numOfNodes > 32)
	shrink_factor = 1.4;
    else if (numOfNodes > 24)
	shrink_factor = 1.6;
    else if (numOfNodes > 16)
	shrink_factor = 2.;
    else if (numOfNodes > 8)
	shrink_factor = 2.5;

    qDebu("BG::generate_antiprism(%d) shrink_factor is %.2f",
	  numOfNodes, shrink_factor);

    // Create the two cycles.  A slight abuse of Rachel's plan for the
    // double_cycle (to avoid a bunch of modular arithmetic).
    qreal inner_rotation = 2 * PI / numOfNodes;
    g->nodes.cycle = create_cycle(g, width, height, halfNumNodes);
    g->nodes.double_cycle.append(create_cycle(g, width / shrink_factor,
					      height / shrink_factor,
					      halfNumNodes,
					      inner_rotation));

    for (int i = 0; i < halfNumNodes; i++)
    {
	if (drawEdges)
	{
	    Edge * edge1, * edge2, * edge3, * edge4;

	    // The edges in the outer cycle:
	    edge1 = new Edge(g->nodes.cycle.at(i),
			     g->nodes.cycle.at((i + 1) % halfNumNodes));
	    edge1->setParentItem(g);

	    // The edges in the inner cycle:
            edge2 = new Edge(g->nodes.double_cycle.at(0).at(i),
			     g->nodes.double_cycle.at(0).at(
				 (i + 1) % halfNumNodes));
            edge2->setParentItem(g);

	    // The edges connecting the inner and outer cycles:
	    edge3 = new Edge(g->nodes.cycle.at(i),
			     g->nodes.double_cycle.at(0).at(i));
            edge3->setParentItem(g);
	    edge4 = new Edge(g->nodes.cycle.at(i),
			     g->nodes.double_cycle.at(0).at(
				 (halfNumNodes + i - 1) % halfNumNodes));
            edge4->setParentItem(g);
	}
    }
}



/*
 * Name:	recursive_binary_tree()
 * Purpose:	Lay out the nodes of a binary tree (really a heap).
 * Arguments:	
 * Outputs:	Nothing.
 * Modifies:	g.
 * Returns:	Nothing.
 * Assumptions:	Args are meaningful.
 * Bugs:	
 * Notes:	The vertical spacing is evenly distributed from 0
 *		(root) to height (leaves).
 *		The horizontal spacing is defined as follows:
 *		- The leaves on a non-full tree are in the same
 *		  positions they would be if the bottom row was full.
 *		- The leaves of a full tree are evenly distributed
 *		  from 0 (left) to width (right).  This means the
 *		  inter-leaf spacing is width(N - 1) where N is the
 *		  number of leaves in the full tree.
 *		- Since the X coord of the parents of the leaves are
 *		  half way between two leaves, we actually use a
 *		  spacing value of width/2*(N-1).
 */

void
BasicGraphs::recursive_binary_tree(Graph * g, int depth, int index,
				   int treeDepth)
{
    if (index >= g->nodes.binaryHeap.size())
	return;

    int leftChildIndex = index * 2 + 1;
    int rightChildIndex = index * 2 + 2;
    qreal height = 1;
    qreal width = 1;
    qreal x;
    qreal y;

    if (treeDepth == 0)
	y = height / 2;
    else
	y = depth * height / treeDepth;

    if (depth == 0)
	x = width / 2;
    else
    {
	int spaceFactor = 1 << (treeDepth - depth + 1);
	int spaceOffset = 0;
	if (treeDepth != depth)
	    spaceOffset = (1 << (treeDepth - depth)) - 1;

	int startIndexThisLevel = (1 << depth) - 1;
	x = ((index - startIndexThisLevel) * spaceFactor + spaceOffset)
	    * (qreal)width / (2 * ((1 << treeDepth) - 1));
    }
    qDeb() << "Node" << index << "at depth" << depth
	   << "is at (" << x << "," << y << ") where treeDepth is"
	   << treeDepth;

    Node * node = new Node();
    node->setPreviewCoords(x - width / 2., y - height / 2.);
    g->nodes.binaryHeap[index] = node;
    node->setParentItem(g);

    if (leftChildIndex < g->nodes.binaryHeap.size())
        recursive_binary_tree(g, depth + 1, leftChildIndex, treeDepth);
    if (rightChildIndex < g->nodes.binaryHeap.size())
        recursive_binary_tree(g, depth + 1, rightChildIndex, treeDepth);
    return;
}



void
BasicGraphs::generate_balanced_binary_tree(Graph * g, int numOfNodes,
					   bool drawEdges)
{
    g->nodes.binaryHeap.resize(numOfNodes);
    qreal treeDepth = floor(log2(numOfNodes));
    recursive_binary_tree(g, 0, 0, treeDepth);

    if (! drawEdges)
	return;

    // TODO: replace this with a simpler i = n/2 to n, connect i to i/2
    for (int i = 0; i < g->nodes.binaryHeap.count() / 2; i++)
    {
	if (2 * i + 1 < g->nodes.binaryHeap.count())
	{
	    Edge * edge = new Edge(g->nodes.binaryHeap.at(i),
				   g->nodes.binaryHeap.at(2 * i + 1));
	    edge->setParentItem(g);
	}
	if (2 * i + 2 < g->nodes.binaryHeap.count())
	{
	    Edge * edge = new Edge(g->nodes.binaryHeap.at(i),
				   g->nodes.binaryHeap.at(2 * i + 2));
	    edge->setParentItem(g);
	}
    }
}



void
BasicGraphs::generate_bipartite(Graph * g, int topNodes, int bottomNodes,
				bool drawEdges)
{
    qreal height = 1;
    qreal width = 1;
    /*
     * Calculates the space between each of the nodes within
     * the user's desired width of the graph.
     */
    qreal topSpacing = 0;
    qreal bottomSpacing = 0;

    /*
     * Calculates the y coordinate of the bottom partition of the nodes.
     * Note: a QGraphicsScene coordinate system has the origin
     *       in the center of the scene.
     */

    qreal x;
    qreal y = height / -2.;

    if (topNodes == 1)
	x = 0;
    else
    {
	topSpacing= topNodes >= bottomNodes
	    ? width / (topNodes - 1) : width / topNodes;
	x = topNodes >= bottomNodes
	    ? width / -2. : width / -2. + topSpacing / 2.;
    }

    for (int i = 0; i < topNodes; i++)
    {
        Node * node = new Node();
        node->setPreviewCoords(x, y);
        g->nodes.bipartite_top.append(node);
        node->setParentItem(g);
        x += topSpacing;
    }

    if (bottomNodes == 1)
	x = 0;
    else
    {
	bottomSpacing = bottomNodes >= topNodes
	    ? width / (bottomNodes - 1)	: width / bottomNodes;
	x = topNodes <= bottomNodes
	    ? width / -2. : width / -2. + bottomSpacing / 2.;
    }
    y = height / 2.;
    for (int i = 0; i < bottomNodes; i++)
    {
        Node * node = new Node();
        node->setPreviewCoords(x, y);
        g->nodes.bipartite_bottom.append(node);
        node->setParentItem(g);
	x += bottomSpacing;
    }

    if (! drawEdges)
	return;

    for (int i = 0; i < topNodes; i++)
	for (int j = 0; j < bottomNodes; j++)
	{
	    Edge * edge = new Edge(g->nodes.bipartite_top.at(i),
				   g->nodes.bipartite_bottom.at(j));
	    edge->setParentItem(g);
	}
}



/*
 * Name:	generate_circulant()
 * Purpose:	Generate a circulant graph.
 * Arguments:	The graph g, the number of nodes, the list of edge offsets,
 *              and whether to draw edges.
 * Outputs:	Nothing.
 * Modifies:	g
 * Returns:	Nothing.
 * Assumptions:	None.
 * Bugs:	There must be a more elegant way to get the list of
 *		numbers out of the offsets string.  And note that
 *		stringstream is grotesque, not elegant.
 * Notes:	Any characters other than digits are used only as delimiters.
 *		Thus even if the UI allows negative numbers or
 *		arithmetic expressions to be entered, this function won't
 *		produce the intended (by the user) result.
 */

void
BasicGraphs::generate_circulant(Graph * g, int numOfNodes, QString offsets,
                                bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;
    QList<int> offsetsList;

    qDebu("BG:generate_circulant(, %d, '%s', %s) called",
	  numOfNodes, offsets.toLocal8Bit().data(),
	  drawEdges ? "true" : "false");

    // Need to parse the offsets string into a list of numbers
    int i = 0;
    while (i < offsets.length())
    {
	int num = 0;
	if (offsets.at(i).isDigit())
	{
	    while (i < offsets.length() && offsets.at(i).isDigit())
		num = 10 * num + offsets.at(i++).digitValue();
	    offsetsList.append(num);
	    qDebu("   added %d to offsetList", num);
	}
	i++;
    }

    g->nodes.cycle = create_cycle(g, width, height, numOfNodes);

    if (! drawEdges)
        return;

    for (int i = 0; i < numOfNodes; i++)
    {
	Node * firstNode = g->nodes.cycle.at(i);
        foreach (int num, offsetsList)
        {
            if (num > 0 && num < g->nodes.cycle.count())
            {
                // Prevent duplicate edges from being made.
                bool edgeExists = false;
		Node * secondNode = g->nodes.cycle.at((i + num) % numOfNodes);
                foreach (Edge * edge, firstNode->edges())
                {
                    if ((edge->destNode() == firstNode
			 && edge->sourceNode() == secondNode)
			|| (edge->sourceNode() == firstNode
			    && edge->destNode() == secondNode))
		    {
			edgeExists = true;
			break;
		    }		    
                }
                if (!edgeExists)
                {
                    Edge * edge = new Edge(firstNode, secondNode);
                    edge->setParentItem(g);
                }
            }
        }
    }
}



void
BasicGraphs::generate_complete(Graph * g, int numOfNodes, bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;

    g->nodes.cycle = create_cycle(g, width, height, numOfNodes);
    if (!drawEdges)
	return;

    for (int i = 0; i < g->nodes.cycle.count(); i++)
	for (int j = i + 1; j < g->nodes.cycle.count(); j++)
	{
	    Edge * edge = new Edge(g->nodes.cycle.at(i),
				   g->nodes.cycle.at(j));
	    edge->setParentItem(g);
	}
}



void
BasicGraphs::generate_crown(Graph * g, int numOfNodes, bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;

    g->nodes.double_cycle.append(create_cycle(g, width, height,
                                                 numOfNodes));
    g->nodes.double_cycle.append(create_cycle(g, width * 0.65,
						 height * 0.65, numOfNodes));
    if (! drawEdges)
	return;

    for (int i = 0; i < g->nodes.double_cycle.at(1).count(); i++)
    {
	Edge * edge = new Edge(g->nodes.double_cycle.at(0).at(i),
			       g->nodes.double_cycle.at(1).at(i));
	edge->setParentItem(g);
	edge = new Edge(g->nodes.double_cycle.at(1).at(i),
			g->nodes.double_cycle.at(1).at((i + 1)
				  % g->nodes.double_cycle.at(1).count()));
	edge->setParentItem(g);
    }
}



void
BasicGraphs::generate_cycle(Graph * g, int numOfNodes, bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;

    g->nodes.cycle = create_cycle(g, width, height, numOfNodes);

    if (! drawEdges)
	return;

    for (int i = 0; i < g->nodes.cycle.count(); i++)
    {
	Edge * edge = new Edge(g->nodes.cycle.at(i),
			       g->nodes.cycle.at((i + 1)
						 % g->nodes.cycle.count()));
	edge->setParentItem(g);
    }
}


/*
* Function:	generate_dutch_windmill()
* Paramters:	g: the graph
*		height: the desired height of the graph (note that
*		        many other functions get height / 2 as the
*			corresponding parameter)
*		blades: the number of blades in the windmill
*		bladeSize: the number of vertices in each blade
*		drawEdges: if True, draw the edges, else don't.
* Idea:	Give each blade and its adjacent air space an angle
*		of 2 * Pi / blades, regardless of how many vertices
*		per blade.
*		That angle is shared amongst the blade and the empty
*		space as follows: the blade is given
*		    0.9 - c * exp(-kn)
*		percent of the share; c = 0.786 and k = 0.135
*		were chosen to have 70% space in a 2-bladed windmill
*		and 50% space in a 5-bladed windmill.
* Assumption:	if # nodes is odd, create_cycle() creates the cycle
*		with a horizontal line at the bottom.
* TODO:		This needs a lot of improvements in most cases.
*		Make the edges from the center node to the adjacent
*		nodes a bit longer, at least for bladeSize = 5 and
*		maybe above.  Otherwise the nodes adjacent to the
*		center are too close, even when the node size is very
*		small.
*		Perhaps we want to take the node size into the
*		calculations when computing bladeWidth, but now that
*		I style nodes after drawing the graph, we can't do
*		that unless we do a "special" case here and look at
*		the size of the nodes when this function is called,
*		and if the user changes them later, they will have to
*		force a re-creation of the graph.
*/

void
BasicGraphs::generate_dutch_windmill(Graph * g, int blades, int bladeSize,
				     bool drawEdges)
{
    qreal height = 1;
    qreal cycleHeight, cycleWidth;
    qreal bladeSpacing = 2 * PI / blades;
    qreal bladeWidth = bladeSpacing * (0.9 - 0.786 * qExp(-0.135 * blades));

    qreal angle = 0;

    qDeb() << "BG:generate_dutch_windmill() called";
    qDeb() << "\theight is " << height << ", blades is " << blades
	   << ", bladeSize is " << bladeSize;
    qDeb() << "\tbladeSpacing is " << bladeSpacing << " radians";
    qDeb() << "\tbladeWidth is " << bladeWidth << " radians";
    qDeb() << "\tbladeWidth * #verts / (#v -2) / Pi = "
	   << bladeWidth * bladeSize / (bladeSize - 2) / PI;

    g->nodes.center = new Node();
    g->nodes.center->setPreviewCoords(0, 0);
    g->nodes.center->setParentItem(g);

    // cycleHeight is the vertical "radius", not "diameter"
    cycleHeight = height / 4;
    // To get the cycle "width", multiply the height by the ratio of
    //        the desired base angle (bladeWidth)
    //        to the corner angle in a regular bladeSize-gon.
    // Note that the angles of the corners of a bladeSize-gon
    // sum to (bladeSize - 2) * Pi.
    cycleWidth = cycleHeight * bladeWidth * bladeSize / ((bladeSize - 2) * PI);

    qDeb() << "cycleWidth will be " << cycleWidth;
    qDeb() << "cycleHeight will be " << cycleHeight;

    QList <QList <Node *>> list_of_cycles = g->nodes.list_of_cycles;

    for (int i = 0; i < blades; i++)
    {
	list_of_cycles.append(create_cycle(g, cycleWidth, cycleHeight,
					   bladeSize, PI));

	// Create a cycle with a single vertex at the bottom
	// and then remove that vertex.  It's neighbours will
	// eventually be joined to the g->node.center vertex.
	QList <Node *> i_cycle = list_of_cycles.at(i);
	i_cycle.at(0)->setParentItem(nullptr);
	i_cycle.removeFirst();

	for (int j = 0; j < i_cycle.count(); j++)
	{
	    // Translate the cycle up so that the center node is where
	    // the deleted vertex should have been:
	    Node * nodej = i_cycle.at(j);
	    //i_cycle.at(j)->setY(i_cycle.at(j)->getPreviewY() - cycleHeight);
	    nodej->setPreviewCoords(nodej->getPreviewX(),
				    nodej->getPreviewY() - cycleHeight);

	    // Now rotate the cycle around (0, 0):
	    qreal x = nodej->getPreviewX();
	    qreal y = nodej->getPreviewY();
	    nodej->setPreviewCoords(x * qCos(angle) - y * qSin(angle), 
				    x * qSin(angle) + y * qCos(angle));
	    nodej->setParentItem(g);

	    if (drawEdges && j != i_cycle.count() - 1)
	    {
		Edge * edge = new Edge(nodej,
				       i_cycle.at((j + 1) % i_cycle.count()));
		edge->setParentItem(g);
	    }
	}
	angle += bladeSpacing;

	if (drawEdges)
	{
	    Edge * edge = new Edge(i_cycle.at(0), g->nodes.center);
	    edge->setParentItem(g);
	    edge = new Edge(i_cycle.at(i_cycle.count() - 1),
			    g->nodes.center);
	    edge->setParentItem(g);
	}
    }

    // Now scale the coords so that the windmill fits in the bounding box
    // as closely as possible.
    qreal minX = 1E10, maxX = -1E10, minY = 1E10, maxY = -1E10;
    for (int i = 0; i < blades; i++)
    {
	QList <Node *> i_cycle = list_of_cycles.at(i);
	for (int j = 0; j < i_cycle.count(); j++)
	{
	    qreal x = i_cycle.at(j)->getPreviewX();
	    qreal y = i_cycle.at(j)->getPreviewY();
	    qDebu("  gdw: blade %d node %d (label %s): x = %.3f, y = %.3f",
		  i, j, i_cycle.at(j)->getLabel().toLatin1().data(), x, y);
	    if (x < minX)
		minX = x;
	    if (x > maxX)
		maxX = x;
	    if (y < minY)
		minY = y;
	    if (y > maxY)
		maxY = y;
	}
    }    

    // Find out which dimension allows the smallest scaling up.
    qreal xSlop = height / (maxX - minX);
    qreal ySlop = height / (maxY - minY);
    qreal minSlop = xSlop < ySlop ? xSlop : ySlop;
    qDebu("BG::generate_dutch_windmill(): X [%.3f, %.3f] Y [%.3f, %.3f]",
	  minX, maxX, minY, maxY);
    qDebu("\tminSlop %.3f", minSlop);

    // Now scale all the node preview locations:
    for (int i = 0; i < blades; i++)
    {
	QList <Node *> i_cycle = list_of_cycles.at(i);
	for (int j = 0; j < i_cycle.count(); j++)
	{
	    Node * n = i_cycle.at(j);
	    qreal x = n->getPreviewX();
	    qreal y = n->getPreviewY();
	    n->setPreviewCoords(x * minSlop, y * minSlop);
	}
    }
}



/*
 * Name:	generate_gear()
 * Purpose:	Create a generalized gear graph.
 * Arguments:	The graph g, the number of nodes, and whether to draw edges.
 * Outputs:	Nothing.
 * Modifies:	g.
 * Returns:	Nothing.
 * Assumptions:	
 * Bugs:	
 * Notes:	
 */
void
BasicGraphs::generate_gear(Graph * g, int numOfNodes, bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;
    int numCycleNodes = numOfNodes & ~1;

    qreal x1, y1, x2, y2;

    g->nodes.cycle = create_cycle(g, width, height, numCycleNodes);

    // Move the odd-numbered nodes in line with their adjacent nodes.
    for (int i = 0; i < numCycleNodes; i++)
    {
        if (i % 2 == 1)
        {
            x1 = g->nodes.cycle.at((i - 1) % numCycleNodes)->getPreviewX();
            y1 = g->nodes.cycle.at((i - 1) % numCycleNodes)->getPreviewY();
            x2 = g->nodes.cycle.at((i + 1) % numCycleNodes)->getPreviewX();
            y2 = g->nodes.cycle.at((i + 1) % numCycleNodes)->getPreviewY();
	    
            g->nodes.cycle.at(i)->setPreviewCoords((x1 + x2) / 2,
						   (y1 + y2) / 2);
        }
    }

    // Now scale the coords so that the gear fits in the bounding box
    // as closely as possible.
    qreal minX = 1E10, maxX = -1E10, minY = 1E10, maxY = -1E10;
    for (int i = 0; i < numCycleNodes; i++)
    {
	qreal x = g->nodes.cycle.at(i)->getPreviewX();
	qreal y = g->nodes.cycle.at(i)->getPreviewY();
	if (x < minX)
	    minX = x;
	if (x > maxX)
	    maxX = x;
	if (y < minY)
	    minY = y;
	if (y > maxY)
	    maxY = y;
    }

    // Find out which dimension allows the smallest scaling up.
    qreal xslop = 2 * width / (maxX - minX);
    qreal yslop = 2 * height / (maxY - minY);
    qreal minSlop = xslop < yslop ? xslop : yslop;
    qDebu("BG::generate_gear(): X [%.3f, %.3f] Y [%.3f, %.3f]",
	  minX, maxX, minY, maxY);
    qDebu("\txslop %.3f; yslop %.3f; minSlop %.3f", xslop, yslop, minSlop);

    // Now scale all the node preview locations:
    for (int i = 0; i < numCycleNodes; i++)
    {
	Node * n = g->nodes.cycle.at(i);
	qreal x = n->getPreviewX();
	qreal y = n->getPreviewY();
	n->setPreviewCoords(x * minSlop, y * minSlop);
    }

    // Add the center one, if it exists.
    Node * center = new Node();
    if (numOfNodes % 2 == 1)
    {
        center->setPreviewCoords(0, 0);
        center->setParentItem(g);
        g->nodes.center = center;
    }

    if (! drawEdges)
	return;

    for (int i = 0; i < g->nodes.cycle.count(); i++)
    {
	if (i % 2 == 0 && numOfNodes % 2 == 1)
	{
	    Edge * edge = new Edge(center, g->nodes.cycle.at(i));
	    edge->setParentItem(g);
	}
	Edge * edge = new Edge(g->nodes.cycle.at(i),
			       g->nodes.cycle.at((i + 1)
						 % g->nodes.cycle.count()));
	edge->setParentItem(g);
    }
}



void
BasicGraphs::generate_grid(Graph * g, int columns, int rows,
			   bool drawEdges)
{
    qreal width = 1;
    qreal height = 1;

    qreal xSpace = columns > 1 ? width / (columns - 1) : 1;
    qreal ySpace = rows > 1 ? height / (rows - 1) : 1;

    qreal  x = 0;
    qreal  y = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            Node * node = new Node();
            qreal xcoord, ycoord;

            xcoord = columns > 1 ? x - width / 2. : 0;
            ycoord = rows > 1 ? y - height / 2. : 0;

            node->setPreviewCoords(xcoord, ycoord);
	    node->setParentItem(g);
            g->nodes.grid.append(node);
            x += xSpace;
        }

        x = 0;
        y += ySpace;
    }

    if (! drawEdges)
	return;

    for (int i = 0; i < g->nodes.grid.count(); i++)
    {
	if ((i + 1) % columns != 0)
	{
	    Edge * edge = new Edge(g->nodes.grid.at(i),
				   g->nodes.grid.at(i + 1));
	    edge->setParentItem(g);
	}
	if (i + columns < g->nodes.grid.count())
	{
	    Edge * edge = new Edge(g->nodes.grid.at(i),
				   g->nodes.grid.at(i + columns));
	    edge->setParentItem(g);
	}
    }
}



void
BasicGraphs::generate_helm(Graph * g, int numOfNodes, bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;

    g->nodes.double_cycle.append(create_cycle(g, width, height,
						 numOfNodes));
    g->nodes.double_cycle.append(create_cycle(g, width * 0.65,
                                                 height * 0.65, numOfNodes));

    Node * node = new Node();
    node->setPreviewCoords(0,0);
    node->setParentItem(g);
    g->nodes.center = node;

    if (! drawEdges)
	return;

    for (int i = 0; i < g->nodes.double_cycle.at(0).count(); i++)
    {
	Edge * edge = new Edge(g->nodes.double_cycle.at(1).at(i),
			       g->nodes.center);
	edge->setParentItem(g);
	edge = new Edge(g->nodes.double_cycle.at(1).at(i),
			g->nodes.double_cycle.at(0).at(i));
	edge->setParentItem(g);
	edge = new Edge(g->nodes.double_cycle.at(1).at(i),
			g->nodes.double_cycle.at(1).at((i + 1)
				  % g->nodes.double_cycle.at(1).count()));
	edge->setParentItem(g);
    }
}



void
BasicGraphs::generate_path(Graph * g, int numOfNodes, bool drawEdges)
{
    if (numOfNodes < 1)
	return;

    if (numOfNodes == 1)
    {
        Node * node = new Node();
        node->setPreviewCoords(0,0);
        node->setParentItem(g);
	return;
    }

    qreal width = 1;
    qreal x = 0;
    qreal spacing = width / (numOfNodes - 1);
    for (int i = 0; i < numOfNodes; i++)
    {
	Node * node = new Node();
	node->setPreviewCoords(x - width / 2., 0);
	node->setParentItem(g);
	g->nodes.path.append(node);
	x += spacing;
    }

    if (! drawEdges)
	return;

    for (int i = 0; i < numOfNodes - 1; i++)
    {
	Edge * edge = new Edge(g->nodes.path.at(i),
			       g->nodes.path.at((i + 1)
						   % g->nodes.path.count()));
	edge->setParentItem(g);
    }

}



void
BasicGraphs::generate_petersen(Graph * g, int numOfNodes,
			       int starSkip, bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;

    g->nodes.double_cycle.append(create_cycle(g, width, height,
						 numOfNodes));
    g->nodes.double_cycle.append(create_cycle(g, width / 2, height / 2,
						 numOfNodes));
    if (! drawEdges)
	return;

    for (int i = 0; i < numOfNodes; i++)
    {
	Edge * edge = new Edge(g->nodes.double_cycle.at(0).at(i),
			       g->nodes.double_cycle.at(0).at((i + 1)
				 % g->nodes.double_cycle.at(0).count()));
	edge->setParentItem(g);

	if (starSkip % numOfNodes != 0)
	{
	    edge = new Edge(g->nodes.double_cycle.at(1).at(i),
			    g->nodes.double_cycle.at(1).at((i + starSkip)
							      % numOfNodes));
	    edge->setParentItem(g);
	}
	Edge * connectEdge = new Edge(g->nodes.double_cycle.at(0).at(i),
				      g->nodes.double_cycle.at(1).at(i));
	connectEdge->setParentItem(g);
    }
}


void
BasicGraphs::generate_prism(Graph * g, int numOfNodes, bool drawEdges)
{
    qreal width = 0.5;
    qreal height = 0.5;
    int halfNumNodes = numOfNodes / 2;

    // These numbers arbitrarily picked while looking at 2.5" x 2.5"
    // graphs with 0.20" nodes.
    qreal shrink_factor = 2.5;
    if (numOfNodes > 32)
	shrink_factor = 1.4;
    else if (numOfNodes > 24)
	shrink_factor = 1.6;
    else if (numOfNodes > 16)
	shrink_factor = 2.;
    else if (numOfNodes > 6)
	shrink_factor = 2.25;

    qDebu("BG::generate_antiprism(%d) shrink_factor is %.2f",
	  numOfNodes, shrink_factor);

    g->nodes.double_cycle.append(create_cycle(g, width, height,
					      halfNumNodes));
    g->nodes.double_cycle.append(create_cycle(g, width / shrink_factor,
					      height / shrink_factor,
					      halfNumNodes));

    if (! drawEdges)
	return;

    for (int i = 0; i < g->nodes.double_cycle.at(0).count(); i++)
    {
	Edge * edge = new Edge(g->nodes.double_cycle.at(0).at(i),
			       g->nodes.double_cycle.at(0).at((i + 1)
				    % g->nodes.double_cycle.at(0).count()));
	edge->setParentItem(g);

	edge = new Edge(g->nodes.double_cycle.at(0).at(i),
			g->nodes.double_cycle.at(1).at(i));
	edge->setParentItem(g);
    }
    for (int i = 0; i < g->nodes.double_cycle.at(1).count(); i++)
    {
	Edge * edge = new Edge(g->nodes.double_cycle.at(1).at(i),
			       g->nodes.double_cycle.at(1).at((i + 1)
				    % g->nodes.double_cycle.at(1).count()));
	edge->setParentItem(g);
    }
}



void
BasicGraphs::generate_star(Graph * g, int numOfNodes, bool drawEdges)
{
    if (numOfNodes <= 0)
	return;

    qreal width = 0.5;
    qreal height = 0.5;

    g->nodes.cycle = create_cycle(g, width, height, numOfNodes - 1);
    Node * node = new Node();
    node->setPreviewCoords(0, 0);
    g->nodes.center = node;
    node->setParentItem(g);

    if (! drawEdges)
	return;

    for (int i = 0; i < g->nodes.cycle.count(); i++)
    {
	Edge * edge = new Edge(node, g->nodes.cycle.at(i));
	edge->setParentItem(g);
    }
}



/*
 * Function:	generate_wheel()
 * Paramters:	g: the graph
 *		height: the desired height of the graph (note that
 *		        many other functions get height / 2 as the
 *			corresponding parameter)
 *		width:the desired width of the graph
 *		numOfNodes: the number of vertices in the graph
 *		drawEdges: if True, draw the edges
 * Output:	Nothing.
 * Modifies:	The Graph g.
 * Returns:	Nothing.
 * Assumptions:	?
 * Bugs:	?
 * Notes:	None.
 */

void
BasicGraphs::generate_wheel(Graph * g, int numOfNodes, bool drawEdges)
{
    if (numOfNodes <= 0)
	return;

    qreal width = 0.5;
    qreal height = 0.5;

    g->nodes.cycle = create_cycle(g, width, height, numOfNodes - 1);
    Node * node = new Node();
    node->setPreviewCoords(0, 0);
    g->nodes.center = node;
    node->setParentItem(g);

    if (! drawEdges)
	return;

    for (int i = 0; i < g->nodes.cycle.count(); i++)
    {
	Edge * edge = new Edge(node, g->nodes.cycle.at(i));
	edge->setParentItem(g);
	edge = new Edge(g->nodes.cycle.at(i),
			g->nodes.cycle.at((i + 1)
					     % g->nodes.cycle.count()));
	edge->setParentItem(g);
    }
}



QString
BasicGraphs::getGraphName(int enumValue)
{
    if (enumValue > 0 && enumValue < Count)
	return Graph_Type_Name[enumValue];

    return "NOT A BASIC GRAPH";
}
