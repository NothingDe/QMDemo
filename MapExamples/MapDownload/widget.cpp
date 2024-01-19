#include "widget.h"
#include "ui_widget.h"

#include <QDateTime>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->resize(800, 600);
    qApp->setStyleSheet("*{font: 9pt '宋体';}");
    this->setWindowTitle(QString("QT下载瓦片地图简单示例--V%1").arg(APP_VERSION));

    m_dThread = new DownloadThread();      // 不能指定父对象
    m_dThreads = new DownloadThreads();
    connect(m_dThread, &DownloadThread::finished, this, &Widget::finished);
    connect(m_dThreads, &DownloadThreads::finished, this, &Widget::finished);

    // 默认下载范围
    ui->dspin_LTLon->setValue(103.979482);
    ui->dspin_LTLat->setValue(30.66596);
    ui->dspin_RDLon->setValue(104.137009);
    ui->dspin_RDLat->setValue(30.554068);
    // 填入ArcGis下载地图类型
    ui->com_type->addItem("NatGeo_World_Map");
    ui->com_type->addItem("USA_Topo_Maps ");
    ui->com_type->addItem("World_Imagery");
    ui->com_type->addItem("World_Physical_Map");
    ui->com_type->addItem("World_Shaded_Relief");
    ui->com_type->addItem("World_Street_Map");
    ui->com_type->addItem("World_Terrain_Base");
    ui->com_type->addItem("World_Topo_Map");
    ui->com_type->addItem("Canvas/World_Dark_Gray_Base");
    ui->com_type->addItem("Canvas/World_Dark_Gray_Reference");
    ui->com_type->addItem("Canvas/World_Light_Gray_Base");
    ui->com_type->addItem("Canvas/World_Light_Gray_Reference");
    ui->com_type->addItem("Elevation/World_Hillshade_Dark");
    ui->com_type->addItem("Elevation/World_Hillshade");
    ui->com_type->addItem("Ocean/World_Ocean_Base");
    ui->com_type->addItem("Ocean/World_Ocean_Reference");
    ui->com_type->addItem("Polar/Antarctic_Imagery");
    ui->com_type->addItem("Polar/Arctic_Imagery");
    ui->com_type->addItem("Polar/Arctic_Ocean_Base");
    ui->com_type->addItem("Polar/Arctic_Ocean_Reference");
    ui->com_type->addItem("Reference/World_Boundaries_and_Places_Alternate ");
    ui->com_type->addItem("Reference/World_Boundaries_and_Places");
    ui->com_type->addItem("Reference/World_Reference_Overlay");
    ui->com_type->addItem("Reference/World_Transportation");
    ui->com_type->addItem("Specialty/World_Navigation_Charts");

    // 填入下载格式
    ui->com_format->addItem("jpg");
    ui->com_format->addItem("png");
    ui->com_format->addItem("bmp");

    qRegisterMetaType<QList<ImageInfo>>("QList<ImageInfo>");
    qRegisterMetaType<ImageInfo>("ImageInfo");
}

Widget::~Widget()
{
    delete m_dThread;
    delete m_dThreads;
    delete ui;
}

/**
 * @brief 获取下载路径
 */
void Widget::on_but_savePath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "下载路径",
                                               qApp->applicationDirPath(),
                                               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(path.isEmpty()) return;
    ui->line_savePath->setText(path + "/map/");
}

/**
 * @brief          单线程下载瓦片地图
 * @param checked
 */
void Widget::on_but_thread_clicked(bool checked)
{
    if(checked)
    {
        m_timer.start();
        ui->progressBar->setValue(0);
        ui->but_thread->setText("停止下载");
        MapInfo info;
        info.topLeft = QPointF(ui->dspin_LTLon->value(), ui->dspin_LTLat->value());
        info.lowRight = QPointF(ui->dspin_RDLon->value(), ui->dspin_RDLat->value());
        info.z = ui->spin_z->value();
        info.type = ui->com_type->currentText();
        info.format = ui->com_format->currentText();
        getArcGisMapInfo(info, m_infos);      // 计算下载信息
        emit m_dThread->getImage(m_infos);    // 开始下载
        ui->progressBar->setMaximum(m_infos.count());
        qDebug() << "瓦片数：" << m_infos.count();
    }
    else
    {
        ui->but_thread->setText("单线程下载");
        ui->but_thread->setChecked(false);
        m_dThread->quit();
    }
}

/**
 * @brief          多线程下载瓦片地图
 * @param checked
 */
void Widget::on_but_threads_clicked(bool checked)
{
    if(checked)
    {
        m_timer.start();
        ui->progressBar->setValue(0);
        ui->but_threads->setText("停止下载");
        MapInfo info;
        info.topLeft = QPointF(ui->dspin_LTLon->value(), ui->dspin_LTLat->value());
        info.lowRight = QPointF(ui->dspin_RDLon->value(), ui->dspin_RDLat->value());
        info.z = ui->spin_z->value();
        info.type = ui->com_type->currentText();
        info.format = ui->com_format->currentText();
        getArcGisMapInfo(info, m_infos);      // 计算下载信息
        m_dThreads->getImage(m_infos);       // 开始下载
        ui->progressBar->setMaximum(m_infos.count());
        qDebug() << "瓦片数：" << m_infos.count();
    }
    else
    {
        ui->but_threads->setText("多线程下载");
        ui->but_threads->setChecked(false);
        m_dThreads->quit();
    }
}

/**
 * @brief        处理下载完成的图片
 * @param index  已经下载的索引
 */
void Widget::finished(ImageInfo info)
{
    if(!info.img.isNull())   // 下载失败图片为空
    {
        // 创建文件夹
        QString strPath = ui->line_savePath->text();
        if(strPath.isEmpty())
        {
            strPath = qApp->applicationDirPath() + "/map";   // 默认保存地址
        }
        strPath += QString("/%1/%2/").arg(info.z).arg(info.x);
        QDir dir;
        dir.mkpath(strPath);
        // 保存文件
        strPath += QString("%1.%2").arg(info.y).arg(ui->com_format->currentText());
        info.img.save(strPath);
    }

    ui->progressBar->setValue(ui->progressBar->value() + 1);
    if(ui->progressBar->value() == m_infos.count())
    {
        ui->but_thread->setChecked(false);
        ui->but_threads->setChecked(false);
        ui->but_thread->setText("单线程下载");
        ui->but_threads->setText("多线程下载");
        qDebug() << "下载时长：" << m_timer.elapsed() <<" ms";
    }
}
