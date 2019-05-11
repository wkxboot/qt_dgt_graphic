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
    enum {
        X_WINDOWS_START = 0,
        X_WINDOWS_SIZE = 2000,
        X_WINDOWS_STEP = 200,

        Y_WINDOWS_START = 0,
        Y_WINDOWS_SIZE = 20000,
        Y_WINDOWS_STEP = 1000

    };

    void parse_configration(QString file_name);
    void save_configration();
signals:
    void open_port(QString,int,int,int);

public slots:
    void handle_open_port_rsp(int,int);
    void handle_notify_data_stream(QList<QPointF> line);

private slots:
    void on_start_button_clicked();

    void on_data_stream_display_textChanged();

    void on_open_configration_button_clicked();

    void on_save_configration_button_clicked();

private:
    Ui::MainWindow *ui;
    communication *m_comm;
    QLineSeries *service[communication::LINE_SERVICE_CNT];
    QValueAxis *x;
    QValueAxis *y;

    int x_windows_size;
    int y_windows_size;
    int x_windows_step;
    int y_windows_step;

};

#endif // MAINWINDOW_H
