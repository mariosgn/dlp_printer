#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSocketNotifier>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow ( const QString& fileName,
                          QWidget* parent = 0 );
    ~MainWindow();
    void startAnimation ( quint64 startDelayMs, quint64 intervalMs );

private slots:
    void readCommand();

private:
    Ui::MainWindow* ui;
    QSocketNotifier *mp_Notifier;
};

#endif // MAINWINDOW_H
