#include "My_structure.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <limits>
#include <algorithm>
#include <locale>
#include <functional>
#include <codecvt> //用于转换编码
#include <cmath>
#include <iostream>
#include <regex> //正则表达式
using namespace std;
// 实现加载进度条功能
void showProgress(size_t current, size_t total)
{
    int barWidth = 50;                       // 进度条的宽度
    float progress = (float)current / total; // 当前进度

    wcout << L"[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos)
            wcout << L"=";
        else if (i == pos)
            wcout << L">";
        else
            wcout << L" ";
    }
    wcout << L"] " << int(progress * 100.0) << L"%\r"; // 回车符让进度条更新而不换行
    wcout.flush();                                     // 确保输出立即显示
}

// 加载词典成一棵前缀树
bool load_dict(const wstring &path, Prefix_tree &tree)
{
    try
    {
        // 使用 UTF-8 编码转换器将 wstring 转换为 string
        wstring_convert<codecvt_utf8<wchar_t>> conv;
        string path_str = conv.to_bytes(path);

        // 打开普通的 ifstream
        ifstream file(path_str, ios::binary);
        if (!file.is_open())
        {
            wcerr << L"无法打开文件: " << path << endl;
            return false;
        }

        wcout << L"成功打开文件: " << path << endl;
        file.seekg(0, ios::end);
        size_t totalSize = file.tellg();
        wcout << totalSize << endl;
        file.seekg(0, ios::beg);

        string line;
        int line_number = 0;
        size_t bytesRead = 0;
        while (getline(file, line))
        {
            bytesRead = file.tellg();
            if (bytesRead == static_cast<size_t>(-1))
            {
                // 如果 tellg() 返回 -1，可能是文件末尾
                bytesRead = totalSize;
            }
            showProgress(bytesRead, totalSize);
            line_number++;

            // 将读取的 UTF-8 string 转换为 wstring
            wstring wline = conv.from_bytes(line);

            wstringstream ss(wline);

            wstring word, symbol;
            int weight;

            // 假设每行格式为 "汉字 权重 标识符"
            if (!(ss >> word >> weight >> symbol))
            {
                wcerr << L"解析行失败 (第 " << line_number << L" 行): " << wline << endl;
                continue; // 跳过格式错误的行
            }

            // 将解析结果添加到前缀树中
            tree.add_word(word, weight);
        }

        file.close();
        return true;
    }
    catch (const exception &e)
    {
        wcerr << L"发生异常: " << e.what() << endl;
        return false;
    }
}

// 最短路径分词
vector<wstring> tokenizer(wstring &sentence, const Prefix_tree &tree)
{
    size_t length = sentence.size();
    vector<int> min_weight(length, INT32_MAX);
    vector<int> split_pos(length, -1);
    size_t pos = 0;
    while (pos < length)
    {
        int record = INT32_MAX;
        size_t re_index = pos;
        for (size_t t = pos; t < length; t++)
        {
            wstring sub = sentence.substr(pos, t - pos + 1);
            int weight = tree.get_weight(sub);
            if (weight != -1)
            {
                weight = (int)log(weight);
                int tw;
                tw = (pos == 0) ? weight : min_weight[pos - 1] + weight;

                if (tw < min_weight[t])
                {
                    min_weight[t] = tw;
                    split_pos[t] = pos;
                }
                if (record > min_weight[t])
                {
                    record = min_weight[t];
                    re_index = t;
                }
            }
        }
        pos = re_index + 1;
    }
    vector<wstring> ans;
    int end = length - 1;
    while (end >= 0)
    {
        int start = split_pos[end];
        if (start == -1)
        {
            start = end;
        }
        ans.push_back(sentence.substr(start, end - start + 1));
        if (start == 0)
            break;
        end = start - 1;
    }
    reverse(ans.begin(), ans.end());
    return ans;
}

// n最短路径分词
vector<vector<wstring>> n_shortest_paths_tokenizer(wstring &sentence, Prefix_tree &tree, int n)
{
    // 用于记录路径
    struct weight_log
    {
        int prior_ver;      // 上一个切分点
        weight_log *former; // 对应上一个路径记录点
        int weight;         // 当前总权值
        weight_log(int a, weight_log *b, int c) : prior_ver(a), former(b), weight(c) {}
    };

    size_t length = sentence.size();

    // split_pos[i]：保存“到达位置 i 的候选路径”(最多保留 n 个)
    vector<vector<weight_log>> split_pos(length + 1);

    // 初始化：在位置 0 推一个权值=0 的“虚拟”切分记录
    // 表示从下标 0 开始，尚未选任何词，权值=0
    split_pos[0].push_back(weight_log(-2, /*path_id*/ 0, /*weight*/ 0));

    // 遍历每个起始位置
    for (size_t pos = 0; pos < length; ++pos)
    {
        // 如果这个位置根本还没有可行的切分路径，就跳过
        if (split_pos[pos].empty())
            continue;

        // 遍历所有可行的下一个切分点 [pos+1, length]
        for (size_t t = pos + 1; t <= length; ++t)
        {
            // 取子串
            wstring sub = sentence.substr(pos, t - pos);
            int weight = (tree.get_weight(sub));

            // 如果这个子串不是合法词，跳过
            if (weight == -1)
                continue;
            else
                weight = (int)log(weight);
            int index = 0;
            // 当前 pos 有多少条候选路径，就逐一延伸
            for (auto &cand : split_pos[pos])
            {
                // 计算新路径权值
                // cand.weight 已经包含“到达 pos 的最优代价”
                int tw = cand.weight + weight;

                // 创建新的记录
                weight_log new_log(
                    /*prior_ver*/ (int)pos,
                    /*path_id*/ &split_pos[pos][index],
                    /*weight*/ tw);

                // 推到 split_pos[t]
                split_pos[t].push_back(new_log);
                if (split_pos[t].size() > n)
                {
                    sort(split_pos[t].begin(), split_pos[t].end(),
                         [](const weight_log &a, const weight_log &b)
                         {
                             return a.weight < b.weight;
                         });
                    split_pos[t].pop_back();
                }
                index++;
            }

            // 只在需要的时候再进行“保留前 n 个”处理
            // 为了效率，可以先插入完本轮再做一次排序/截断
        }

        // 这里不是必须对 split_pos[pos] 再排序，因为已经用不到它了
        // 但可以保留以防内存暴涨。如果确实需要多轮插入，可在此处截断
    }
    // 将 split_pos[length] 按 weight 升序排序
    sort(split_pos[length].begin(), split_pos[length].end(),
         [](const weight_log &a, const weight_log &b)
         {
             return a.weight < b.weight;
         });

    // 回溯出 n 条最优路径（如果有）
    // 这里的回溯逻辑只是示例，还需根据你的需要修正
    vector<vector<wstring>> result;

    // 最多取前 n 个
    for (int i = 0; i < n && i < (int)split_pos[length].size(); ++i)
    {
        // 取出第 i 条最优路径
        weight_log cur = split_pos[length][i];
        vector<wstring> path;

        // 从末尾回溯
        int end = (int)length;
        while (end > 0 && cur.prior_ver >= 0)
        {
            int start = cur.prior_ver;
            path.push_back(sentence.substr(start, end - start));
            cur = *cur.former;
            end = start;
        }

        // 整个 path 是从后往前收集的
        reverse(path.begin(), path.end());
        result.push_back(path);
    }

    return result;
}

// 将中文文本按照标点分词
vector<wstring> splitByPunctuation(const wstring &sentence)
{
    // 定义中文标点符号的正则表达式
    wregex punctuation_regex(L"[\u3000-\u303F\uFF00-\uFFEF\u2000-\u206F]+|[\u0021-\u002F\u003A-\u0040\u005B-\u0060\u007B-\u007E]+");
    wsregex_token_iterator iter(sentence.begin(), sentence.end(), punctuation_regex, {-1});
    wsregex_token_iterator end;

    // 将分割结果保存到向量中
    vector<wstring> result;
    while (iter != end)
    {
        if (!iter->str().empty())
        { // 跳过空结果
            result.push_back(iter->str());
        }
        ++iter;
    }
    // for (auto string : result)
    // {
    //     wcout << string << endl;
    // }
    return result;
}

int main()
{
    // 设置全局本地化环境以支持宽字符
    locale::global(locale(""));
    wcout.imbue(locale());
    wcin.imbue(locale());

    Prefix_tree tree;
    wstring path;

    // 读取词典路径
    wcout << L"请输入词典地址：";
    wcin >> path;

    // 加载字典文件
    if (load_dict(path, tree))
    {
        wcout << endl
              << L"字典加载成功!" << endl;
    }
    else
    {
        wcout << endl
              << L"字典加载失败!" << endl;
    }

    int sign = 0;
    while (sign != -1)
    {
        wcout << L"请选择以下操作：" << endl;
        wcout << L" [1]: 重新选择词典 " << endl;
        wcout << L" [2]: 最短路径分词" << endl;
        wcout << L" [3]: N最短路径分词" << endl;
        wcout << L" [-1]: 退出程序" << endl;
        wcout << L"==========================================" << endl;
        wcout << L"请输入操作编号：";

        // 使用 wcin 读取操作编号
        while (!(wcin >> sign))
        {
            wcin.clear();
            wcin.ignore(numeric_limits<streamsize>::max(), '\n');
            wcout << L"无效输入，请重新输入" << endl;
        }

        switch (sign)
        {
        case 1:
        {
            tree.clear();
            wstring p;
            wcout << L"请输入词典地址：";
            wcin >> p;
            if (load_dict(p, tree))
            {
                wcout << endl
                      << L"字典重新加载成功!" << endl;
            }
            else
            {
                wcout << endl
                      << L"字典重新加载失败!" << endl;
            }
            break;
        }
        case 2:
        {
            wstring sentence;
            wcout << L"请输入字符串进行分词：";
            wcin >> sentence; // 如果需要读取包含空格的整行，建议使用 getline(wcin, sentence);
            vector<wstring> ans = tokenizer(sentence, tree);
            for (const wstring &s : ans)
            {
                wcout << s << L"\\";
            }
            wcout << endl;
            break;
        }
        case 3:
        {
            int n;
            wcout << L"请输入N：";
            wcin >> n; // 如果需要读取包含空格的整行，建议使用 getline(wcin, sentence);
            wstring osentence;
            wcout << L"请输入字符串进行分词：";
            wcin >> osentence;
            vector<wstring> sentences = splitByPunctuation(osentence);
            for (wstring sentence : sentences)
            {
                vector<vector<wstring>> ans = n_shortest_paths_tokenizer(sentence, tree, n);
                for (vector<wstring> a : ans)
                {
                    for (const wstring &s : a)
                    {
                        wcout << s << L"\\";
                    }
                    wcout << endl;
                }
                wcout << endl;
            }
            break;
        }
        case -1:
            wcout << L"退出程序。" << endl;
            break;
        default:
            wcout << L"无效的操作编号，请重新选择。" << endl;
            break;
        }
    }

    return 0;
}
