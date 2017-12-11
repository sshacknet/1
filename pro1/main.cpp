// main.cpp: 定义控制台应用程序的入口点。
//


#include "HTMLparser.h"
#include "cxxopts.hpp"
#include "StringHashTable.h"
#include "CSVparser.hpp"
#include <future>
#include <thread>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <codecvt>
#include <locale>
#include <fstream>
#define _CRT_SECURE_NO_WARNINGS
#define CPU_CORES 4 //解析网页的线程数，与cpu线程数相同
using namespace std;
StringHashTable initDictionary();//加载词典
LinkList<String> divideWords(String words, StringHashTable dic);//分词
void Init();//全局初始化
void process();//主要的处理过程
void cURL(string url, string outfile);//文件下载
LinkList<PageInfo> extractInfo(int pagenumbers);//解析网页结构
void setoption(int argc, char* argv[]);//解析命令行参数

/*
* 用于字符编码转换
*/
extern const std::locale empty_locale = std::locale::empty();
typedef std::codecvt_utf8<wchar_t> converter_type; //std::codecvt_utf16
extern const converter_type* converter = new converter_type;
extern const std::locale utf8_locale = std::locale(empty_locale, converter);

// 全局属性
bool DOWNLOAD_CLOUD_DIC = true;//是否加载云词典
bool DOWNLOAD_HTML = false;//是否下载网页文件
bool DEBUG_MODE = false;//输出调试信息
bool POWERSHELL_CURL = false;//是否使用powershell中的curl(Invoke-WebRequest)下载网页

int main(int argc, char* argv[])
{
    setoption(argc, argv);
    setlocale(LC_ALL, "chs");
    Init();
    process();
    cout << "处理完成" << endl;
    getchar();
    return 0;
}


void Init()
{
    if (DOWNLOAD_CLOUD_DIC)
    {
        //获取云词典
        cURL("http://sync.chenyijie.me/dictionary/cloud_dictionary.dic", "dictionary/cloud_dictionary.dic");
    }

    //创建result.csv文件
    system("@mkdir output >> error.log");
    FILE* fp;
    fp = fopen("output/result.csv", "w");
    char szBOM[4] = {(char)0xEF,(char)0xBB,(char)0xBF,0};
    fprintf(fp, "%s", szBOM);
    fclose(fp);
    // 输出bom头

    wofstream result("output/result.csv", ios::app);
    result.imbue(utf8_locale);
    wstring header(L"\"序号\",\"网址\",\"发帖大类\",\"发帖小类\",\"发帖标题\",\"发帖内容\",\"发帖人\",\"发帖日期\",\"发帖类型\",\"分词结果\"");
    result << header << endl;
    result.close();
    //输出结果的表格栏
}


void setoption(int argc, char* argv[])
{
    cxxopts::Options options("main.exe", "Course Project for DataStructure and Algorithm");
    options.add_options("1")
        ("S,skip", "Skip Download Cloud Dictionary")
        ("H,help", "Get Help of Options")
        ("D,debug", "Opening debug mode")
        ("P,powershell", "Using curl.exe instead of powershell curl")
        ("N,notdownloadhtml", "Skip download html file");
    options.parse(argc, argv);
    if (options.count("help"))
    {
        std::cout << options.help({"","1"});
        exit(0);
    }
    if (options.count("skip"))
        DOWNLOAD_CLOUD_DIC = false;
    if (options.count("powershell"))
        POWERSHELL_CURL = false;
    if (options.count("debug"))
        DEBUG_MODE = true;
    if (options.count("notdownloadhtml"))
        DOWNLOAD_HTML = false;
}

/**
* \brief 从三个词典文件加载词典，对于本地的两个词典，若不能打开则会 throw 错误
* \return 存储词典的hashtable
*/
StringHashTable initDictionary()
{
    StringHashTable strhashtable;


    std::wifstream dic_fin("dictionary/dictionary.dic");
    dic_fin.imbue(utf8_locale);


    int total = 0;
    if (dic_fin)
    {
        wstring temp;
        while (getline(dic_fin, temp))
        {
            total++;
            if (total % 1000 == 0)
            {
                cout << "\r Loading Dictionary... [" << total << "] words";
            }
            String tmp(temp);
            strhashtable.insert(tmp);
        }
    }
    else
    {
        cout << "error";
        throw Error("fail to load dictionary");
    }
    std::wifstream extra_dic_fin("dictionary/extra_dictionary.dic");
    extra_dic_fin.imbue(utf8_locale);
    if (extra_dic_fin)
    {
        wstring temp;
        while (getline(extra_dic_fin, temp))
        {
            total++;
            if (total % 100 == 0)
            {
                cout << "\r Loading Extra Dictionary... [" << total << "] words";
            }
            String tmp(temp);
            strhashtable.insert(tmp);
        }
    }
    else
    {
        cout << "error";
        throw Error("fail to load extra dictionary");
    }

    if (DOWNLOAD_CLOUD_DIC)
    {
        std::wifstream cloud_dic_fin("dictionary/cloud_dictionary.dic");
        cloud_dic_fin.imbue(utf8_locale);
        if (cloud_dic_fin)
        {
            wstring temp;
            while (getline(cloud_dic_fin, temp))
            {
                total++;
                if (total % 100 == 0)
                {
                    cout << "\r Loading Cloud Dictionary... [" << total << "] words";
                }
                String tmp(temp);
                strhashtable.insert(tmp);
            }
        }
        else
        {
            cout << "error\n";
            cout << "fail to load cloud dictionary";
        }
    }
    return strhashtable;
}

/**
* \brief 使用给定的词典及最大正向匹配法进行分词
* \param words 待分词的字符串
* \param dic 字典
* \return
*/
LinkList<String> divideWords(String words, StringHashTable dic)
{
    LinkList<String> tempresult;  
    words = removeChineseSymbol(words);
    if (words.empty())
        return tempresult;

    if (words.size() == 1)
    {
        tempresult.push_back(words);
        return tempresult;
    }
    String currentstring(words);
    while (!currentstring.empty())
    {
        int i;
        String substr;
        int break_flag = 0;
        for (i = currentstring.size(); i > 0; i--)
        {
            substr = currentstring.substr(0, i);
            if (dic[substr])
            {
                tempresult.insert(substr);
                if (i != currentstring.size())
                {
                    currentstring = currentstring.substr(i, currentstring.size());
                    break;
                }
                else
                {
                    break_flag = 1;

                    break;
                }
            }
        }
        if (break_flag == 1)
        {
            break;
        }
        if (i == 0)
        {
            substr = currentstring.substr(0, 1);

            tempresult.insert(substr);
            currentstring = currentstring.substr(1, currentstring.size());
        }
        if (currentstring.size() == 1)
        {
            break;
        }
    }
    LinkList<String> result;
    auto current = tempresult.head;
    String numbers;
    while (current)
    {
        if (current->_data.size() >= 2)
        {
            if (!numbers.empty())
            {
                result.push_back(numbers);
                numbers = String();
            }
            result.push_back(current->_data);
        }
        else
        {
            if (is_letter_or_number(current->_data[0]))
            {
                numbers = numbers.concat(current->_data);
            }
        }
        current = current->_next;
    }
    if (!numbers.empty())
    {
        result.push_back(numbers);
    }


    return result;
}


void process()
{
    try
    {
        system("@mkdir temp > error.log");
        csv::Parser parser("input/url.csv");
        int pagenumbers = parser.rowCount();
        vector<string> urls;
        if (DOWNLOAD_HTML)
        {
            cout << "开始下载网页，请耐心等待约5分钟" << endl;
        }
        for (int i = 0; i < pagenumbers; i++)
        {
            std::ostringstream stringStream;
            stringStream << "temp/" << i << ".html";
            string url = parser[i][1].substr(1, parser[i][1].size() - 2);
            urls.push_back(url);

            if (DOWNLOAD_HTML)
            {
                thread th(cURL, url, stringStream.str());
                th.detach();//线程开太多了
            }
                //cURL(url, stringStream.str());
        }
        //* TODO 
        //下载网页并保存

        const LinkList<PageInfo> infos = extractInfo(pagenumbers);
        //解析网页

        const StringHashTable dic = initDictionary();

        wofstream result("output/result.csv", ios::app);
        result.imbue(utf8_locale);

        //写入文件保存
        auto current = infos.head;
        int num = 0;
        while (current)
        {
            num++;
            //L"\"序号\",\"网址\",\"发帖大类\",\"发帖小类\",\"发帖标题\",\"发帖内容\",\"发帖人\",\"发帖日期\",\"发帖类型\",\"分词结果\"");
            result << num << ',' << urls[num].c_str() << ",\"" << current->_data.category.c_str() << "\","
                << "\"" << current->_data.subcategory.c_str() << L"\","
                << "\"" << current->_data.title.c_str() << L"\","
                << "\"" << current->_data.content.c_str() << L"\","
                << "\"" << current->_data.author.c_str() << L"\","
                << "\"" << current->_data.time.c_str() << L"\","
                << "\"" << current->_data.type.c_str() << L"\",";

            result << "\"";
            const LinkList<String> content_wordsresult = divideWords(current->_data.content, dic);
            const LinkList<String> title_wordsresult = divideWords(current->_data.title, dic);

            auto c = content_wordsresult.head;
            result << L"[发帖内容分词]:";
            while (c)
            {
                result << c->_data.c_str() << "  ";
                c = c->_next;
            }
            c = title_wordsresult.head;
            result << L"[发帖标题分词]:";
            while (c)
            {
                result << c->_data.c_str() << "  ";
                c = c->_next;
            }
            result << "\"";
            result << endl;
            current = current->_next;
        }
    }
    catch (Error& e)
    {
        e.what();
    }
}

PageInfo extractInfo_onepage(string filename)
{
    HTMLparser parser(filename);
    parser.toknize();
    return parser.parse();
}


LinkList<PageInfo> extractInfo(int pagenumbers)
{
    LinkList<PageInfo> result;
    for (int i = 0; i < pagenumbers; i += CPU_CORES)
    {
        //        cout << "正在处理第" << i + 1 << "个网页" << endl;
        //        HTMLparser htmlparser(sstream.str());
        //        htmlparser.toknize();
        //
        //        auto pageinfo = htmlparser.parse();
        //        result.push_back(pageinfo);
        //        pageinfo.show();
        //        htmlparser.destory();
        future<PageInfo> temp[4];
        for (int j = i; j < pagenumbers && j < (i + CPU_CORES); j++)
        {
            ostringstream sstream;
            sstream << j << ".html";
            temp[j - i] = async(extractInfo_onepage, sstream.str());
        }
        for (int j = i; j < pagenumbers && j < (i + CPU_CORES); j++)
        {
            auto tempinfo = temp[j - i].get();
            result.push_back(tempinfo);
            cout << "正在处理第" << j + 1 << "个网页" << endl;
            tempinfo.show();
        }
    }
    return result;
}

void cURL(string url, string outfile)
{
    //cout << "\r正在下载:" << url;
    string cmd;
    if (POWERSHELL_CURL)
    {
        cmd = "powershell curl " + url + " -O " + outfile;
    }
    else
    {
        cmd = "curl.exe -s " + url + " -o " + outfile;
    }

    system(cmd.c_str());
}
