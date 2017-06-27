#pragma once

#include "bidsmodel.h"

#include <QAbstractItemModel>
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

    void setAuctionsModel(QAbstractItemModel *model);
    void setBidsModel(QAbstractItemModel *model);

private slots:
    void onDoubleClicked(const QModelIndex &idx);

private:
    Ui::MainWindow *ui;
};
