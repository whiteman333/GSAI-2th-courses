#include "Css_selector.h"
#include <limits>

int main()
{
    Css_selector *selector = new Css_selector();
    Html_parser *parser = selector->get_parser();
    parser->file_input();
    int sign = 0;
    while (sign != -1)
    {
        cout << "==========================================" << endl;
        cout << "                  主菜单" << endl;
        cout << "==========================================" << endl;
        cout << "请选择以下操作：" << endl;
        cout << " [1]: 重新选择文件进行解析 " << endl;
        cout << "        - 加载一个新的 HTML 文件进行解析操作。" << endl;
        cout << " [2]: 查找符合条件的 HTML 节点" << endl;
        cout << "        - 根据条件查询并返回相关节点信息。" << endl;
        cout << " [3]: 打印解析出的 HTML 文件" << endl;
        cout << "        - 显示整个解析后的 HTML 文件结构。" << endl;
        cout << " [-1]: 退出程序" << endl;
        cout << "        - 结束程序并退出。" << endl;
        cout << "==========================================" << endl;
        cout << "请输入操作编号：";

        while (!(cin >> sign))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "无效输入，请重新输入" << endl;
        }; // 注意sign不是数字的情况
        switch (sign)
        {
        case 1:
        {
            char *args[] = {const_cast<char *>("./css"), nullptr};
            execv(args[0], args);
            // 如果 execv 执行失败
            perror("execv failed");
            exit(EXIT_FAILURE);
            break;
        }
        case 2:
        {
            string query;
            cout << "请输入查询语句：";
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            getline(cin, query);
            selector->query(query, parser->get_root());
            selector->print_result();
            int si = 0;
            while (si != -1)
            {
                cout << "==========================================" << endl;
                cout << "              查询结果操作菜单" << endl;
                cout << "==========================================" << endl;
                cout << "请选择以下操作：" << endl;
                cout << " [0]:  对节点进行二次查询" << endl;
                cout << "        - 允许对已选节点执行进一步的查询操作" << endl;
                cout << " [1]:  输出节点的内部文本" << endl;
                cout << "        - 提取并显示节点的纯文本内容。" << endl;
                cout << " [2]:  输出节点的内部 HTML" << endl;
                cout << "        - 显示节点及其子节点的完整 HTML 结构。" << endl;
                cout << " [3]:  输出节点的 href 属性 (仅适用于 <a> 标签)" << endl;
                cout << "        - 提取并显示超链接地址。" << endl;
                cout << " [4]:  重新打印查询结果列表" << endl;
                cout << "        - 查看上一次查询的结果列表。" << endl;
                cout << " [-1]: 重新输入查询条件" << endl;
                cout << "        - 开始新的查询操作。" << endl;
                cout << "==========================================" << endl;
                cout << "请输入操作编号：";

                while (!(cin >> si))
                {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "无效输入，请重新输入" << endl;
                }; // 注意sign不是数字的情况
                switch (si)
                {
                case 0:
                {
                    int index;
                    cout << "请输入你想查找的节点索引(从0开始):" << endl;
                    cin >> index;
                    if (index >= selector->get_answer()->size())
                    {
                        cout << "Error:索引超出限度" << endl;
                        break;
                    }
                    Node<html_node *> *start = (*selector->get_answer())[index];
                    cout << "当前选择的节点：" << (*selector->get_answer())[index]->data->tag;
                    selector->clear_answer();
                    cout << "请输入查询语句：";
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    getline(cin, query);
                    selector->query(query, start->data);
                    selector->print_result();
                    break;
                }
                case 1:
                {
                    int index;
                    cout << "请输入你想查找的节点索引(从0开始):" << endl;
                    cin >> index;
                    if (index >= selector->get_answer()->size())
                    {
                        cout << "Error:索引超出限度" << endl;
                    }
                    Node<html_node *> *nownode = (*selector->get_answer())[index];
                    cout << selector->get_parser()->extract_text(nownode->data, nownode->data->parent->tag_name) << endl;
                    break;
                }
                case 2:
                {
                    int index;
                    cout << "请输入你想查找的节点索引(从0开始):" << endl;
                    cin >> index;
                    if (index >= selector->get_answer()->size())
                    {
                        cout << "Error:索引超出限度" << endl;
                        break;
                    }
                    Node<html_node *> *nownode = (*selector->get_answer())[index];
                    cout << selector->get_parser()->extract_html(nownode->data, nownode->data->parent->tag_name) << endl;
                    break;
                }
                case 3:
                {
                    int index;
                    cout << "请输入你想查找的a标签索引(从0开始):" << endl;
                    cin >> index;
                    if (index >= selector->get_answer()->size())
                    {
                        cout << "Error:索引超出限度" << endl;
                    }
                    Node<html_node *> *nownode = (*selector->get_answer())[index];
                    if (nownode->data->tag_name == "a")
                    {
                        if (nownode->data->attr.find("href") != nownode->data->attr.end())
                        {
                            cout << nownode->data->attr["href"] << endl;
                        }
                        else
                        {
                            cout << "该a标签没有href属性" << endl;
                        }
                    }
                    else
                    {
                        cout << "Error:该节点不是a标签" << endl;
                    }
                    break;
                }
                case 4:
                {
                    selector->print_result();
                    break;
                }
                case -1:
                {
                    break;
                }
                default:
                {
                    cout << "无效输入，请重新输入" << endl;
                    break;
                }
                }
            }
            selector->clear_answer();
            break;
        }
        case 3:
        {
            selector->get_parser()->printall();
            break;
        }
        case -1:
        {
            return 0;
            break;
        }
        default:
            cout << "无效输入，请重新输入" << endl;
            break;
        }
    }
    return 0;
}