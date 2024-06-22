#include "graphmimedata.h"
#include "graph.h"
#include <QStringList>

GraphMimeData::GraphMimeData(Graph * aGraphItem)
{
    myGraphItem = aGraphItem;
    myFormats << "GraphItem";
}

QStringList GraphMimeData::formats() const
{
    return myFormats;
}

QVariant GraphMimeData::retrieveData(const QString &format, QVariant::Type preferredType) const
{
    QVariant v;
    if (format == "GraphItem")
        return preferredType;
    else
        return QMimeData::retrieveData(format, preferredType);
}
