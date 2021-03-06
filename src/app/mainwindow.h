#pragma once

#include "bidsmodel.h"

#include <QAbstractItemModel>
#include <QtCore/QSortFilterProxyModel>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setAuctionsModel(std::shared_ptr<AuctionsModel> model);
    void setBidsModel(std::shared_ptr<BidsModel> model);

private slots:
    void onDoubleClicked(const QModelIndex &idx);
    void onDoubleClicked2(const QModelIndex &idx);
    void onAuctionsFilterChanged(const QString &text);

private:
    Ui::MainWindow *ui;
    std::shared_ptr<AuctionsModel> _auctionsModel;
    std::shared_ptr<QSortFilterProxyModel> _auctionsProxyModel;
    std::shared_ptr<BidsModel> _bidsModel;
};
