/*
 * File:	html-label.cpp	    Formerly label.cpp
 * Author:	Rachel Bood
 * Date:	2014-??-??
 * Version:	1.10
 * 
 * Purpose:	Implement the functions relating to node and edge labels.
 *		(Some places in the code use "weight" for "edge label".)
 *
 * Modification history:
 * Nov 13, 2019 (JD V1.1)
 *  (a) Rationalize the naming to clarify that this is the HTML
 *	version of a (node or edge) label which will be displayed on
 *	the canvas.  That is,
 *	Label -> HTML_Label , label.{h, cpp} -> html-label.{h, cpp}
 *	and labelText -> htmlLabelText .
 *  (b) Move (and improve) code to HTML-ize node labels, which may
 *	contain super- and sub-scripts, into this file (from node.cpp).
 *  (c) Various and sundry minor formatting and comment additions.
 * Nov 13, 2019 (JD V1.2)
 *   - in setHtmlLabel() (currently only called for edges) replace the
 *     fontification code with strToHtml() so that edge labels can have
 *     super- and subscripts as well.  Add function comment to setHtmlLabel().
 * Nov 18, 2019 (JD V1.3)
 *  (a) Set default font size for labels, since up until now vertex labels
 *      get font size 9 unless they had labels set during the graph creation.
 *  (b) Remove spurious font variable in HTML_Label().
 *  (c) Fix some comments.
 * Nov 30, 2019 (JD V1.4)
 *  (a) Add qDeb() / #ifdef DEBUG jazz and a few debug outputs.
 * Jul 9, 2020 (IC V1.5)
 *  (a) Change the Z value of the HTML label to 5.  (JD Q: why?)
 * Jul 29, 2020 (IC V1.6)
 *  (a) Added eventFilter() to receive canvas events so we can identify
 *      the node/edge being edited/looked at in the edit tab list.
 * Aug 18, 2020 (JD V1.7)
 *  (a) Changed strToHtml() so that a label it can't parse is now
 *	returned as a raw string set in cmtt10 (so the user can see
 *      it), rather than returning an empty string.
 * Aug 19, 2020 (IC V1.8)
 *  (a) Changed the default label font from cmmi10 to cmtt10.  This
 *	causes the labels to be displayed in cmtt10 while the label is
 *	being edited.
 *  (b) Updated eventFilter to display the label text in TeX formatting while
 *      editing on the canvas and to signal the node/edge when when the label
 *      loses keyboard focus, either by clicking somewhere else on the canvas
 *      or pressing escape/enter.
 * Aug 20, 2020 (JD, V1.9)
 *  (a) *Major* rework of the code to HTML-ize the labels.  This code
 *	handles far more complex labels, although
 *	(i) it certainly doesn't handle many TeX math constructs, and
 *	(ii) the HTML renderer in Qt (at least to 5.15.0) does not
 *	     (seem to) allow either subscripts and superscripts to be
 *	     vertically stacked, or sub-sub/sup-sup scripts.  The
 *	     latter is supported by HTML4, the former maybe not.
 *  (b) Remove the function "setTextInteraction()" which has not been
 *	called in many a year, if ever.
 *  (c) #include defuns.h to get debugging defns, remove them from here.
 * Sep 9, 2020 (IC V1.9)
 *  (a) Added some precautions to the event filter incase a label gets focus
 *      without the edit tab being populated yet.
 * Sep 11, 2020 (JD V1.10)
 *  (a) Created a single font (cmzsd10) containing all the characters
 *      that are supported now in a valid math expression.
 *	(Appropriate chars were pulled from cmr10, cmsy7, cmsy10 and
 *	cmmi10.)  Simplified the code considerably since we no longer
 *	have to figure out what font to use, and so on.  Also made the
 *	code handle more weird but (more or less) valid constructs,
 *	such as "a^\{", "a\_", and many more such pathological expressions.
 *  (b) Remove setHtmlLabel(), which was made redundant by the changes
 *	to edge.cpp on Aug 21, 2020.
 */

#include "defuns.h"
#include "html-label.h"

#include <QTextCursor>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QInputMethodEvent>



HTML_Label::HTML_Label(QGraphicsItem * parent)
{
    qDeb() << "HTML_Label constructor called";

    this->setParentItem(parent);
    texLabelText = "";
    setZValue(5);

    QFont font;
    // Set the default font of HTML labels to cmtt10 so that the text
    // stands out while the label is being edited *and* so that when
    // the TeX of focused-out label is invalid, the label shows up on
    // the canvas in cmtt10.
    font.setFamily(QStringLiteral("cmtt10"));
    font.setBold(false);
    font.setWeight(50);
    font.setPointSize(12);
    this->setFont(font);
    setTextInteractionFlags(Qt::TextEditorInteraction);

    if (parentItem() != nullptr)
        setPos(parentItem()->boundingRect().center().x()
	       - boundingRect().width() / 2.,
               parentItem()->boundingRect().center().y()
	       - boundingRect().height() / 2.);

    editTabLabel = nullptr;
    installEventFilter(this);
}



/*
 * Name:        eventFilter()
 * Purpose:     Intercepts events related to canvas labels so we can
 *              identify the location of the item on the edit tab and
 *              transfer text data between the nodes/edges and the labels.
 * Arguments:	An object and the event of possible interest.
 * Output:	Nothing.
 * Modifies:	The "editability" of the label.
 * Returns:	Whatever QObject::eventFilter(obj, event) returns.
 * Assumptions:
 * Bugs:
 * Notes:       Try using QEvent::HoverEnter and QEvent::HoverLeave
 */

bool
HTML_Label::eventFilter(QObject * obj, QEvent * event)
{
    qDeb() << "HL:eventfilter() called with texLabelText = '"
	   <<  texLabelText << "' and obj = "
	   << obj << " and event = " << event;
    if (event->type() == QEvent::FocusIn)
    {
        // Embolden the header for this item's entry in the edit tab.
        if (editTabLabel != nullptr)
        {
            QFont font = editTabLabel->font();
            font.setBold(true);
            editTabLabel->setFont(font);
        }

        // While we are editing the label, display it in cmtt10 to
	// make it obvious to the user that it is being edited.
        QString text = "<font face=\"cmtt10\">" + texLabelText + "</font>";
        setHtml(text);
    }
    else if (event->type() == QEvent::FocusOut)
    {
        if (editTabLabel != nullptr)
        {
            // Undo the bold text
            QFont font = editTabLabel->font();
            font.setBold(false);
            editTabLabel->setFont(font);
        }

        // Let the parent know to update and reformat the label text.
        emit editDone(toPlainText());
    }
    else if (event->type() == QEvent::KeyPress)
    {
        // Trigger the focus out if any of these keys are pressed.
        QKeyEvent * keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape)
            clearFocus();
        else if (keyEvent->key() == Qt::Key_Enter
		 || keyEvent->key() == Qt::Key_Return)
        {
            clearFocus();
	    // Returning true prevents the event from being further processed.
	    // Without this the "Return" would go into the label field.
	    return true;
        }
    }

    return QObject::eventFilter(obj, event);
}



/*
 * Name:	paint()
 * Purpose:	Draw the label on the preview or main canvas.
 * Arguments:	As below.
 * Outputs:	Text to the preview or main canvas.
 * Modifies:	The preview or main canvas.
 * Returns:	Nothing.
 * Assumptions:	?
 * Bugs:	?
 * Notes:	This function is called when a label is changed,
 *		when a label edit widget in "Edit Nodes and Edges"
 *		gets or loses focus, and about once per second when
 *		the label on the canvas is being edited.
 *		And maybe some other times.
 */

void
HTML_Label::paint(QPainter * painter,
		  const QStyleOptionGraphicsItem * option,
		  QWidget * widget)
{
    QGraphicsTextItem::paint(painter, option, widget);
}



// All of the following code is for outputting labels in a TeX-ish way.
// HTML4 and Qt can't handle all of TeX math, but the code below makes
// relatively simple things look realistic.
// I allow some usage (e.g., \^ and \_) which are not the same as TeX,
// and this needs to be taken care of when exporting to TikZ.

/*
 * Name:	mathFontify()
 * Purpose:	Take a (non-HTMLized) string and create a new string
 *		with appropriate font tags to display the string on
 *		the Qt canvas in "math mode".
 *		THIS DOES NOT HANDLE subscript (_) or superscript (^).
 *		THAT IS DONE ELSEWHERE.
 *		However, prime (') is handled here.
 * Arguments:	A QString to be fontified.
 * Outputs:	Nothing.
 * Modifies:	Nothing.
 * Returns:	A QString with the fontified version of the original.
 * Assumptions:	
 * Bugs:	Does not work for non-"Local8Bit" chars in the string.
 * Notes:	When TeX displays a math formula it uses cmmi, but
 *		uses the cmr for digits (cmmi contains "old style"
 *		digits).  Unfortunately Qt's HTML display doesn't
 *		duplicate this, so this function handraulically
 *		fontifies each char of the string.
 *		This function is a crude approximation to what TeX
 *		does, in that it only implements basic math formula
 *		typesetting.  One thing it handles is throwing away
 *		braces which are not preceded by a '\'.
 */

static QString
mathFontify(QByteArray chars)
{
    QString htmlMathStr = "";
    bool prevWasBS = false;		// BackSlash
    int len = chars.length();

    qDebu("HL:mathFontify(\"%s\") called", chars.data());

    if (len == 0)
	return htmlMathStr;

    for (int i = 0; i < len; i++)
    {
	char c = chars[i];
	if (c == '\\' && ! prevWasBS)
	{
	    prevWasBS = true;
	    continue;
	}

	// TeX doesn't display braces if they are not preceded with '\'
	// Ditto for spaces in math formulae.
	if (c == '{' || c == '}' || c == ' ')
	{
	    if (! prevWasBS)
		continue;
	}

	// Map '<' and '>' to avoid conflicts with HTML tags.
	// NOTE: Although cmzsd10 has the space glyph from cmr10, and
	// other programs (e.g., gnumeric) show a visible space when
	// using cmzsd10, Qt does not display any visible space unless
	// the font is changed to cmr10.  (I could probably also use
	// cmtt10's space, but it is 50 or 60% wider than cmr10's,
	// so I'll use cmr10 for now.
	// TODO: figure out why cmzsd10's space isn't OK here.
	if (c == '<')
	    htmlMathStr += "&lt;";
	else if (c == '>')
	    htmlMathStr += "&gt;";
	else if (c == ' ' && prevWasBS)
	    htmlMathStr += "<font face = \"cmr10\">&nbsp;</font>";
	    // htmlMathStr += "&nbsp;";
	else if (c == '^' && prevWasBS)
	    htmlMathStr += "^";
	else if (c == '_' && prevWasBS)
	    htmlMathStr += "_";
	else if (c == '\'')
	    htmlMathStr += "<sup>'</sup>";
	else
	    htmlMathStr += c;

	prevWasBS = false;
    }

    qDeb() << "mathFontify(" << chars << ") -> /" << htmlMathStr << "/";

    return htmlMathStr;
}



/*
 * Name:	strToHtml2()
 * Purpose:	Parse the arg string, turn it into HTML, return that text.
 * Arguments:	A hopefully-correct TeX-ish node or edge label string.
 * Outputs:	Nothing.
 * Modifies:	Nothing.
 * Returns:	If able to completely parse the arg string as proper TeX, 
 *		the HTML-ized text.  On failure, ideally return the
 *		empty string.
 * Assumptions:	The label string is not deviously invalid.
 * Bugs:	(Arguable.)  Should return more useful information
 *		should parsing fail.
 * Notes:	Algorithm:
 *		Case 1: no '^' or '_'
 *		        -> just call mathFontify()
 *		Case 2: '^' or '_' at position 0 (i.e, a recursive call)
 *		     2a: sub/sup is a single token
 *		        -> handle base directly, recursive call for rest
 *		     2b: sub/sub is a brace group
 *		        -> find & handle base directly, recursive call for rest
 *		Case 3: first '^' or '_' not at position 0, at depth 0
 *		        -> find text before ^/_, recursive call
 *			-> recursive call on rest (including the ^/_)
 *		Case 4: first '^' or '_' not at position 0, not at depth 0
 *			-> mathfontify() any text before first '{'
 *			-> find braced text, call strToHtml2() on
 *			   text inside braces
 *			-> call strToHtml2() on remaining text
 */

static QString
strToHtml2(QByteArray chars)
{
    QByteArray prefix, rest;
    int firstUnderscore = chars.indexOf('_');
    int firstCircumflex = chars.indexOf('^');
    int length = chars.length();
    int first, depth, end;
    QString result = "";
    QString closeTag;

    qDebu("HL:strToHtml2(%s) called", chars.data());

    while (firstUnderscore > 0 && chars[firstUnderscore - 1] == '\\')
    {
	qDebu("  firstUnderscore() = %d, but \\ before it", firstUnderscore);
	firstUnderscore = chars.indexOf('_', firstUnderscore + 1);
    }
    while (firstCircumflex > 0 && chars[firstCircumflex - 1] == '\\')
	firstCircumflex = chars.indexOf('_', firstCircumflex + 1);

    qDebu("  firstUnderscore() = %d", firstUnderscore);
    qDebu("  firstCircumflex() = %d", firstCircumflex);
    
    // Case 1 (trivial): no superscript or subscript:
    if (firstUnderscore == -1 && firstCircumflex == -1)
    {
	qDeb() <<"  strToHtml2(): trivial case, returning mathFontify()";
	return mathFontify(chars);
    }

    if (firstUnderscore == -1)
	first = firstCircumflex;
    else if (firstCircumflex == -1)
	first = firstUnderscore;
    else
	first = firstUnderscore < firstCircumflex
	    ? firstUnderscore : firstCircumflex;
    qDebu("  .. first = %d", first);

    // Case 2: If first == 0, this means that the text before the ^/_
    // has already been dealt with, and then strToHtml2() was called
    // recursively to deal with the rest of the string.  We must find
    // the sub/sup, HTMLize that, and then deal with the rest of the string.
    if (first == 0)
    {
	if (chars[1] != '{')
	{
	    // Case 2a: sub/sup is just a single token.
	    // FOR NOW JUST HANDLE A SINGLE CHAR HERE (as opposed to a
	    // control sequence).
	    int chrs = 1;
	    if (chars[1] == '\\')
	    {
		if (length == 2)
		    return "";
		chrs = 2;
	    }
	    qDebu("   Case 2a: script is '%s'", chars.mid(1, chrs).data());
	    qDebu("   Case 2a: rest is '%s'", chars.mid(chrs + 1).data());
	    if (chars[0] == '^')
		result += "<sup>" + mathFontify(chars.mid(1, chrs)) + "</sup>";
	    else
		result += "<sub>" + mathFontify(chars.mid(1, chrs)) + "</sub>";
	    result += strToHtml2(chars.mid(chrs + 1));
	    return result;
	}

	// Case 2b: Find the sub/sup section in braces, strip the
	// braces and send it off for processing.
	qDebu("   Case 2b:");
	depth = 1;
	for (end = 2; end < length && depth > 0; end++)
	{
	    qDebu("  -- looking at '%c' where end = %d", (char)chars[end], end);
	    if (chars[end] == '{' && (end == 0 || chars[end - 1] != '\\'))
		depth++;
	    else if (chars[end] == '}' && (end == 0 || chars[end - 1] != '\\'))
		depth--;
	}
	end--;
	qDebu(" .. case 2b: end is %d, brace section is '%s'",
	      end, chars.mid(1, end).data());
	if (chars[0] == '^')
	    result += "<sup>" + strToHtml2(chars.mid(2, end - 2)) + "</sup>";
	else
	    result += "<sub>" + strToHtml2(chars.mid(2, end - 2)) + "</sub>";

	qDebu("  result so far is '%s'", result.toLocal8Bit().data());
	qDebu(" -.-. rest is '%s'", chars.mid(end + 1).data());
	if (end < length - 1)
	    result += strToHtml2(chars.mid(end + 1));

	return result;
    }

    // If we get here, there is at least one '^' or '_', but the first
    // one is not at the very beginning of the string.
    // See whether the first ^/_ is at depth 0 or it is not.
    depth = 0;
    for (int i = 0; i < first; i++)
    {
	if (chars[i] == '{' && (i == 0 || chars[i - 1] != '\\'))
	    depth++;
	else if (chars[i] == '}' && (i == 0 || chars[i - 1] != '\\'))
	    depth--;
    }
    qDebu("  depth of first ^/_ is %d", depth);

    if (depth == 0)
    {
	// Case 3: look for any text before first ^/_, deal with it.
	//         Then do a recursive call on the rest.
	qDebu("  Case 3: first ^/_ at depth 0");
	qDebu("     text before ^/_ is '%s'", chars.left(first).data());
	result += strToHtml2(chars.left(first));
	result += strToHtml2(chars.mid(first));
	return result;
    }

    // Case 4: break the string into three parts:
    //	    (a) possibly-empty prefix before first '{'
    //	    (b) the balanced {...} text
    //	    (c) possibly-empty suffix following the balanced {...} text
    qDebu("  Case 4:  first ^/_ NOT at depth 0");
    int firstBrace = 0;
    while (chars[firstBrace] != '{'
	   || (firstBrace != 0 && chars[firstBrace - 1] == '\\'))
	firstBrace++;
    qDebu("  ... firstBrace = %d", firstBrace);

    // (a) prefix
    if (firstBrace != 0)
	result += strToHtml2(chars.left(firstBrace));

    // (b) the balanced { ... } text
    depth = 1;
    for (end = firstBrace + 1; end < length && depth > 0; end++)
    {
	if (chars[end] == '{' && (end == 0 || chars[end - 1] != '\\'))
	    depth++;
	else if (chars[end] == '}' && (end == 0 || chars[end - 1] != '\\'))
	    depth--;
    }
    end--;
    qDebu(" .. case 4: end is %d, brace section is '%s'",
	  end, chars.mid(firstBrace + 1, end - firstBrace - 1).data());
    result += strToHtml2(chars.mid(firstBrace + 1, end - firstBrace - 1));

    // (c) The remaining text, if any.
    if (end < length - 1)
	result += strToHtml2(chars.mid(end + 1));

    return result;
}



/*
 * Name:	strToHtml()
 * Purpose:	Parse the arg string, turn it into HTML, return that text.
 * Arguments:	A hopefully-correct TeX-ish label string.
 * Outputs:	Nothing.
 * Modifies:	Nothing.
 * Returns:	If able to completely parse the arg string as proper TeX, 
 *		it returns the HTML-ized text.  On failure, the
 *		literal characters are set in cmtt10.
 * Assumptions:	The label string is not deviously invalid.
 * Bugs:	?
 * Notes:	Qt5 doesn't properly display 2nd-level sub/sups.
 *		This function boldly uses the dreaded and feared goto!
 *		The "prev" variable below holds the syntactic value of
 *		the previous char, not necessarily the actual char.
 */

QString
HTML_Label::strToHtml(QString str)
{
    QByteArray chars = str.toLocal8Bit();
    QString html;
    int length = chars.length();
    bool bogusSubSup = false;
    int depth = 0;
    char prev = 'o';	// Effective prev char, not actual ('o' = other)

    if (length == 0)
	return "";

    qDebu("\nHL:strToHtml(%s) called", chars.data());

    // Do some basic sanity checking
    if (chars[0] == '}' || chars[0] == '^' || chars[0] == '_')
	goto BOGUS;

    if (chars[0] == '{')
	depth = 1;

    if (chars[0] == '\\')
	prev = '\\';

    for (int i = 1; i < length && depth >= 0 && !bogusSubSup; i++)
    {
	switch (chars[i])
	{
	  case '{':
	    if (prev != '\\')
		prev = '{', depth++;
	    else
		prev = 'o';
	    break;

	  case '}':
	    if (prev != '\\')
		prev = '}', depth--;
	    else
		prev = 'o';
	    break;

	  case '\\':
	    if (prev != '\\')
		prev = '\\';
	    else
		prev = 'o';
	    break;

	  case '^':
	  case '_':
	    if (prev == '{')
		goto BOGUS;
	    if (i + 1 < length && chars[i + 1] == '}')
		goto BOGUS;
	    if (prev == '\\')
		prev = 'o';
	    else
		prev = '_';	// '_' represents the syntax type '^' or '_'.
	    break;

	  default:
	    prev = 'o';
	    break;
	}
    }
    if (depth != 0 || prev == '\\' || prev == '_')
	goto BOGUS;

    // The string wasn't empty, and not obviously bogus.
    // If strToHtml2() returns an empty string, assume that Something Is Wrong.
    html = strToHtml2(chars);
    if (html.length() == 0)
	goto BOGUS;

    // Success!
    html = "<font face=\"cmzsd10\">" + html + "</font>";
    qDebu("  strToHtml() returns \"%s\"", html.toLocal8Bit().data());
    return html;

  BOGUS:
    qDebu("  HL:strToHtml(): the label is invalid\n");
    // The default font for labels is cmtt10, so it will continue to
    // stand out without further htmlizing the text.
    return str;
}
