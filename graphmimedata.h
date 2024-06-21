#ifndef GRAPHMIMEDATA_H
#define GRAPHMIMEDATA_H

#include "graph.h"

#include <QMimeData>
#include <QList>
#include <QStringList>


class GraphMimeData : public QMimeData
{
    Q_OBJECT

public:
    GraphMimeData(Graph * aGraphItem);
    Graph * graphItem() const {return myGraphItem;}
    QStringList formats() const;
protected:
    QVariant retrieveData(const QString &format,
                          QVariant::Type preferredType) const;
private:
    QStringList myFormats;
    mutable Graph * myGraphItem;
};

#endif // GRAPHMIMEDATA_H
