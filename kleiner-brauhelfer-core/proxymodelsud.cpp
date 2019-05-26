#include "proxymodelsud.h"
#include "brauhelfer.h"
#include "modelsud.h"

ProxyModelSud::ProxyModelSud(QObject *parent) :
    ProxyModel(parent),
    mColumnId(-1),
    mColumnStatus(-1),
    mColumnMerklistenID(-1),
    mFilterMerkliste(false),
    mFilterStatus(Alle),
    mFilterText(QString())
{
    connect(this, SIGNAL(sourceModelChanged()), this, SLOT(onSourceModelChanged()));
}

void ProxyModelSud::onSourceModelChanged()
{
    if(SqlTableModel* model = dynamic_cast<SqlTableModel*>(sourceModel()))
    {
        mColumnId = model->fieldIndex("ID");
        mColumnSudname = model->fieldIndex("Sudname");
        mColumnKommentar = model->fieldIndex("Kommentar");
        mColumnStatus = model->fieldIndex("Status");
        mColumnMerklistenID = model->fieldIndex("MerklistenID");
        setFilterDateColumn(model->fieldIndex("Braudatum"));
    }
    else if(ProxyModel* model = dynamic_cast<ProxyModel*>(sourceModel()))
    {
        mColumnId = model->fieldIndex("ID");
        mColumnSudname = model->fieldIndex("Sudname");
        mColumnKommentar = model->fieldIndex("Kommentar");
        mColumnStatus = model->fieldIndex("Status");
        mColumnMerklistenID = model->fieldIndex("MerklistenID");
        setFilterDateColumn(model->fieldIndex("Braudatum"));
    }
}

bool ProxyModelSud::filterMerkliste() const
{
    return mFilterMerkliste;
}

void ProxyModelSud::setFilterMerkliste(bool value)
{
    mFilterMerkliste = value;
    invalidate();
}

ProxyModelSud::FilterStatus ProxyModelSud::filterStatus() const
{
    return mFilterStatus;
}

void ProxyModelSud::setFilterStatus(FilterStatus state)
{
    mFilterStatus = state;
    invalidate();
}

QString ProxyModelSud::filterText() const
{
    return mFilterText;
}

void ProxyModelSud::setFilterText(const QString& text)
{
    mFilterText = text;
    invalidate();
}

bool ProxyModelSud::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index2;
    bool accept = ProxyModel::filterAcceptsRow(source_row, source_parent);
    if (accept && mFilterMerkliste)
    {
        index2 = sourceModel()->index(source_row, mColumnMerklistenID, source_parent);
        if (index2.isValid())
            accept = sourceModel()->data(index2).toInt() > 0;
    }
    if (accept && mFilterStatus != Alle)
    {
        index2 = sourceModel()->index(source_row, mColumnStatus, source_parent);
        if (index2.isValid())
        {
            int status = index2.data().toInt();
            switch (mFilterStatus)
            {
            case Alle:
                break;
            case NichtGebraut:
                accept = status < Sud_Status_Gebraut;
                break;
            case Gebraut:
                accept = status >= Sud_Status_Rezept;
                break;
            case NichtAbgefuellt:
                accept = status < Sud_Status_Abgefuellt;
                break;
            case GebrautNichtAbgefuellt:
                accept = status == Sud_Status_Gebraut;
                break;
            case Abgefuellt:
                accept = status >= Sud_Status_Abgefuellt;
                break;
            case NichtVerbraucht:
                accept = status == Sud_Status_Abgefuellt;
                break;
            case Verbraucht:
                accept = status >= Sud_Status_Verbraucht;
                break;
            }
        }
    }
    if (accept && !mFilterText.isEmpty())
    {
        QRegExp rx(mFilterText, Qt::CaseInsensitive, QRegExp::FixedString);
        index2 = sourceModel()->index(source_row, mColumnSudname, source_parent);
        accept = sourceModel()->data(index2).toString().contains(rx);
        if (!accept)
        {
            index2 = sourceModel()->index(source_row, mColumnKommentar, source_parent);
            accept = sourceModel()->data(index2).toString().contains(rx);
        }
        if (!accept)
        {
            ModelSud* modelSud = dynamic_cast<ModelSud*>(sourceModel());
            if (modelSud)
            {
                index2 = sourceModel()->index(source_row, mColumnId, source_parent);
                int id = sourceModel()->data(index2).toInt();
                SqlTableModel* model = modelSud->bh->modelMalzschuettung();
                int colSudId = model->fieldIndex("SudID");
                int colName = model->fieldIndex("Name");
                for (int i = 0; i < model->rowCount(); i++)
                {
                    if (model->data(model->index(i, colSudId)).toInt() == id)
                    {
                        QString name = model->data(model->index(i, colName)).toString();
                        accept = name.contains(rx);
                        if (accept)
                            break;
                    }
                }
                if (!accept)
                {
                    model = modelSud->bh->modelHopfengaben();
                    colSudId = model->fieldIndex("SudID");
                    colName = model->fieldIndex("Name");
                    for (int i = 0; i < model->rowCount(); i++)
                    {
                        if (model->data(model->index(i, colSudId)).toInt() == id)
                        {
                            QString name = model->data(model->index(i, colName)).toString();
                            accept = name.contains(rx);
                            if (accept)
                                break;
                        }
                    }
                }
                if (!accept)
                {
                    model = modelSud->bh->modelHefegaben();
                    colSudId = model->fieldIndex("SudID");
                    colName = model->fieldIndex("Name");
                    for (int i = 0; i < model->rowCount(); i++)
                    {
                        if (model->data(model->index(i, colSudId)).toInt() == id)
                        {
                            QString name = model->data(model->index(i, colName)).toString();
                            accept = name.contains(rx);
                            if (accept)
                                break;
                        }
                    }
                }
                if (!accept)
                {
                    model = modelSud->bh->modelWeitereZutatenGaben();
                    colSudId = model->fieldIndex("SudID");
                    colName = model->fieldIndex("Name");
                    for (int i = 0; i < model->rowCount(); i++)
                    {
                        if (model->data(model->index(i, colSudId)).toInt() == id)
                        {
                            QString name = model->data(model->index(i, colName)).toString();
                            accept = name.contains(rx);
                            if (accept)
                                break;
                        }
                    }
                }
            }
        }
    }
    return accept;
}
