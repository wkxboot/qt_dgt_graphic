#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qthread.h"
#include "qmessagebox.h"
#include "qdatetime.h"

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*添加端口*/
       ui->port_name_list->addItem("COM1");
       ui->port_name_list->addItem("COM2");
       ui->port_name_list->addItem("COM3");
       ui->port_name_list->addItem("COM4");
       ui->port_name_list->addItem("COM5");
       ui->port_name_list->addItem("COM6");
       ui->port_name_list->addItem("COM7");
       ui->port_name_list->addItem("COM8");
       ui->port_name_list->addItem("COM9");
       ui->port_name_list->addItem("COM10");
       ui->port_name_list->addItem("COM11");
       ui->port_name_list->addItem("COM12");
       ui->port_name_list->addItem("COM13");
       ui->port_name_list->addItem("COM14");
       ui->port_name_list->setCurrentIndex(0);

       /*添加波特率*/
       ui->baudrates_list->addItem("115200");
       ui->baudrates_list->addItem("57600");
       ui->baudrates_list->addItem("19200");
       ui->baudrates_list->addItem("9600");
       ui->baudrates_list->setCurrentIndex(0);

       /*添加数据位*/
       ui->databits_list->addItem("8");
       ui->databits_list->addItem("7");
       ui->databits_list->setCurrentIndex(0);


       /*添加校验类型*/
       ui->parity_list->addItem("无校验");
       ui->parity_list->addItem("奇校验");
       ui->parity_list->addItem("偶校验");
       ui->parity_list->setCurrentIndex(0);



       QThread *comm_thread = new QThread(nullptr);

       m_comm = new communication(nullptr);

       m_comm->moveToThread(comm_thread);
       m_comm->m_serial->moveToThread(comm_thread);
       comm_thread->start();



       /*chart*/
       QChart *dgt_graphic = new QChart();


       /*坐标X轴*/
       x = new QValueAxis();
       /*10秒钟范围*/
       x->setRange(0,5000);
       x->setGridLineVisible(true);
       x->setTickCount(11);
       x->setTitleText("时间：MS");

       /*坐标Y轴*/
       y = new QValueAxis();
       /*-10kg到20kg*/
       y->setRange(16740000,16750000);
       y->setGridLineVisible(true);
       y->setTickCount(11);
       y->setTitleText("重量：g");



       unsigned int color[communication::LINE_SERVICE_CNT]= { Qt::red,Qt::blue,Qt::green,Qt::yellow};

       /*定义所有数据源*/
       for (int i = 0;i < communication::LINE_SERVICE_CNT; i++) {
           service[i] = new QLineSeries();

           service[i]->setUseOpenGL(true);
           service[i]->useOpenGL();

           dgt_graphic->addSeries(service[i]);
           dgt_graphic->setAxisX(x,service[i]);
           dgt_graphic->setAxisY(y,service[i]);
       }

       ui->graphicsView->setChart(dgt_graphic);

       QObject::connect(this,SIGNAL(open_port(QString,int,int,int)),m_comm,SLOT(handle_open_port(QString,int,int,int)));
       QObject::connect(m_comm,SIGNAL(rsp_open_port(int,int)),this,SLOT(handle_open_port_rsp(int,int)));

       QObject::connect(m_comm,SIGNAL(notify_data_stream(QList<QPointF>)),this,SLOT(handle_notify_data_stream(QList<QPointF>)));

}

MainWindow::~MainWindow()
{
    delete ui;
}


/*通知串口操作*/
void MainWindow::on_start_button_clicked()
{
    QString port_name;
    int baudrates,databits,parity;

    port_name = ui->port_name_list->currentText();
    baudrates = ui->baudrates_list->currentText().toInt();
    databits = ui->databits_list->currentText().toInt();
    parity = ui->parity_list->currentText().toInt();

    emit open_port(port_name,baudrates,databits,parity);
}


/*处理串口操作结果*/
void MainWindow::handle_open_port_rsp(int rc,int code)
{
    if (code == communication::SERIAL_CODE_OPEN) {
        if (rc == communication::SERIAL_SUCCESS){
            ui->start_button->setText("停止");
            ui->port_name_list->setEnabled(false);
            ui->baudrates_list->setEnabled(false);
            ui->databits_list->setEnabled(false);
            ui->parity_list->setEnabled(false);

        } else {
            QMessageBox::warning(this,"错误","串口打开失败！",QMessageBox::Ok);
        }
    }
    if (code == communication::SERIAL_CODE_CLOSE) {
        if (rc == communication::SERIAL_SUCCESS){
            ui->start_button->setText("开始");
            ui->port_name_list->setEnabled(true);
            ui->baudrates_list->setEnabled(true);
            ui->databits_list->setEnabled(true);
            ui->parity_list->setEnabled(true);

        } else {
            QMessageBox::warning(this,"错误","串口关闭失败！",QMessageBox::Ok);
        }
    }
}

/*处理数据流*/
void MainWindow::handle_notify_data_stream(QList<QPointF> line)
{
   QVector<QPointF> s[communication::LINE_SERVICE_CNT];

   int offset = 0,offset_max = 0;
   int data_cnt;
   int x_min,x_max;

   QVector<QPointF> vector;

   x_min = x->min();
   x_max = x->max();

   for (int i = 0;i < communication::LINE_SERVICE_CNT && i < line.size();i ++) {
       /*每个数据源向量表*/
       s[i] = service[i]->pointsVector();

       /*如果新数据大于坐标轴*/
       offset = line[i].x() - x_max;
       if (offset > offset_max) {
           offset_max = offset;
       }
       s[i].append(line[i]);

       data_cnt = s[i].size();
       if (data_cnt > communication::SERVICE_DATA_CNT) {
            s[i].removeFirst();
       }
       service[i]->replace(s[i]);
   }

     if (offset > 0) {
         x->setRange(x_min + offset,x_max + offset);
     }

}
