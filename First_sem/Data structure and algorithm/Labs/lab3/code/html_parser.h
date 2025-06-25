#include <fstream>
#include "My_structure.h"
#include <string>
#include <set>
#include <algorithm>
#include <regex>
#include <map>
#include <curl/curl.h>
using namespace std;
struct html_node
{
    /* data */
    html_node *parent;               // 父母节点
    string tag_name;                 // 标签名
    string tag;                      // tag的html原文
    Link_list<html_node *> son_node; // 子节点
    map<string, string> attr;

    html_node(string t) : tag(""), tag_name(t), parent(nullptr), son_node(), attr() {};
    html_node() : tag(""), tag_name(""), parent(nullptr), son_node(), attr() {}
};

class Html_parser
{
private:
    html_node *root;

public:
    Html_parser() : root(nullptr) {};
    html_node *get_root() { return root; }
    bool parse(string &htmlcontent) // 解析html文本
    {
        Stack<html_node *> nodestack;
        size_t i = 0;

        while (i < htmlcontent.size())
        {
            // 跳过<!DOCTYPE>
            if (htmlcontent.substr(i, 9) == "<!DOCTYPE")
            {
                size_t doctype_end = htmlcontent.find('>', i);
                if (doctype_end != string::npos)
                {
                    i = doctype_end + 1; // 跳过 <!DOCTYPE ...> 的内容
                }
                else
                {
                    break; // 如果找不到结束符号 '>', 终止解析
                }
            }
            // 跳过注释
            if (htmlcontent.substr(i, 4) == "<!--")
            {
                size_t comment_end = htmlcontent.find("-->", i);
                if (comment_end != string::npos)
                {
                    i = comment_end + 3;
                }
                else
                {
                    break; // 注释未关闭，终止解析
                }
            }
            // 跳过 script 标签及其内容
            else if (htmlcontent.substr(i, 7) == "<script")
            {
                size_t script_end = htmlcontent.find("</script>", i);
                if (script_end != string::npos)
                {
                    i = script_end + 9; // 跳过 </script>
                }
                else
                {
                    break; // 如果找不到结束的 </script>，终止解析
                }
            }
            // 处理标签
            else if (htmlcontent[i] == '<')
            {
                size_t tag_end = htmlcontent.find('>', i);
                if (tag_end == string::npos)
                {
                    cout << "标签未闭合" << endl;
                    clear();
                    return false; // 如果找不到 '>'，退出解析
                }

                string tag = htmlcontent.substr(i, tag_end - i + 1); // 获取完整的标签字符串
                string tagname;
                parseTag(tag, tagname);                        // 提取标签名
                if (tagname[0] == '/' && !nodestack.isEmpty()) // 检测到闭合标签
                {
                    if (nodestack.top()->tag_name == tagname.substr(1))
                    {
                        nodestack.pop();
                    }
                    else
                    {
                        // 标签未闭合，打印错误信息
                        cout << "Debug: Stack top tag name = [" << nodestack.top()->tag_name << "]" << endl;
                        cout << "Debug: Closing tag = [" << tagname.substr(1) << "]" << endl;
                        cout << "There is an error in this HTML element. The closing tag should be "
                             << nodestack.top()->tag_name
                             << " , not "
                             << tagname.substr(1)
                             << "."
                             << endl;
                        clear();
                        return 0;
                    }
                    i = tag_end + 1;
                }
                else // 不是闭合标签，说明是开始标签
                {    // 创建新节点并设置其标签名和完整的标签内容
                    html_node *new_node = new html_node(tagname);
                    new_node->tag = trim_and_add_spaces(tag);
                    ParseAttr(tag, new_node->attr);

                    // 如果栈为空，这就是根节点
                    if (nodestack.isEmpty())
                    {
                        root = new_node;
                        nodestack.push(new_node);
                    }
                    else if (isSelfClosingTag(tagname)) // 如果是自闭合标签
                    {
                        new_node->parent = nodestack.top();
                        nodestack.top()->son_node.append(new_node); // 合法标签作为当前节点的子节点,但是不入栈
                    }
                    else if (tag[0] != '/') // 如果不是自闭合标签且不是结束标签，就压入栈中
                    {
                        // '''if (is_SpecialBlockTag(nodestack.top()->tag_name) && !isInlineTag(tagname)) // 特殊块级元素（<h1>-<h6>、<p>、<dt>）：只能嵌套⾏内元素
                        // {
                        //     cout << "Debug: Stack top tag name = [" << nodestack.top()->tag_name << "]" << endl;
                        //     cout << "Debug: now_tag = [" << tagname << "]" << endl;
                        //     cout << "There is an error in this HTML element.  特殊块级元素（<h1>-<h6>、<p>、<dt>）：只能嵌套⾏内元素 "
                        //          << endl;
                        //     clear();
                        //     return 0;
                        // }
                        // else if (isBlockTag(tagname))
                        // {
                        //     if (isInlineTag(nodestack.top()->tag_name) && nodestack.top()->tag_name != "a") // 除了<a>,块级元素不能嵌套在行内元素中。
                        //     {
                        //         cout << "Debug: Stack top tag name = [" << nodestack.top()->tag_name << "]" << endl;
                        //         cout << "Debug: now_tag = [" << tagname << "]" << endl;
                        //         cout << "There is an error in this HTML element. 块级元素不能嵌套在行内元素中。 "
                        //              << endl;
                        //         clear();
                        //         return 0;
                        //     }
                        // }
                        // else if (tagname == "a" && nodestack.top()->tag_name == "a") // 特殊⾏内元素（<a>）：可以嵌套块级和⾏内元素，但不能嵌套⾃身。
                        // {
                        //     cout << "There is an error in this HTML element. 特殊⾏内元素（<a>）：可以嵌套块级和⾏内元素，但不能嵌套⾃身。 "
                        //          << endl;
                        //     clear();
                        //     return 0;
                        // }

                        new_node->parent = nodestack.top();
                        nodestack.top()->son_node.append(new_node); // 合法标签作为当前节点的子节点
                        nodestack.push(new_node);
                    }

                    i = tag_end + 1; // 移动到标签结束位置的下一个字符
                }
            }
            // 处理文本节点（将文本处理为<text>标签）
            else
            {
                size_t text_end = htmlcontent.find('<', i);
                string text = htmlcontent.substr(i, text_end - i);
                text = trim_and_add_spaces(text);

                if (!text.empty() && !nodestack.isEmpty())
                {
                    html_node *textnode = new html_node("text");
                    textnode->tag = text; // 对于文本节点，tag字段中存储实际文本内容
                    textnode->parent = nodestack.top();
                    nodestack.top()->son_node.append(textnode);
                }

                i = text_end; // 跳到下一个标签位置
            }
        }
        return true;
    }
    void parseTag(const string &tagcontent, string &tagname) // 提取标签名
    {
        size_t pos = 1;
        string illegal_char = ">\n\r ";
        while (pos < tagcontent.size() && illegal_char.find(tagcontent[pos]) == string::npos)
        {
            tagname += tagcontent[pos++];
        }
        transform(tagname.begin(), tagname.end(), tagname.begin(), ::tolower);
    }
    void ParseAttr(const string &tagcontent, map<string, string> &attr)
    {
        size_t pos = tagcontent.find(' '); // 跳过标签名
        if (pos == string::npos)
            return; // 如果没有属性直接返回
        pos++;      // 跳过空格

        while (pos < tagcontent.size())
        {
            // 跳过多余空格
            while (pos < tagcontent.size() && isspace(tagcontent[pos]))
            {
                pos++;
            }

            // 解析属性名
            string name;
            while (pos < tagcontent.size() && tagcontent[pos] != '=' && tagcontent[pos] != '>' && !isspace(tagcontent[pos]))
            {
                name += tagcontent[pos++];
            }

            // 如果到达末尾或遇到关闭符号，退出
            if (pos == tagcontent.size() || tagcontent[pos] == '>')
            {
                break;
            }

            // 跳过等号
            if (tagcontent[pos] == '=')
            {
                pos++;
            }
            else
            {
                continue; // 属性名后没有等号则跳过该部分
            }

            // 跳过引号
            if (tagcontent[pos] == '"')
            {
                pos++;
            }

            // 解析属性值
            string content;
            while (pos < tagcontent.size() && tagcontent[pos] != '"')
            {
                content += tagcontent[pos++];
            }

            // 跳过结束引号
            if (pos < tagcontent.size() && tagcontent[pos] == '"')
            {
                pos++;
            }

            // 保存属性键值对
            attr[name] = content;
        }
    }
    void text(string address) // 返回对应地址的所有文本
    {
        Link_list<string> addresses;
        size_t i = 0;
        while (i < address.size())
        {
            size_t end = address.find('/', i + 1);
            if (end == string::npos)
            {
                addresses.append(address.substr(i + 1));
                break;
            }
            string ad = address.substr(i + 1, end - i - 1);
            if (ad != "html" && ad != "")
                addresses.append(ad);
            i = end;
        }
        string result;
        tree_travel(root, addresses.get_head()->next, result, "text");

        cout << result << endl;
    }
    void outerHtml(string address) // 返回对应地址的所有html代码
    {
        Link_list<string> addresses;
        size_t i = 0;
        while (i < address.size())
        {
            size_t end = address.find('/', i + 1);
            if (end == string::npos)
            {
                addresses.append(address.substr(i + 1));
                break;
            }
            string ad = address.substr(i + 1, end - i - 1);
            if (ad != "html" && ad != "")
                addresses.append(ad);
            i = end;
        }
        string result;
        tree_travel(root, addresses.get_head()->next, result, "html");

        cout << result << endl;
    }

    void tree_travel(html_node *Hnode, Node<string> *tag_name, std::string &result, string sign) // 按照地址遍历
    {

        if (!tag_name || tag_name->data == "")
        {
            if (sign == "text")
            {
                result = extract_text(root, " ");
                return;
            }
            else if (sign == "html")
            {
                result = extract_html(root, " ");
                return;
            }
        }

        Node<html_node *> *L = Hnode->son_node.get_head()->next;
        if (tag_name->next == nullptr)
        {
            while (L != nullptr)
            {
                Node<html_node *> *nownode = L;
                if (nownode->data->tag_name == tag_name->data)
                {
                    if (sign == "text")
                    {
                        std::string textcontent = extract_text(nownode->data, nownode->data->tag_name);
                        if (!textcontent.empty())
                        {
                            if (!result.empty() && result.back() != '\n')
                            {
                                result += '\n';
                            }
                            result += textcontent;
                        }
                    }
                    else if (sign == "html")
                    {
                        std::string textcontent = extract_html(nownode->data, nownode->data->tag_name);
                        if (!textcontent.empty())
                        {
                            if (!result.empty() && result.back() != '\n')
                            {
                                result += '\n';
                            }
                            result += textcontent;
                        }
                    }
                }
                L = nownode->next;
            }
            return;
        }
        while (L != nullptr)
        {
            Node<html_node *> *nownode = L;
            if (nownode->data->tag_name == tag_name->data)
            {
                tree_travel(nownode->data, tag_name->next, result, sign);
            }
            L = nownode->next;
        }
    }
    string extract_html(html_node *node, string parent_tag) // 提取标签下所有的html代码
    {
        string text;
        text += node->tag;
        if (!isInlineTag(parent_tag))
        {
            text = '\n' + text;
        }
        // 遍历子节点并收集所有文本内容
        if (!node->son_node.isEmpty())
        {
            Node<html_node *> *child = node->son_node.get_head()->next;
            while (child != nullptr)
            {
                text += extract_html(child->data, node->tag_name);
                child = child->next;
            }
        }
        if (!isSelfClosingTag(node->tag_name) && node->tag_name != "text")
        {
            text += ("</" + node->tag_name + ">");
        }
        return trim_and_add_spaces(text);
    }
    string extract_text(html_node *node, string parent_tag) // 提取表签下所有的文本
    {
        string text;
        if (node->tag_name == "text")
        {
            // 如果是文本节点，直接获取文本内容
            text = node->tag;
            text = trim_and_add_spaces(text);
            if (!isInlineTag(parent_tag) && text != " ")
            {
                text = '\n' + text;
            }
            return text;
        }

        // 遍历子节点并收集所有文本内容
        if (!node->son_node.isEmpty())
        {
            Node<html_node *> *child = node->son_node.get_head()->next;
            while (child != nullptr)
            {
                text += extract_text(child->data, node->tag_name);
                child = child->next;
            }
        }
        return trim_and_add_spaces(text);
    }

    bool parseFromURL(const string &url) // 实现爬虫，解析url返回的html文件
    {
        CURL *curl;
        CURLcode res;
        string readBuffer;

        curl = curl_easy_init();
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // 可选：查看请求的详细信息

            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
                curl_easy_cleanup(curl);
                return false;
            }

            // 解析获取到的 HTML 内容
            bool result = parse(readBuffer);
            curl_easy_cleanup(curl);
            return result;
        }
        return false;
    }

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *userp) // curl库的回调函数
    {
        size_t totalSize = size * nmemb;
        userp->append((char *)contents, totalSize); // 将数据追加到 userp 指向的字符串中
        return totalSize;
    }

    bool isSelfClosingTag(const string &tagname) // 判断是否是自闭合标签
    {
        static const set<string> selfClosingTags = {
            "area", "base", "br", "col", "embed", "hr", "img", "input", "link",
            "meta", "param", "source", "track", "wbr"};
        return selfClosingTags.find(tagname) != selfClosingTags.end();
    }

    string trim_and_add_spaces(const string &input) // 去除字符串中间多余的空白符
    {
        string result = input;

        regex re("\\s+");

        // 使用 regex_replace 替换连续的空白符为单个空格
        return regex_replace(result, re, " ");
    }

    void printTree(html_node *node, int depth = 0) // 打印某个节点下的整个html文件
    {
        if (node == nullptr)
            return;

        // 根据节点的深度来添加缩进，以展示树的层次结构

        for (int i = 0; i < depth; ++i)
        {
            cout << "  "; // 两个空格表示一级缩进
        }

        // 打印当前节点的信息
        cout << "<" << node->tag_name;
        for (const auto &pair : node->attr)
        {
            cout << " " << pair.first << "=" << pair.second;
        }
        cout << ">";

        // 如果节点是文本节点，直接输出其内容
        if (node->tag_name == "text")
        {
            cout << " " << node->tag;
        }

        cout << endl;

        // 递归地打印子节点
        if (!node->son_node.isEmpty())
        {
            Node<html_node *> *child = node->son_node.get_head()->next;
            while (child != nullptr)
            {
                printTree(child->data, depth + 1); // 深度加一，表示子节点
                child = child->next;
            }
        }
        // 打印结束标签，但不对自闭合标签打印
        if (!isSelfClosingTag(node->tag_name))
        {
            for (int i = 0; i < depth; ++i)
            {
                cout << "  "; // 两个空格表示一级缩进
            }
            cout << "</" << node->tag_name << ">" << endl;
        }
    }

    void printall() // 打印解析出的整个html文件
    {
        printTree(root, 0);
    }

    void clear()
    {
        if (root != nullptr)
        {
            deleteTree(root);
            root = nullptr; // 重置 root
        }
    }

    bool isBlockTag(const std::string &tagName) // 判断是否是块级标签
    {
        // 定义块级元素集合
        static const std::set<std::string> blockTags = {
            "div", "p", "h1", "h2", "h3", "h4", "h5", "h6",
            "ul", "ol", "li", "header", "footer", "section",
            "article", "aside", "blockquote", "table", "tr",
            "th", "td", "form", "hr", "address", "main"};

        return blockTags.find(tagName) != blockTags.end();
    }

    bool isInlineTag(const std::string &tagName) // 判断是否是内联标签
    {
        set<std::string> inlineTags = {
            "a", "span", "strong", "em", "b", "i", "img", "input",
            "label", "code", "small", "time", "sub", "sup", "q",
            "cite", "del", "ins"};
        return inlineTags.find(tagName) != inlineTags.end();
    }

    bool is_SpecialBlockTag(const std::string &tagName) // 判断是否是特殊的内联标签
    {
        string special = "h1 h2 h3 h4 h5 h6 p dt";
        return !(special.find(tagName) == string::npos);
    }

    void deleteTree(html_node *node)
    {
        if (node == nullptr)
            return;

        Node<html_node *> *current = node->son_node.get_head();
        if (current == nullptr || current->next == nullptr)
        {
            delete node; // 如果无子节点，直接删除节点
            return;
        }

        // 删除子节点
        current = current->next; // 跳过哑节点
        while (current != nullptr)
        {
            Node<html_node *> *nextNode = current->next;
            deleteTree(current->data); // 递归删除子节点
            current = nextNode;
        }
    }

    ~Html_parser()
    {
        clear();
    }

    void file_input() // 读入文件并解析文件
    {
        this->clear();

        cout << "请选择输入方式：" << endl;
        cout << "[1]: 输入文件地址" << endl;
        cout << "[2]: 输入 URL" << endl;
        int choice;
        cin >> choice;
        if (choice == 1)
        {
            string address;
            cout << "请输入文件地址" << endl;
            cin >> address;
            ifstream file(address);
            while (!file)
            {
                cerr << "无法打开文件！请重新输入文件地址！" << endl;
                cout << "请输入文件地址：" << endl;
                cin >> address;
                file.clear();
                file.close();
                file.open(address);
            }

            // 使用 istreambuf_iterator 读取文件内容到字符串中
            string content((istreambuf_iterator<char>(file)),
                           istreambuf_iterator<char>());
            while (!this->parse(content))
            {
                cout << "文件不合法，请重新给出文件" << endl;
                cout << "请输入文件地址：" << endl;
                cin >> address;
                file.clear();       // 重置文件流状态
                file.close();       // 确保关闭文件
                file.open(address); // 重新打开文件
                while (!file)
                {
                    cerr << "无法打开文件！请重新输入文件地址！" << endl;
                    cout << "请输入文件地址：" << endl;
                    cin >> address;
                    file.clear();
                    file.close();
                    file.open(address);
                }

                // 使用 istreambuf_iterator 读取文件内容到字符串中
                content.clear();
                content.assign((istreambuf_iterator<char>(file)),
                               istreambuf_iterator<char>());
            }
        }
        else if (choice == 2)
        {
            string address;
            cout << "请输入 URL 地址：" << endl;
            cin >> address;

            // 解析 URL
            while (!this->parseFromURL(address))
            {
                cout << "URL 解析失败，请检查 URL 的有效性。" << endl;
                cout << "请输入 URL 地址：" << endl;
                cin >> address;
            }
        }
        else
        {
            cout << "无效的选择，请重新选择。" << endl;
        }
        cout << "文件解析成功" << endl;
    }
};
