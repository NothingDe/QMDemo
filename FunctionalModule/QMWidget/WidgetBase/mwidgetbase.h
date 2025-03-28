﻿/******************************************************************************
 * @文件名       mwidgetbase.h
 * @功能        自定义widget窗口基类
 *
 * @开发者     mhf
 * @时间       2022/03/22
 * @备注
 *****************************************************************************/
#ifndef MWIDGETBASE_H
#define MWIDGETBASE_H

#include <qpushbutton.h>
#include <QWidget>

class MWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit MWidgetBase(QWidget* parent = nullptr);
    ~MWidgetBase();
    void setTitleBar(QWidget* titleBar);   // 传入窗口的自定义标题栏

private:
    void init();                                           // 初始化
    void thisEvent(QObject* watched, QEvent* event);       // 当前窗口事件处理
    void titleBarEvent(QObject* watched, QEvent* event);   // 标题栏事件处理

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    bool winMouseEvent(MSG* msg, qintptr* result);   // 鼠标事件处理
#else
    bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
    bool winMouseEvent(MSG* msg, long* result);   // 鼠标事件处理
#endif
signals:
    /**
     * @brief               窗口最大化、最小化、还原时发出
     * @param windowStates
     */
    void windowStateChanged(Qt::WindowStates windowStates);

private:
    QWidget* m_titleBar = nullptr;   // 标题栏
};

#endif   // MWIDGETBASE_H
