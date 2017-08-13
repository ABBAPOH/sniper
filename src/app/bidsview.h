#pragma once

#include "bidsmodel.h"

#include <QtWidgets/QTreeView>

#include <memory>

class BidsView : public QTreeView
{
public:
    explicit BidsView(QWidget *parent = nullptr);

    std::unique_ptr<QMenu> createDefaultMenu(const QPoint &pos);

    void setModel(std::shared_ptr<BidsModel> model);

private slots:
    void makeBid();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    std::shared_ptr<BidsModel> _model;
};
