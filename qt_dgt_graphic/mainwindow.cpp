#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qthread.h"
#include "qmessagebox.h"
#include "qdatetime.h"
#include "qfiledialog.h"
#include "qsettings.h"
#include "qdebug.h"

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
       x->setRange(X_WINDOWS_START,X_WINDOWS_START + X_WINDOWS_SIZE);
       x->setGridLineVisible(true);
       x->setTickCount(X_WINDOWS_SIZE / X_WINDOWS_STEP + 1);
       x->setTitleText("X轴");

       /*坐标Y轴*/
       y = new QValueAxis();
       /*-10kg到20kg*/
       y->setRange(Y_WINDOWS_START,Y_WINDOWS_START + Y_WINDOWS_SIZE);
       y->setGridLineVisible(true);
       y->setTickCount(Y_WINDOWS_SIZE / Y_WINDOWS_STEP + 1);
       y->setTitleText("Y轴");



       Qt::GlobalColor color[communication::LINE_SERVICE_CNT]= { Qt::red,Qt::green,Qt::yellow,Qt::blue};

       /*定义所有数据源*/
       for (int i = 0;i < communication::LINE_SERVICE_CNT; i++) {
           service[i] = new QLineSeries();
           service[i]->setColor(color[i]);
           service[i]->setName("未定义");

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
    parity = ui->parity_list->currentIndex();

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
       /*处理数据显示*/
      ui->data_stream_display->append(QString::number(line[i].x(),'f',0) + "," + QString::number(line[i].y(),'f',0));
    }
   if (ui->data_stream_display->document()->lineCount() > 100) {
       ui->data_stream_display->clear();

   }
    if (offset > 0) {
        x->setRange(x_min + offset,x_max + offset);
    }




}

void MainWindow::on_data_stream_display_textChanged()
{
     ui->data_stream_display->moveCursor(QTextCursor::End);
}

void MainWindow::parse_configration(QString file_name)
{
    /*解析颜色*/
    QByteArray file_text;
    QByteArray temp;

    QString color;
    QString split;
    QString start,end;
    int temp_index,split_index,start_index,end_index;


    QFile *open_file = new QFile(file_name);
    if (open_file->open(QIODevice::ReadOnly)) {
        file_text = open_file->readAll();
        if (file_text.contains("[color]")) {
            temp_index = file_text.indexOf("[color]");
            split_index = file_text.indexOf("=",temp_index);
            start_index = file_text.indexOf("\"",split_index);


        }
    }

}


/*打开配置文件*/
void MainWindow::on_open_configration_button_clicked()
{

    QString path;
    /*上次路径*/
    QSettings *setting = new QSettings("./Setting.ini", QSettings::IniFormat);  //QSettings能记录一些程序中的信息，下次再打开时可以读取出来
    /*文件选择对话框*/
    QFileDialog *file_dialog = new QFileDialog(this);

    file_dialog->setWindowTitle("选择配置文件");

    file_dialog->setDirectory(setting->value("last_file_path").toString());

    file_dialog->setNameFilter(tr("配置文件(*.cfg)"));
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    file_dialog->setFileMode(QFileDialog::ExistingFile);
    //设置视图模式
    file_dialog->setViewMode(QFileDialog::Detail);


    if (file_dialog->exec() ==  QDialog::Accepted) {
        path = file_dialog->selectedFiles()[0];
        if (!path.isEmpty()) {
            setting->setValue("last_file_path",path);
             parse_configration(path);
        }

    } else {
        file_dialog->close();
    }
}

/*保存配置文件*/
void MainWindow::save_configration()
{
   QFile *new_file;
   QByteArray new_text;

   /*上次路径*/
   QSettings *setting = new QSettings("./Setting.ini", QSettings::IniFormat);  //QSettings能记录一些程序中的信息，下次再打开时可以读取出来

   QString file_name = QFileDialog::getSaveFileName(this, tr("保存配置文件"),setting->value("last_file_path").toString(), "*.cfg");
        if (!file_name.isEmpty())
        {
            // 如果文件后缀为空，则默认使用.pdf
            if (QFileInfo(file_name).suffix().isEmpty())
            {
                file_name.append(".cfg");
            }
         new_file = new QFile(file_name);
         new_file->open(QIODevice::WriteOnly);

         new_text = ui->configration_text_display->toPlainText().toUtf8();

         new_file->write(new_text);

         new_file->close();
        }


}

void MainWindow::on_save_configration_button_clicked()
{
    save_configration();
}
