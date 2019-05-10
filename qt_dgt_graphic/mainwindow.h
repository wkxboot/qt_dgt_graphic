#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qchart.h"
#include "qlineseries.h"
#include "qvalueaxis.h"
#include "math.h"
#include "communication.h"
#include "qmessagebox.h"

using namespace QtCharts;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void open_port(QString,int,int,int);

public slots:
    void handle_open_port_rsp(int,int);
    void handle_notify_data_stream(QList<QPointF> line);

private slots:
    void on_start_button_clicked();

private:
    Ui::MainWindow *ui;
    communication *m_comm;
    QLineSeries *service[communication::LINE_SERVICE_CNT];
    QValueAxis *x;
    QValueAxis *y;
};

#endif // MAINWINDOW_H
