#include "html_parser.h"
#include <sstream>
#include <unordered_set>
#include <unistd.h>
string removeQuotes(string value)
{
    // 去除前后的空格
    size_t start = value.find_first_not_of(" \t\n\r\f\v");
    size_t end = value.find_last_not_of(" \t\n\r\f\v");

    if (start == string::npos || end == string::npos) // 如果字符串为空或全是空白字符
        return "";

    value = value.substr(start, end - start + 1); // 去掉前后的空格

    // 去掉引号
    if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\'')))
    {
        return value.substr(1, value.size() - 2); // 去掉引号
    }
    return value;
}

typedef struct
{
    string name;
    string id;
    vector<string> classes;
    vector<string> attrprocess;
    void print() const
    {
        cout << "Tag Details:\n";
        cout << "  Name: " << name << "\n";
        cout << "  ID: " << (id.empty() ? "(none)" : id) << "\n";
        cout << "  Classes: ";
        if (classes.empty())
        {
            cout << "(none)";
        }
        else
        {
            for (const auto &cls : classes)
            {
                cout << cls << " ";
            }
        }
        cout << "\n";
    }

    // 以下都是用于判断一个node是否符合相应的条件
    bool is_rclass(html_node *node)
    {
        bool flag = true;
        if (node->attr.find("class") == node->attr.end() && classes.size() > 0)
        {
            return false;
        }
        stringstream ss(node->attr["class"]);
        unordered_set<string> node_classes;
        string temp;
        while (getline(ss, temp, ' '))
        {
            node_classes.insert(temp);
        }
        for (string c : classes)
        {
            if (node_classes.find(c) == node_classes.end())
            {
                flag = false;
                break;
            }
        }
        return flag;
    }

    bool is_rid(html_node *Node)
    {
        if (id == "")
        {
            return true;
        }
        else
        {
            if (Node->attr.find("id") == Node->attr.end())
            {
                return false;
            }
            return (Node->attr["id"] == id);
        }
    }

    bool is_rtagname(html_node *node)
    {
        if (name == "")
        {
            return true;
        }
        else
        {
            return (node->tag_name == name);
        }
    }

    bool is_rattr(html_node *node)
    {
        string opts = "=~|^$*";
        bool flag = true;

        // 遍历所有的属性条件
        for (string attr : attrprocess)
        {
            size_t pos = attr.find_first_of(opts); // 查找操作符的位置

            // [attr] 的情况
            if (pos == string::npos)
            {
                if (node->attr.find(attr) == node->attr.end()) // 如果没有找到该属性
                {
                    flag = false;
                    break;
                }
            }
            else
            {
                string key = attr.substr(0, pos);
                string value = attr.substr(pos + 2);
                value = removeQuotes(value); // 去掉属性值中的引号，并去除两端空格

                // 属性值的匹配逻辑
                switch (attr[pos])
                {
                case '=':                         // [attr=value]
                    value = attr.substr(pos + 1); // 如果是“=”则跳过一个字符就行了
                    if (node->attr.find(key) == node->attr.end() || node->attr[key] != value)
                    {
                        flag = false;
                    }
                    break;

                case '~': // [attr~=value] 包含子串
                    if (node->attr.find(key) == node->attr.end() || node->attr[key].find(value) == string::npos)
                    {
                        flag = false;
                    }
                    break;

                case '|': // [attr|=value] 前缀匹配
                    if (node->attr.find(key) == node->attr.end() || node->attr[key].find(value) != 0)
                    {
                        flag = false;
                    }
                    break;

                case '^': // [attr^=value] 前缀匹配
                    if (node->attr.find(key) == node->attr.end() || node->attr[key].find(value) != 0)
                    {
                        flag = false;
                    }
                    break;

                case '$': // [attr$=value] 后缀匹配
                {
                    string key_value = node->attr[key];
                    size_t len1 = key_value.size(), len2 = value.size();
                    if (len1 < len2 || key_value.substr(len1 - len2) != value)
                    {
                        flag = false;
                    }
                }
                break;

                case '*': // [attr*=value] 包含子串
                    if (node->attr.find(key) == node->attr.end() || node->attr[key].find(value) == string::npos)
                    {
                        flag = false;
                    }
                    break;

                default:
                    cout << "非法query: " << attr << endl;
                    exit(1);
                }

                if (!flag)
                    break;
            }
        }
        return flag;
    }
} tag;

class Css_selector
{
private:
    Html_parser *parser;
    vector<Node<html_node *> *> *answer;

public:
    Css_selector() : parser(new Html_parser()), answer(new vector<Node<html_node *> *>) {}
    Html_parser *get_parser()
    {
        return parser;
    }

    vector<Node<html_node *> *> *get_answer()
    {
        return answer;
    }

    void query(string query, html_node *start)
    {
        // 处理询问字符串query,解析为tag名和运算符，tag名应该包含标签名和属性列表，将tag名和运算符分别压入栈
        // 处理询问字符串query
        string opts = " +>~";

        // 第一步：处理“，”
        string temp;
        stringstream ss(query);
        vector<string> subqueries;
        while (getline(ss, temp, ','))
        {
            subqueries.push_back(temp);
        }
        // 第二步：分别处理每个子query
        for (string subquery : subqueries)
        {
            Stack<tag *> tagstack;
            Stack<char> operator_stack;
            vector<Node<html_node *> *> *subanswer = new vector<Node<html_node *> *>;
            // 解析子query字符串
            string temp_tag;
            subquery = trim(subquery); // 去除子串两边多余空白符
            for (size_t t = 0; t < subquery.size(); t++)
            {
                if (subquery[t] == '[')
                {
                    size_t k = subquery.find(']', t + 1);
                    temp_tag += subquery.substr(t, k - t + 1);
                    t = k;
                }
                else if (opts.find(subquery[t]) == string::npos)
                {
                    temp_tag += subquery[t];
                }
                else if (subquery[t] == ' ')
                {

                    if (opts.find(subquery[t - 1]) == string::npos && opts.find(subquery[t + 1]) == string::npos)
                    {
                        cout << "opt:" << subquery[t] << endl;
                        operator_stack.push(subquery[t]);
                        tag *a = process_tag(temp_tag);
                        a->print();
                        tagstack.push(a);
                        temp_tag.clear();
                    }
                }
                else
                {
                    cout << "opt:" << subquery[t] << endl;
                    operator_stack.push(subquery[t]);
                    tag *a = process_tag(temp_tag);
                    a->print();
                    tagstack.push(a);
                    temp_tag.clear();
                }
            }
            process_tag(temp_tag)->print();
            tagstack.push(process_tag(temp_tag));
            // 开始查询
            if (!tagstack.isEmpty())
            {
                tag *begin_tag = tagstack.top();
                Node<html_node *> Root(start);
                global_search(begin_tag, subanswer, &Root);
                tagstack.pop();
                while ((!tagstack.isEmpty()) && (!operator_stack.isEmpty()))
                {
                    cout << "正在运算：" << endl
                         << "opt:" << operator_stack.top() << endl
                         << "栈顶tag:";
                    tagstack.top()->print();
                    subanswer = select_operation(tagstack.top(), subanswer, operator_stack.top());
                    tagstack.pop();
                    operator_stack.pop();
                }
            }
            answer->insert(answer->end(), subanswer->begin(), subanswer->end());
        }
        // 第三步，对answer去重
        vector<Node<html_node *> *> *result = new vector<Node<html_node *> *>();

        unordered_set<Node<html_node *> *> seen;

        for (Node<html_node *> *ptr : *answer)
        {
            if (seen.find(ptr) == seen.end())
            {
                result->push_back(ptr);
                seen.insert(ptr);
            }
        }
        answer->clear();
        *answer = *result;
    };

    tag *process_tag(string name)
    {
        tag *ptag = new tag();
        if (name == "*")
        {
            return ptag;
        }
        size_t pos = name.find_first_of(".#[");
        if (pos == string::npos)
        {
            ptag->name = name;
        }
        else
        {
            ptag->name = name.substr(0, pos);
            size_t tpos = 0;
            while (pos != string::npos && pos < name.length())
            {
                string attr;
                tpos = name.find_first_of(".#[", pos + 1);
                if (tpos == string::npos)
                {
                    attr = name.substr(pos + 1);
                    if (name[pos] == '.')
                    {
                        ptag->classes.push_back(attr);
                    }
                    else if (name[pos] == '#')
                    {
                        ptag->id = attr;
                    }
                    else if (name[pos] == '[')
                    {
                        size_t a = name.find(']', pos + 1);
                        if (a == string::npos)
                        {
                            cout << "\'[\'未封闭" << endl;
                            exit(1);
                        }
                        attr = name.substr(pos + 1, a - pos - 1);
                        ptag->attrprocess.push_back(attr);
                    }
                    break;
                }
                else
                {
                    attr = name.substr(pos + 1, tpos - pos - 1);
                    if (name[pos] == '[')
                    {
                        size_t a = name.find(']', pos + 1);
                        if (a == string::npos)
                        {
                            cout << "\'[\'未封闭" << endl;
                            exit(1);
                        }
                        attr = name.substr(pos + 1, a - pos - 1);
                        ptag->attrprocess.push_back(attr);
                        pos = a + 1;
                    }
                    else if (name[pos] == '.')
                    {
                        ptag->classes.push_back(attr);
                        pos = tpos;
                    }
                    else if (name[pos] == '#')
                    {
                        ptag->id = attr;
                        pos = tpos;
                    }
                }
            }
        }
        return ptag;
    }

    vector<Node<html_node *> *> *select_operation(tag *a, vector<Node<html_node *> *> *b, char opt)
    {
        vector<Node<html_node *> *> *ans = new vector<Node<html_node *> *>;
        switch (opt)
        {
        case ' ':
        {
            for (Node<html_node *> *node : *b)
            {
                html_node *nownode = node->data;
                Node<html_node *> *org = node;
                int flag = 0;
                while (nownode->parent != nullptr)
                {
                    html_node *parent = nownode->parent;
                    if (a->is_rclass(parent) && a->is_rid(parent) && a->is_rtagname(parent) && a->is_rattr(parent))
                    {
                        flag = 1;
                        break;
                    }
                    nownode = parent;
                }
                if (flag == 1)
                    ans->push_back(org);
            }
            break;
        }
        case '>':
        {
            for (Node<html_node *> *node : *b)
            {
                Node<html_node *> *org = node;
                if (node->data->parent != nullptr)
                {
                    html_node *parent = node->data->parent;
                    if (a->is_rclass(parent) && a->is_rid(parent) && a->is_rtagname(parent) && a->is_rattr(parent))
                    {
                        ans->push_back(org);
                    }
                }
            }
            break;
        }
        case '+':
        {
            for (Node<html_node *> *node : *b)
            {
                Node<html_node *> *org = node;
                while (node->prev != nullptr && node->prev->data != nullptr && node->prev->data->tag_name == "text") // 跳过文本节点
                {
                    node = node->prev;
                }
                if (node->prev != nullptr && node->prev->data != nullptr)
                {
                    html_node *brother = node->prev->data;
                    if (a->is_rclass(brother) && a->is_rid(brother) && a->is_rtagname(brother) && a->is_rattr(brother))
                    {
                        ans->push_back(org);
                    }
                }
            }
            break;
        }
        case '~':
        {
            for (Node<html_node *> *node : *b)
            {
                Node<html_node *> *org = node;
                while (node->prev != nullptr && node->prev->data != nullptr)
                {
                    Node<html_node *> *brother = node->prev;
                    if (a->is_rclass(brother->data) && a->is_rid(brother->data) && a->is_rtagname(brother->data) && a->is_rattr(brother->data))
                    {
                        ans->push_back(org);
                        break;
                    }
                    node = brother;
                }
            }
            break;
        }

        default:
            cout << "含有不合法运算符" << endl;
            exit(1);
            break;
        }
        return ans;
    }

    void global_search(tag *a, vector<Node<html_node *> *> *b, Node<html_node *> *node)
    {
        if (a->is_rclass(node->data) && a->is_rid(node->data) && a->is_rtagname(node->data) && a->is_rattr(node->data) && node->data->tag_name != "text")
        {
            b->push_back(node);
        }
        if (!node->data->son_node.isEmpty())
        {
            Node<html_node *> *first = node->data->son_node.get_head()->next;
            while (first != nullptr && first->data != nullptr)
            {
                global_search(a, b, first);
                first = first->next;
            }
        }
    }

    void print_result()
    {
        cout << "[" << endl;
        int index = 0;
        for (Node<html_node *> *node : *answer)
        {
            cout << "{" << index << "}" << node->data->tag << endl;
            index++;
        }
        cout << "]" << endl;
    }
    // query的结构：name/opt/name..(倒序)先搜寻符合第一个标签名的，再倒回去匹配，","独立解析
    string trim(const std::string &str)
    {
        const std::string whitespace = " \t\n\r\f\v";

        // 找到第一个非空白字符的位置
        size_t start = str.find_first_not_of(whitespace);
        if (start == std::string::npos)
        {
            // 如果全是空白字符，返回空字符串
            return "";
        }

        // 找到最后一个非空白字符的位置
        size_t end = str.find_last_not_of(whitespace);

        // 返回截取的子字符串
        return str.substr(start, end - start + 1);
    }

    void clear_answer()
    {
        this->answer->clear();
    }
};
