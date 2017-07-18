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

void MainWindow::setAuctionsModel(std::shared_ptr<AuctionsModel> model)
{
    if (_auctionsModel == model)
        return;

    if (_auctionsModel) {
        disconnect(ui->actionUpdateAuctions, &QAction::triggered,
                   _auctionsModel.get(), &AuctionsModel::update);
    }

    _auctionsModel = model;
    ui->auctionsView->setModel(model.get());

    if (model) {
        connect(ui->actionUpdateAuctions, &QAction::triggered,
                _auctionsModel.get(), &AuctionsModel::update);
    }
}

void MainWindow::setBidsModel(std::shared_ptr<BidsModel> model)
{
    if (_bidsModel == model)
        return;

    _bidsModel = model;
    ui->bidsView->setModel(model.get());
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

