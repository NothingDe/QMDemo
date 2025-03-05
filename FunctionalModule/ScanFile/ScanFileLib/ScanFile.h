#ifndef SCANFILE_H
#define SCANFILE_H

#ifdef SCANFILE_EXPORTS
#define SCANFILE_API __declspec(dllexport)   // 使用DLL导出符号
#else
#define SCANFILE_API __declspec(dllimport)   // 使用DLL导入符号
#endif

#include "ThreadPool.h"
#include "ThreadsafeQueue.h"
#include <functional>
#include <string>
#include <windows.h>

struct FileInfo
{
    std::string fileName;
    ULONGLONG size;
};

using MessageCall = std::function<void(const FileInfo&)>;   // 定义回调函数类型

class SCANFILE_API ScanFile
{
public:
    ScanFile();
    ~ScanFile();
    void setCallback(MessageCall callback);   // 设置回调函数
    void scan(const std::string& path);       // 执行扫描操作
    void stop();                              // 停止扫描操作

private:
    LONGLONG getFileSize(const std::string& path);   // 获取文件大小
    bool isDirectory(const std::string& path);       // 判断是否为目录

    void scanPath(std::string path);   // 扫描路径
    std::string LPCWSTRToString(const LPCWSTR wstr); // LPCWSTR转字符串
    std::wstring stringToLPCWSTR(const std::string& str); // 字符串转wstring


private:
    MessageCall m_callback;   // 回调函数类型定义
    ThreadPool* m_threadPool = nullptr;
    bool m_quit = false;
};

#endif   // SCANFILE_H
