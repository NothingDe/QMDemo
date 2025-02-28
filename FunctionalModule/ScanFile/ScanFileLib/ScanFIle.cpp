#include "ScanFile.h"

#include <iostream>
#include <string>
#include <memory>

using namespace std;

ScanFile::ScanFile()
{
    m_threadPool = new ThreadPool();
}

ScanFile::~ScanFile()
{
    if (m_threadPool)
    {
        delete m_threadPool;
        m_threadPool = nullptr;
    }

    // 清理回调函数指针
    m_callback = nullptr;
}

/**
 * @brief 设置回调函数
 *
 * 将回调函数赋值给成员变量 m_callback，以便在需要时调用。
 *
 * @param callback 回调函数，类型为 MessageCall
 */
void ScanFile::setCallback(MessageCall callback)
{
    m_callback = callback;
}

/**
 * @brief 判断给定路径是否为目录
 *
 * 根据给定的文件路径，判断该路径是否指向一个目录。
 *
 * @param path 文件路径
 * @return 如果路径指向一个目录，则返回true；否则返回false
 */
bool ScanFile::isDirectory(const std::string& path)
{
    if (path.empty())
        return false;   // 空路径不认为是目录

    // 获取文件属性信息
    DWORD fileAttributes = GetFileAttributesA(path.c_str());   // 获取文件属性信息

    // 检查文件属性是否表示目录
    // INVALID_FILE_ATTRIBUTES 表示路径无效，例如文件不存在或无法访问。
    // FILE_ATTRIBUTE_DIRECTORY 表示路径是一个目录。
    return (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

/**
 * @brief 扫描文件
 *
 * 根据提供的文件路径调用回调函数，传递文件路径和文件大小作为参数。
 *
 * @param path 要扫描的文件路径
 */
void ScanFile::scan(const std::string& path)
{
    if (!m_callback)
        return;   // 如果回调函数未设置，则直接返回

    m_quit = false;
    m_threadPool->enqueue(std::bind(&ScanFile::scanPath, this, path));
}


/**
 * @brief        LPCWSTR转string
 * @param wstr
 * @return
 */
std::string ScanFile::LPCWSTRToString(const LPCWSTR wstr)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    char* str = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, nullptr, nullptr);
    std::string result(str);
    delete[] str;
    return result;
}

/**
 * @brief 将 std::string 类型的字符串转换为 std::wstring 类型的宽字符字符串
 *
 * 将输入的 std::string 类型的字符串转换为 UTF-16 编码的 std::wstring 类型的宽字符字符串。
 *
 * @param str 要转换的 std::string 类型的字符串
 * @return 转换后的 std::wstring 类型的宽字符字符串
 */
std::wstring ScanFile::stringToLPCWSTR(const std::string& str) {
    if (str.empty()) return std::wstring();

    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0); // 获取宽字符数组的长度
    std::wstring wstrTo(len, 0); // 创建宽字符数组
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], len); // 转换字符串到宽字符数组
    return wstrTo;
}

/**
 * @brief 扫描指定路径下的所有文件和目录
 * @param path 要扫描的路径
 */
void ScanFile::scanPath(string path)
{
    std::string searchPath = path + "/*";   // 构建搜索路径，包括子目录的通配符
    WIN32_FIND_DATAW findFileData;          // 用于存储文件信息的结构体
    std::wstring wstr = stringToLPCWSTR(searchPath); // 将路径转换为宽字符串形式，以便与WIN32 API兼容
    HANDLE hFind = FindFirstFileW(wstr.c_str(), &findFileData);   // 开始搜索文件

    if (hFind == INVALID_HANDLE_VALUE)   // 如果查找失败，则退出函数
    {
        std::cerr << "FindFirstFile failed (" << GetLastError() << ")" << searchPath.c_str() << std::endl;
        return;
    }

    do
    {
        std::string name = LPCWSTRToString(findFileData.cFileName);
    // std::cout << "name = " << name << std::endl;
        // 跳过当前目录(.)和上级目录(..)
        if ((strcmp(name.c_str(), ".") != 0) && (strcmp(name.c_str(), "..") != 0))
        {
            std::string filePath = path + "/" + name;
            // 如果是目录，则加入目录队列，否则加入文件队列
            // dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY 判断是否为目录，此处用的是WIN32 API的标志位。
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (m_quit) // 如果需要退出，则直接返回
                {
                    break;
                }
                // 在线程池执行目录扫描任务
                m_threadPool->enqueue(std::bind(&ScanFile::scanPath, this, filePath));
            }
            else
            {
                // 计算文件大小
                ULARGE_INTEGER fileSize;
                fileSize.LowPart = findFileData.nFileSizeLow;
                fileSize.HighPart = findFileData.nFileSizeHigh;

                if (m_callback)
                {
                    m_callback({filePath, fileSize.QuadPart});
                }
            }
        }

    } while (FindNextFileW(hFind, &findFileData));   // 如果有下一个文件，则继续循环

    // 检查FindNextFile是否失败
    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        std::cerr << "FindNextFile failed (" << GetLastError() << ")" << std::endl;
    }

    // 关闭查找句柄
    FindClose(hFind);
}

/**
 * @brief 停止扫描文件操作
 *
 * 该函数用于停止当前正在进行的文件扫描操作。
 *
 * 首先，将成员变量 m_quit 设置为 true，表示需要停止扫描。
 * 然后，如果成员变量 m_threadPool 不为空，则调用其 quit 方法停止线程池。
 */
void ScanFile::stop()
{
    m_quit = true;
    if (m_threadPool)
    {
        m_threadPool->quit();
    }
}
    