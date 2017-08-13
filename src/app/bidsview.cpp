#include "bidsview.h"

#include <QtWidgets/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtCore/QDebug>

BidsView::BidsView(QWidget* parent):
    QTreeView(parent)
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

std::unique_ptr<QMenu> BidsView::createDefaultMenu(const QPoint& pos)
{
    std::unique_ptr<QMenu> menu(new QMenu);

    const auto index = indexAt(pos);
    if (index.isValid()) {
        const auto action = menu->addAction("Make bid");
        action->setData(QPersistentModelIndex(index));
        connect(action, &QAction::triggered, this, &BidsView::makeBid);
    }

    return menu;
}

void BidsView::setModel(std::shared_ptr<BidsModel> model)
{
    _model = model;
    QTreeView::setModel(model.get());
}

void BidsView::makeBid()
{
    const auto action = qobject_cast<QAction *>(sender());
    if (!action) {
        qWarning() << "sender is not an action";
        return;
    }

    const auto index = action->data().value<QPersistentModelIndex>();
    _model->makeBid(index);
}

void BidsView::contextMenuEvent(QContextMenuEvent* event)
{
    auto menu = createDefaultMenu(event->pos());
    menu->exec(event->globalPos());
}
