#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QObject>
#include "QByteArray"
#include "QVector"
#include "QPointF"
#include "QTimer"
#include "QtSerialPort/qserialport.h"
#include "QtSerialPort/qserialportinfo.h"

class communication : public QObject
{
    Q_OBJECT
public:
    enum {
        SERIAL_CODE_OPEN = 1,
        SERIAL_CODE_CLOSE = 2,
        SERIAL_SUCCESS = 0,
        SERIAL_FAIL = -1,

        LINE_SERVICE_CNT = 4


    };
    QSerialPort *m_serial;

public:
    explicit communication(QObject *parent = nullptr);

signals:
    void rsp_open_port(int rc,int code);
    void notify_data_stream(QList<QPointF> line);


public slots:
    void handle_open_port(QString port_name,int baud_rates,int data_bits,int parity);
    void handle_data_ready();

private:
    bool m_open;
    QByteArray m_buffer;
    QTimer *m_rsp_timer;

};

#endif // COMMUNICATION_H
