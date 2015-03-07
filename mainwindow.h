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

protected:
    void changeEvent(QEvent *e);

public Q_SLOTS:
    void procEnumerate(/*const QStringList &l*/);
    void procSerialMessages(const QString &msg, QDateTime dt);

    void serialDataReceive();
    void serialDataSend();
    void serialErrorHandler(QSerialPort::SerialPortError error);

    void printTrace(const QByteArray &data);
    void RecToFile(QPointF point);

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
    QAction *controlButton;
    QPushButton *sendButton;
    QComboBox *portBox;
    QStatusBar *statusBar;
    Plot *plot;

    QLCDNumber *LCDBig;

    QGridLayout *GridLayoutLeft;
    QVBoxLayout *VLayoutRight;
    QHBoxLayout *HLayout;

    QByteArray dataArray;
};

#endif // MAINWINDOW_H
