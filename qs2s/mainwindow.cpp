#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow ( const QString& fileName, QWidget* parent ) :
    QMainWindow ( parent ),
    ui ( new Ui::MainWindow )
{
    mp_Notifier = new QSocketNotifier ( fileno ( stdin ), QSocketNotifier::Read, this );
    connect ( mp_Notifier, &QSocketNotifier::activated, this, &MainWindow::readCommand );

    ui->setupUi ( this );
    ui->svgViewer->openFile ( fileName );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startAnimation ( quint64 startDelayMs, quint64 intervalMs )
{
    ui->svgViewer->startAnimation ( startDelayMs, intervalMs );

}

void MainWindow::readCommand()
{
    QTextStream stream ( stdin, QIODevice::ReadOnly );
    qDebug() <<"COMMAND "<< stream.readLine();
}
