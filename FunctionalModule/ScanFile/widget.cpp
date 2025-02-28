#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QFileDialog>

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_scanFile.setCallback(std::bind(&Widget::fun, this, std::placeholders::_1));
    connect(&m_timer, &QTimer::timeout, this, &Widget::on_timeout);
}

Widget::~Widget()
{
    m_timer.stop();
    delete ui;
}

void Widget::fun(const FileInfo& fileInfo)
{
    std::lock_guard<SpinLock> lock(m_spinLock);
    m_strFileInfo += QString("%1  [%2字节]\n").arg(fileInfo.fileName.c_str()).arg(fileInfo.size);
    m_size += fileInfo.size;
}

void Widget::on_timeout()
{
    std::lock_guard<SpinLock> lock(m_spinLock);
    if (m_strFileInfo.isEmpty())
    {
        return;
    }
    ui->plainTextEdit->appendPlainText(m_strFileInfo);
    m_strFileInfo.clear();
    ui->label_size->setText(QString("总大小：%1字节").arg(m_size));
}

void Widget::on_pushButton_clicked(bool checked)
{
    if (checked)
    {
        QString path = QFileDialog::getExistingDirectory(this, "", "./");
        if (path.isEmpty())
        {
            return;
        }
        m_size = 0;
        m_strFileInfo.clear();
        ui->lineEdit->setText(path);
        m_scanFile.scan(path.toStdString());
        m_timer.start(50);
    }
    else
    {
        m_scanFile.stop();
        m_timer.stop();
    }
}
