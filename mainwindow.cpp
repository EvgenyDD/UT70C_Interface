#include <QtCore/QStringList>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "plot.h"
#include <QString>

#define SCAN_MS 50

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dataArray(0),
    disconnectTimer(10)
{
    ui->setupUi(this);

/* UI Elements */
    createActions();
    //createMenus();
    createToolBars();
    createCentralWidget();
    initMainWidgetCloseState();
    connect(controlButton, SIGNAL(triggered()), this, SLOT(openPortButtonClick()));
    connect(refreshButton, SIGNAL(triggered()), this, SLOT(procEnumerate()));

    refresh->setValue(SCAN_MS);
    refresh->setMaximum(100000);
    refresh->setMinimum(SCAN_MS);
    connect(refresh, SIGNAL(valueChanged(int)), this, SLOT(refreshScanRate(int)));

    procEnumerate();

/* SerialPort */
    serial = new QSerialPort();
    connect(serial, SIGNAL(readyRead()), this, SLOT(serialDataReceive()));
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)),
            this,   SLOT(serialErrorHandler(QSerialPort::SerialPortError)));

    timer = new QTimer();
    timer->setInterval(SCAN_MS);
    connect(timer, SIGNAL(timeout()), this, SLOT(serialWriteData()));
    timer->start();
}


MainWindow::~MainWindow()
{
    if (this->serial && this->serial->isOpen())
        this->serial->close();
}




/* Private SLOTS section */
void MainWindow::procEnumerate(/*const QStringList &l*/)
{
    QStringList list;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        list.append(info.portName());

    portBox->clear();
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
            serial->setBaudRate(QSerialPort::Baud9600);
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
            }

            plot->start();
        }

        (result) ? this->initMainWidgetOpenState() : this->initMainWidgetCloseState();
        //this->sendButton->setEnabled(result ? true : false);
    }
}



void MainWindow::serialDataReceive()
{
    if (this->serial && this->serial->isOpen())
    {
        QByteArray byte = this->serial->readAll();
        if(byte.at(0) == '\x0A')
        {
            if(/*dataArray.size() == 7 || */dataArray.size() == 11)
            {
                UT70_GetData(dataArray);
                disconnectTimer=0;
            }

            dataArray = 0;
            return;
        }
        dataArray += byte;
    }
}


void MainWindow::refreshScanRate(int value)
{
    timer->setInterval(value);
}


void MainWindow::UT70_GetData(QByteArray array)
{   
    BRange->setEnabled(true);
    BHold->setEnabled(true);
    BRel->setEnabled(true);
    BMax->setEnabled(true);
    BCap->setEnabled(true);
    BHz->setEnabled(true);

    float base=1, result=0;
    QString str="", desc="";
    uint div = (((uchar)array.at(2))>>3)&7;
    bool overload = ((uchar)array.at(4))&(1<<3);
    bool autoMode = !(((uchar)array.at(2))&(1<<6));
    LAuto->setText(autoMode?"AUTO":"");
    LHold->setText((((uchar)array.at(4))&(1<<0))?"HOLD":"");
    BHzActive = (((uchar)array.at(4))&(1<<2))?true:false;
    BDuty = (((uchar)array.at(2))&7)==5?true:false;

    bool extend5num = ((uchar)array.at(4))&(1<<2);
    int sign = (((uchar)array.at(4))&(1<<4))?-1:1;

    if(extend5num) result += ((uchar)array.at(5)-0x30)*10000;
    for(int i=6; i<10; i++)
        result += ((uchar)array.at(i)-0x30)*pow(10, 9-i);

    if(BHzActive)
    {
        if(div<=1)
        {
            base = pow(10, 1+div);
            desc.append("Hz");
        }
        else
        {
            base = pow(10, 6-div);
            desc.append("kHz");
        }
        //qDebug() << "Div="<<div;
    }
    else if(BDuty)
    {
        base = 100;
        desc.append("%");
    }
    else
        switch(((uchar)array.at(1)))
        {
        case 248: //V~
            if(!div) base = extend5num?100:10;
            else     base = pow(10, (4-div+(extend5num?1:0)));
            desc.append(!div?"mV":"V");
            break;

        case 240: //V=
            base = pow(10, 3-div+(extend5num?1:0));
            desc.append("V");
            break;

        case 232: //mV
            base = pow(10, 2-div+(extend5num?1:0));
            desc.append("mV");
            break;

        case 168: //A=
        case 169: //A~
            base = pow(10, 3-div+(extend5num?1:0));
            if(BHzActive)
                desc.append("Hz2");
            else
                desc.append("A");
            break;

        case 176: //mA=
        case 177: //mA~
            base = pow(10, 2-div+(extend5num?1:0));
            if(BHzActive)
                desc.append("Hz1");
            else
                desc.append("mA");
            break;

        case 224: //R
            if(!div)        base = 1000;
            else if(div<=3) base = pow(10, 4-div+(extend5num?1:0));
            else if(div<=5) base = pow(10, 7-div+(extend5num?1:0));
            else            base = 100;

            if(!div)             desc.append("Ω");
            if(div>=1 && div<=3) desc.append("kΩ");
            if(div>=4 && div<=5) desc.append("MΩ");
            if(div == 6)         desc.append("nS");
            break;

        case 225: //C
            base = pow(10, 3-(div%3)+(extend5num?1:0));

            if(div <= 2) desc.append("nF");
            else         desc.append("uF");
            break;
        }

    str.append(QString::number(result/base, 'f', log10(base)));
    if(sign == -1) str.insert(0,'-');

    int num = extend5num?6:5;
    if(sign == -1) num++;
    if(base == 1) num--;

    if(str.length() < num)
        while(str.length() != num)
            str.insert((sign==1)?0:1, '0');

    LCDBig->display(overload ? "" : str);
    LUnit->setText(overload ? "OV" : desc);

    if(!overload) plot->appendPoint(QPointF(plot->d_clock.elapsed()/1000.0, (qreal)(sign*result/base)));
//qDebug()<<"PNT: "<<QPointF(plot->d_clock.elapsed()/1000.0, (float)(sign*result/base));

    static QString dt;
    bool e = (dt == QTime::currentTime().toString())?false:true;
    QString out = (     (e?"<font color=\"MediumBlue\">":"") +
                        QTime::currentTime().toString() +
                        " > " +
                        (overload ? "" :str) +
                        (overload ? "OV" : desc) +
                        (e?"</font>":""));

    textEdit->appendHtml(out);

    //if(record->isChecked()) RecToFile(out);


    dt = QTime::currentTime().toString();

    QScrollBar *bar = textEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}


void MainWindow::RecToFile(/*QPointF point*/QString str)
{   
#if 0
    QFile f("UT70C_Log_.log" /*+ QDateTime::currentDateTime().toString() + ".log"*/);
    if(f.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream out(&f);
        out << str << "\n";
        f.close();
    }
    else
        qWarning("Can not open file");
#endif
}


void MainWindow::serialErrorHandler(QSerialPort::SerialPortError error)
{
    qDebug() << "serial Error: " << serial->errorString();
    if(error == QSerialPort::ResourceError)
    {
        openPortButtonClick();
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());    
    }
}



void MainWindow::serialWriteData()
{
    disconnectTimer++;
    if(disconnectTimer > 1000/SCAN_MS)
    {
        LCDBig->display("");
        LUnit->setText("N.C.");
        BRange->setEnabled(false);
        BHold->setEnabled(false);
        BRel->setEnabled(false);
        BMax->setEnabled(false);
        BCap->setEnabled(false);
        BHz->setEnabled(false);
        plot->stop();
    }
    if(!serial->isOpen()) return;

    QByteArray msg;
    msg[0] = 0x89;

    serial->write(msg);
}


void MainWindow::serialWriteCommand()
{
    if(!serial->isOpen()) return;

    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    QString clickedOperator = clickedButton->text();

    QByteArray msg;
    if(clickedOperator == "Range") msg[0] = 0xA1;
    else if(clickedOperator == "Hold") msg[0] = 0xA5;
    else if(clickedOperator == "Rel.") msg[0] = 0xA7;
    else if(clickedOperator == "MaxMin") msg[0] = 0xA9;
    else if(clickedOperator == "Cap = ~") msg[0] = 0xAD;
    else if(clickedOperator == "Hz") msg[0] = 0xA3;
    else return;

    qDebug() << "Send_com: " << msg.toHex();
    serial->write(msg);
}






/* Private METHODS section */
void MainWindow::initMainWidgetCloseState()
{
    //this->sendButton->setEnabled(false);
    portBox->setEnabled(true);
    controlButton->setText(QString(tr("Open")));
    refreshButton->setEnabled(true);
}


void MainWindow::initMainWidgetOpenState()
{
    portBox->setEnabled(false);
    controlButton->setText(QString(tr("Close")));
    refreshButton->setEnabled(false);
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
     refreshButton = new QAction(tr("Refresh"), this);
     //record = new QCheckBox("Record to file");
     refresh = new QSpinBox();//"Scan Time in ms."

}


void MainWindow::createMenus()
{
     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutqtAct);
     helpMenu->addAction(aboutAct);
}


void MainWindow::createToolBars()
{
    statusBar = new QStatusBar();

    portBox = new QComboBox(ui->mainToolBar);
    portBox->setObjectName("Ports");
    ui->mainToolBar->addAction(refreshButton);
    ui->mainToolBar->addWidget(portBox);
    ui->mainToolBar->addAction(controlButton);
    //ui->mainToolBar->addWidget(record);
    ui->mainToolBar->addWidget(refresh);
}


void MainWindow::createCentralWidget()
{
    subPlotHLayout = new QHBoxLayout();

    plot = new Plot(this);
    LCDBig = new QLCDNumber(8);
    //LCDBig->setFixedWidth(260);
    LCDBig->setFixedHeight(80);

    LUnit = new QLabel();
    LUnit->setText("N.C.");
    LUnit->setStyleSheet("font: 24pt;");

    LAuto = new QLabel("Auto");
    LAuto->setFixedHeight(14);
    LAuto->setStyleSheet("font: 12pt;");
    LHold = new QLabel("Hold");
    LHold->setStyleSheet("font: 12pt;");

#define BTN_WIDTH 55
    BRange = new QPushButton("Range");
    BRange->setFixedWidth(BTN_WIDTH/5*4);
    BHold = new QPushButton("Hold");
    BHold->setStyleSheet("background-color: red");
    BHold->setFixedWidth(BTN_WIDTH/5*4);
    BRel = new QPushButton("Rel.");
    BRel->setFixedWidth(BTN_WIDTH/3*2);
    BMax = new QPushButton("MaxMin");
    BMax->setStyleSheet("background-color: blue");
    BMax->setFixedWidth(BTN_WIDTH);
    BCap = new QPushButton("Cap = ~");
    BCap->setFixedWidth(BTN_WIDTH+5);
    BCap->setStyleSheet("background-color: yellow");
    BHz = new QPushButton("Hz");
    BHz->setStyleSheet("background-color: green");
    BHz->setFixedWidth(BTN_WIDTH/2);

    /*checksendL = new QCheckBox("\\n");
    checkSendN = new QCheckBox("\\0");
    checksendL->setFixedWidth(30);
    checkSendN->setFixedWidth(30);*/


    subPlotHLayout = new QHBoxLayout();
    plotMode[0] = new QRadioButton("All");
    plotMode[0]->setChecked(true);
    plotMode[1] = new QRadioButton("Smooth");
    plotMode[2] = new QRadioButton("Frame");
    intervalBox = new QSpinBox();
    intervalBox->setMaximum(120);
    intervalBox->setMinimum(2);
    intervalBox->setValue(10);
    updateOnTime = new QCheckBox("TimeUpdate");
    updateOnTime->setChecked(true);

    VLayoutRight = new QVBoxLayout();
    VLayoutRight->addWidget(plot);
    for(int i=0; i<3; i++)
        subPlotHLayout->addWidget(plotMode[i]);
    subPlotHLayout->addWidget(intervalBox);
    subPlotHLayout->addWidget(updateOnTime);


    textSend = new QPlainTextEdit;
    //textSend->setFixedHeight(30);
    //textSend->setFixedWidth(200);

    textEdit = new QPlainTextEdit;
    //textEdit->setFixedWidth(260);

    //sendButton = new QPushButton("Send");
    //sendButton->setFixedWidth(260);
    //HoldButton = new QPushButton("Hold");
    //HoldButton->setFixedWidth(260);

    HLayout = new QHBoxLayout();
    GridLayoutLeft = new QGridLayout();



    VLayoutRight->addLayout(subPlotHLayout);

    GridLayoutLeft->addWidget(LAuto, 0,0);
    GridLayoutLeft->addWidget(LHold, 0,1);

    GridLayoutLeft->addWidget(LCDBig, 1,0,1,5);
    GridLayoutLeft->addWidget(LUnit, 1,5);

    GridLayoutLeft->addWidget(BRange, 2,0);
    GridLayoutLeft->addWidget(BHold, 2,1);
    GridLayoutLeft->addWidget(BRel, 2,2);
    GridLayoutLeft->addWidget(BMax, 2,3);
    GridLayoutLeft->addWidget(BCap, 2,4);
    GridLayoutLeft->addWidget(BHz, 2, 5);

    //GridLayoutLeft->addWidget(checksendL, 1,1);
    //GridLayoutLeft->addWidget(checkSendN, 1,2);
    //GridLayoutLeft->addWidget(textSend, 1,0);
    //GridLayoutLeft->addWidget(sendButton, 2,0, 1,3);
    //GridLayoutLeft->addWidget(HoldButton, 3,0);
    GridLayoutLeft->addWidget(textEdit, 3,0, 1,6);

    HLayout->addLayout(GridLayoutLeft);
    HLayout->addLayout(VLayoutRight);

    centralWidget()->setLayout(HLayout);
    setWindowTitle(tr("UART Terminal & Graph"));

    connect(BRange, SIGNAL(clicked()), this, SLOT(serialWriteCommand()));
    connect(BHold, SIGNAL(clicked()), this, SLOT(serialWriteCommand()));
    connect(BRel, SIGNAL(clicked()), this, SLOT(serialWriteCommand()));
    connect(BCap, SIGNAL(clicked()), this, SLOT(serialWriteCommand()));
    connect(BMax, SIGNAL(clicked()), this, SLOT(serialWriteCommand()));
    connect(BHz, SIGNAL(clicked()), this, SLOT(serialWriteCommand()));

    connect(plotMode[0], SIGNAL(pressed()), plot, SLOT(setScaleMode()));
    connect(plotMode[1], SIGNAL(pressed()), plot, SLOT(setScaleMode()));
    connect(plotMode[2], SIGNAL(pressed()), plot, SLOT(setScaleMode()));
    connect(intervalBox, SIGNAL(valueChanged(int)), plot, SLOT(intervalChanged(int)));
    connect(updateOnTime, SIGNAL(clicked(bool)), plot, SLOT(onUpdateOnTime(bool)));
    //connect(updateOnTime, SIGNAL(clicked(bool)), this, SLOT(onUpdateOnTime(bool)));

    this->resize(800,400);
}


void MainWindow::about()
{
    QMessageBox::about(this, tr("About Application"),
        tr("<b>QSerialDevice and QWT for HabraHabr</b> - example of simple terminal<p> Created by Alex Alexeev. 2011"));
}


