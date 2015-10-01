#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QtCore/QDateTime>
#include <qwidget.h>
#include <QMainWindow>

#include <qwt_plot.h>

#include <QPlainTextEdit>
#include <QComboBox>
#include <QScrollBar>
#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLCDNumber>
#include <QTimer>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>

#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>


namespace Ui {
    class MainWindow;
}


class Plot;


class MainWindow : public QMainWindow
{
    Q_OBJECT
signals:

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public Q_SLOTS:
    void procEnumerate();

    void serialDataReceive();
    void serialErrorHandler(QSerialPort::SerialPortError error);
    void serialWriteData();
    void serialWriteCommand();

    void refreshScanRate(int);

    void UT70_GetData(QByteArray array);

    void RecToFile(QString str);

    // Toolbar buttons click
    void openPortButtonClick();
    void about();

private:
    Ui::MainWindow *ui;

    QSerialPort *serial;

    void initMainWidgetCloseState();
    void initMainWidgetOpenState();

    void initButtonConnections();
    void initBoxConnections();

    void createActions();
    void createMenus();
    void createToolBars();
    void createCentralWidget();

    QMenu *helpMenu;

    QPlainTextEdit *textEdit, *textSend;

    QCheckBox *checkSendN, *checksendL;

    QAction *aboutqtAct;
    QAction *aboutAct;
    QAction *refreshButton;
    QAction *controlButton;
    QCheckBox *record;

    //QPushButton *sendButton;
    //QPushButton *HoldButton;
    QComboBox *portBox;
    QStatusBar *statusBar;
    Plot *plot;
    QHBoxLayout *subPlotHLayout;

    QLCDNumber *LCDBig;
    QLabel *LAuto, *LHold;
    bool BHzActive, BDuty;
    QLabel *LUnit;
    QPushButton *BRange, *BHold, *BRel, *BMax, *BCap, *BHz;

    QGridLayout *GridLayoutLeft;
    QVBoxLayout *VLayoutRight;
    QHBoxLayout *HLayout;

    QByteArray dataArray;

    int disconnectTimer;

    QRadioButton *plotMode[3];
    QSpinBox *intervalBox, *refresh;
    QCheckBox *updateOnTime;


    QTimer *timer;
};

#endif // MAINWINDOW_H
