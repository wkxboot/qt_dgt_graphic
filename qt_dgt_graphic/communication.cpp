#include "communication.h"
#include "qdebug.h"


communication::communication(QObject *parent) : QObject(parent)
{
    m_open = false;
    m_buffer.clear();

    m_serial = new QSerialPort(this);

   qRegisterMetaType<QList<QPointF>>("QList<QPointF>");

    QObject::connect(m_serial,SIGNAL(readyRead()),this,SLOT(handle_data_ready()));
}

/*处理数据*/
void communication::handle_data_ready()
{
    int index;
    QByteArray temp;
    QStringList data_list;
    QStringList value_list;


    QList<QPointF> list;
    QPointF value;

   m_buffer.append(m_serial->readAll());

    if (m_buffer.isEmpty()) {
        return;
    }

    /*解析数据*/
    while ((index = m_buffer.indexOf("\r\n",0)) >= 0) {

    temp = m_buffer.mid(0,index);
    m_buffer.remove(0,index + 2);

    if (temp.size() == 0) {
        continue;
    }

    data_list = QString::fromLocal8Bit(temp).split(';');
    for (int i = 0;i < data_list.size();i++) {
        value_list = data_list[i].split(',');
        if (value_list.size() < 2) {
            continue;
        }
        /*只用2个*/
        value.setX(value_list[0].toInt());
        value.setY(value_list[1].toInt());
        list.append(value);
        qDebug()<< QString("[data:%1]:%2.").arg(i).arg(data_list.at(i));
    }
    emit notify_data_stream(list);

    }




}




/*处理串口操作*/
void communication::handle_open_port(QString port_name,int baud_rates,int data_bits,int parity)
{
    bool success;

    if (m_open) {
        m_serial->close();
        qDebug() << QString(port_name + "关闭成功.");
        emit rsp_open_port(SERIAL_SUCCESS,SERIAL_CODE_CLOSE);
        m_open = false;
    } else {
        m_serial->setPortName(port_name);
        m_serial->setBaudRate(baud_rates);
        if (data_bits == 8) {
           m_serial->setDataBits(QSerialPort::Data8);
        } else {
           m_serial->setDataBits(QSerialPort::Data7);
        }

        if (parity == 0){
            m_serial->setParity(QSerialPort::NoParity);
        } else if (parity == 1){
            m_serial->setParity(QSerialPort::OddParity);
        } else  {
            m_serial->setParity(QSerialPort::EvenParity);
        }

        success = m_serial->open(QSerialPort::ReadWrite);
        if (success) {
            qDebug() << QString(port_name + "打开成功.");
            m_open = true;
            m_serial->flush();

            emit rsp_open_port(SERIAL_SUCCESS,SERIAL_CODE_OPEN);/*发送成功信号*/
        } else {
            qDebug() << QString(port_name + "打开失败.");
            emit rsp_open_port(SERIAL_FAIL,SERIAL_CODE_OPEN);/*发送失败信号*/
        }
    }
}






