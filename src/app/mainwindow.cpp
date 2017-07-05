#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "loghandler.h"
#include "bidsmodel.h"

#include <QtCore/QCoreApplication>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(ui->auctionsView, &QTreeView::doubleClicked, this, &MainWindow::onDoubleClicked);
    connect(ui->bidsView, &QTreeView::doubleClicked, this, &MainWindow::onDoubleClicked2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setAuctionsModel(QAbstractItemModel* model)
{
    if (ui->auctionsView->model()) {
        disconnect(ui->actionUpdateAuctions, &QAction::triggered,
                   qobject_cast<AuctionsModel*>(ui->auctionsView->model()), &AuctionsModel::update);
    }

    ui->auctionsView->setModel(model);

    if (model) {
        connect(ui->actionUpdateAuctions, &QAction::triggered,
                qobject_cast<AuctionsModel*>(ui->auctionsView->model()), &AuctionsModel::update);
    }
}

void MainWindow::setBidsModel(QAbstractItemModel *model)
{
    ui->bidsView->setModel(model);
}

void MainWindow::onDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        qWarning() << "Invalid index";
        return;
    }

    const auto auctionsModel = qobject_cast<AuctionsModel *>(ui->auctionsView->model());
    const auto data = auctionsModel->auctionData(index);

    bool ok = false;
    int bid = QInputDialog::getInt(this, tr("Bid"), tr("Enter bid"), data.bid, data.bid, 1000000, 1, &ok);
    if (!ok)
        return;

    const auto bidsModel =qobject_cast<BidsModel *>(ui->bidsView->model());
    bidsModel->addBid(data, bid);
}

void MainWindow::onDoubleClicked2(const QModelIndex &index)
{
    if (!index.isValid()) {
        qWarning() << "Invalid index";
        return;
    }

    const auto bidsModel = qobject_cast<BidsModel *>(ui->bidsView->model());
    bidsModel->update(index);
}

