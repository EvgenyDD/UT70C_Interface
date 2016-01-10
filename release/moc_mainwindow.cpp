/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[16];
    char stringdata[208];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10),
QT_MOC_LITERAL(1, 11, 13),
QT_MOC_LITERAL(2, 25, 0),
QT_MOC_LITERAL(3, 26, 17),
QT_MOC_LITERAL(4, 44, 18),
QT_MOC_LITERAL(5, 63, 28),
QT_MOC_LITERAL(6, 92, 5),
QT_MOC_LITERAL(7, 98, 15),
QT_MOC_LITERAL(8, 114, 18),
QT_MOC_LITERAL(9, 133, 15),
QT_MOC_LITERAL(10, 149, 12),
QT_MOC_LITERAL(11, 162, 5),
QT_MOC_LITERAL(12, 168, 9),
QT_MOC_LITERAL(13, 178, 3),
QT_MOC_LITERAL(14, 182, 19),
QT_MOC_LITERAL(15, 202, 5)
    },
    "MainWindow\0procEnumerate\0\0serialDataReceive\0"
    "serialErrorHandler\0QSerialPort::SerialPortError\0"
    "error\0serialWriteData\0serialWriteCommand\0"
    "refreshScanRate\0UT70_GetData\0array\0"
    "RecToFile\0str\0openPortButtonClick\0"
    "about"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x0a /* Public */,
       3,    0,   65,    2, 0x0a /* Public */,
       4,    1,   66,    2, 0x0a /* Public */,
       7,    0,   69,    2, 0x0a /* Public */,
       8,    0,   70,    2, 0x0a /* Public */,
       9,    1,   71,    2, 0x0a /* Public */,
      10,    1,   74,    2, 0x0a /* Public */,
      12,    1,   77,    2, 0x0a /* Public */,
      14,    0,   80,    2, 0x0a /* Public */,
      15,    0,   81,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QByteArray,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->procEnumerate(); break;
        case 1: _t->serialDataReceive(); break;
        case 2: _t->serialErrorHandler((*reinterpret_cast< QSerialPort::SerialPortError(*)>(_a[1]))); break;
        case 3: _t->serialWriteData(); break;
        case 4: _t->serialWriteCommand(); break;
        case 5: _t->refreshScanRate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->UT70_GetData((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 7: _t->RecToFile((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: _t->openPortButtonClick(); break;
        case 9: _t->about(); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, 0, 0}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
