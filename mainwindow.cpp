#include <QtCore/QStringList>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "plot.h"
#include <QString>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dataArray(0)
{
    ui->setupUi(this);

/* UI Elements */
    createActions();
    createMenus();
    createToolBars();
    createCentralWidget();
    initMainWidgetCloseState();
    connect(controlButton, SIGNAL(triggered()), this, SLOT(openPortButtonClick()));

    procEnumerate();

/* SerialPort */
    serial = new QSerialPort();
    connect(serial, SIGNAL(readyRead()), this, SLOT(serialDataReceive()));
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)),
            this,   SLOT(serialErrorHandler(QSerialPort::SerialPortError)));

    connect(sendButton, SIGNAL(clicked()), this, SLOT(serialDataSend()));
}


MainWindow::~MainWindow()
{
    /*if (this->enumerator && this->enumerator->isEnabled())
            this->enumerator->setEnabled(false);*/

    /*if (this->serial && this->serial->isOpen())
        this->serial->close();*/

    delete ui;
}


void MainWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);

    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        break;
    }
}





/* Private SLOTS section */
void MainWindow::procEnumerate(/*const QStringList &l*/)
{
    qDebug() << "procEnumerate()";

    // Fill ports box.
    portBox->clear();

    QStringList list;

    //QSerialPortInfo info = enumerator->availablePorts();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        list.append(info.portName());

    portBox->addItems(list);
}


void MainWindow::openPortButtonClick()
{
    if(this->serial)
    {
        bool result = this->serial->isOpen();
        if(result)
        {
            this->serial->close();
            qDebug() << "Port closed";
            ui->statusBar->showMessage(tr("Port closed"));
            result = false;
        }
        else
        {
            //SettingsDialog::Settings p = settings->settings();
            serial->setPortName(this->portBox->currentText());
            serial->setBaudRate(QSerialPort::Baud115200);
            serial->setDataBits(QSerialPort::Data8);
            serial->setParity(QSerialPort::NoParity);
            serial->setStopBits(QSerialPort::OneStop);
            serial->setFlowControl(QSerialPort::NoFlowControl);

            result = this->serial->open(QIODevice::ReadWrite);

            if(!result)
            {
                qDebug() << "!Serial device: " << this->serial->objectName() << " open fail!";
                QMessageBox::critical(this, tr("Error"), serial->errorString());
                ui->statusBar->showMessage(tr("Error Port opening!"));
                return;
            }
            else
            {
                qDebug() << "Port opened";
                ui->statusBar->showMessage(tr("Port opened"));
                /*ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
                   */
            }

            plot->start();
        }

        (result) ? this->initMainWidgetOpenState() : this->initMainWidgetCloseState();
        this->sendButton->setEnabled(result ? true : false);
    }
}


void MainWindow::procSerialMessages(const QString &msg, QDateTime dt)
{
    qDebug()<< "procSerialMessages" << " ...";
    QString s = dt.time().toString() + " > " + msg;
    textEdit->appendPlainText(s);
}


void MainWindow::serialDataReceive()
{

    /*
     *     QByteArray temp_data = serial->readLine(); // Заполняем массив данными
    if (temp_data.indexOf("\n") != -1) {
        // Получили переход на новую строку - значит приняли данные
        bytes += temp_data;

        // Грязная магия - парсим данные. Делим массив на две части: до пробела (команда) и после (параметры)
        // Затем очищаем параметры от \r\n, и преобразуем в unsigned int
        uint adc_value = bytes.split(' ').at(1).split('\r').at(0).toUInt();

        // Преобразуем данные АЦП в миливольты.
        // А за тем uint преобразовывается в строку, которую отображаем в тестовом поле leADC
        ui->leADC->setText(QString::number(adc_value / 1.365));
        bytes.clear();
    } else {
        bytes += temp_data;
    }
*/
    qDebug()<<"procSerialDataReceive"<<" receiving";

    if (this->serial && this->serial->isOpen())
    {
        QByteArray byte = this->serial->readAll();
        qDebug()<<byte;

        qDebug()<<"1";
        this->printTrace(byte);
qDebug()<<"2";

        if(byte.at(0) != '\n')
        {
            dataArray.append(byte);
            qDebug()<<"3";

            double elapsed = (plot->dclock_elapsed())/ 1000.0;
            QPointF point(elapsed, 9);
            plot->appendPoint(point);
            QPointF point2(elapsed, 15);
            plot->appendPoint(point2);
            plot->setIntervalLength(plot->dclock_elapsed()/1000.0);
        }
        else
        {
            qDebug()<<"4 "<<dataArray.size();
            if(dataArray.size()==0) return;

            if(dataArray.at(0)=='C')
            {
                qDebug()<<"5";
                if(dataArray.at(3) == '3')
                {
                   double elapsed = (plot -> dclock_elapsed())/ 1000.0;
                   QByteArray u;
                   //u[0]= dataArray.at(5);
                   int j=5;
                   /*for(j=5;j<9;j++)
                   {
                        if(dataArray.at(j)!='\r') u[j-5]= dataArray.at(j);
                   }*/
                   while(dataArray.at(j)!='\r')
                   {
                       u[j-5]= dataArray.at(j);
                       j++;
                   }
                   QPointF point(elapsed,u.toDouble()*5/1024);
                   plot ->appendPoint(point);
                   RecToFile(point);
                }
            }

            dataArray = 0;
         }
    }
}


void MainWindow::RecToFile(QPointF point)
{
    qDebug() << "RecToFile ...recording";
    /*
    QFile f("test.dat");
    if (f.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&f);
        out << point.x() << "\t" << point.y() << "\n";
        f.close();
    }
    else
    {
        qWarning("Can not open file test.dat");
    }
    */
}


void MainWindow::printTrace(const QByteArray &data)
{
    //textEdit->setTextColor((directionRx) ? Qt::darkBlue : Qt::darkGreen);
    textEdit->insertPlainText(QString(data));

    QScrollBar *bar = textEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}


void MainWindow::serialErrorHandler(QSerialPort::SerialPortError error)
{
    qDebug() << "serial Error: " << serial->errorString();
    if(error == QSerialPort::ResourceError)
    {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        openPortButtonClick();
    }

    ui->statusBar->showMessage(tr("Error!"));
}


void MainWindow::serialDataSend()
{
    qDebug() << "Data sending...";
    QByteArray data = textSend->toPlainText().toUtf8();

    if(this->checksendL->isChecked()) data.append('\n');
    if(this->checksendL->isChecked()) data.append('\0');

    QString string;

    QString alertHtml = "<font color=\"DeepPink\">";
    QString notifyHtml = "<font color=\"Lime\">";
    QString infoHtml = "<font color=\"Aqua\">";
    QString endHtml = "</font><br>";

    string.append(notifyHtml);
    QString time = QTime::currentTime().toString();
    string.append(time);
    string.append(": ");
    string.append(data);
    string.append(endHtml);

    textEdit->appendHtml(string);

    serial->write(data);
}






/* Private METHODS section */
void MainWindow::initMainWidgetCloseState()
{
    this->sendButton->setEnabled(false);
    portBox->setEnabled(true);
    controlButton->setText(QString(tr("Open")));
}


void MainWindow::initMainWidgetOpenState()
{
    portBox->setEnabled(false);
    controlButton->setText(QString(tr("Close")));
}


void MainWindow::createActions()
{
     aboutAct = new QAction(tr("About"), this);
     aboutAct->setStatusTip(tr("Show the About box"));
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

     aboutqtAct = new QAction(tr("AboutQT"), this);
     aboutqtAct->setStatusTip(tr("Show the Qt library's About box"));
     connect(aboutqtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

     controlButton = new QAction(tr("Open"), this);
     //controlButton->setStatusTip(tr("Open port"));
}


void MainWindow::createMenus()
{
     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutqtAct);
     helpMenu->addAction(aboutAct);
}


void MainWindow::createCentralWidget()
{
    plot = new Plot(this);
    LCDBig = new QLCDNumber(6);
    LCDBig->setFixedWidth(260);
    float x = 54.0;
    LCDBig->display(x);

    checksendL = new QCheckBox("\\n");
    checkSendN = new QCheckBox("\\0");
    checksendL->setFixedWidth(30);
    checkSendN->setFixedWidth(30);

    textSend = new QPlainTextEdit;
    textSend->setFixedHeight(30);
    textSend->setFixedWidth(200);

    textEdit = new QPlainTextEdit;
    textEdit->setFixedWidth(260);

    sendButton = new QPushButton("Send");
    sendButton->setFixedWidth(260);

    HLayout = new QHBoxLayout();
    GridLayoutLeft = new QGridLayout();

    VLayoutRight = new QVBoxLayout();
    VLayoutRight->addWidget(plot);

    GridLayoutLeft->addWidget(LCDBig, 0,0, 1,3);
    GridLayoutLeft->addWidget(checksendL, 1,1);
    GridLayoutLeft->addWidget(checkSendN, 1,2);
    GridLayoutLeft->addWidget(textSend, 1,0);
    GridLayoutLeft->addWidget(sendButton, 2,0, 1,3);
    GridLayoutLeft->addWidget(textEdit, 3,0, 2,3);

    HLayout->addLayout(GridLayoutLeft);
    HLayout->addLayout(VLayoutRight);

    centralWidget()->setLayout(HLayout);
    setWindowTitle(tr("UART Terminal & Graph"));

    this->resize(800,400);
}


void MainWindow::createToolBars()
{
    statusBar = new QStatusBar();

    portBox = new QComboBox(ui->mainToolBar);
    portBox->setObjectName("Ports");

    ui->mainToolBar->addWidget(portBox);
    ui->mainToolBar->addAction(controlButton);

}


void MainWindow::about()
{
    QMessageBox::about(this, tr("About Application"),
        tr("<b>QSerialDevice and QWT for HabraHabr</b> - example of simple terminal<p> Created by Alex Alexeev. 2011"));
}
