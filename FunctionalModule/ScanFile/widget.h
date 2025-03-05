#ifndef WIDGET_H
#define WIDGET_H

#include "ScanFile.h"
#include <QTimer>
#include <QWidget>

QT_BEGIN_NAMESPACE

namespace Ui {
class Widget;
}

QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget* parent = nullptr);
    ~Widget();

    void fun(const FileInfo& fileInfo);
private slots:
    void on_pushButton_clicked(bool checked);

private:
    void on_timeout();

private:
    Ui::Widget* ui;
    ScanFile m_scanFile;
    quint64 m_size = 0;
    SpinLock m_spinLock;
    QString m_strFileInfo;
    QTimer m_timer;
};
#endif   // WIDGET_H
