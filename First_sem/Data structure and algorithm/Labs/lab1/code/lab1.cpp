#include <iostream>
#include <vector>
#include <cwctype>
#include <map>
#include <locale>
#include <limits>
using namespace std;

// 定义节点结构
struct Node
{
    wstring data; // 节点数据
    Node *prev;   // 指向前一个节点
    Node *next;   // 指向下一个节点

    // 构造函数
    Node(wstring val) : data(val), prev(nullptr), next(nullptr) {}
};

// 判断是否为中文标点符号
bool isChinesePunct(wchar_t ch)
{
    // 常见的中文标点符号
    wstring chinesePuncts = L"，。！？；：《》【】（）“”‘’";

    // 判断字符是否为中文标点
    return chinesePuncts.find(ch) != wstring::npos;
}

// 定义双向链表类
class DoublyLinkedList
{
private:
    Node *head; // 指向链表的头节点
    Node *tail; // 指向链表的尾节点
    int lenth;

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr), lenth(0) {}

    ~DoublyLinkedList()
    {
        Node *current = head;
        while (current != nullptr)
        {
            Node *nextNode = current->next;
            delete current;
            current = nextNode;
        }
    }

    void input()
    {
        this->clear();
        wstring text;
        wcin.ignore(numeric_limits<streamsize>::max(), '\n');
        wcout << L"请输入文本：" << endl;
        getline(wcin, text);

        wstring word;
        for (wchar_t ch : text)
        {
            if (iswpunct(ch) || iswspace(ch) || isChinesePunct(ch))
            {
                if (!word.empty())
                {
                    this->append(word);
                    word.clear();
                }
            }
            else
            {
                word += ch;
            }
        }
        if (!word.empty())
        {
            this->append(word);
        }
    }

    // 在链表末尾插入一个新节点
    void append(wstring val)
    {
        Node *newNode = new Node(val);
        if (tail == nullptr)
        { // 如果链表为空
            head = tail = newNode;
        }
        else
        {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        this->lenth++;
    }

    // 在链表头插入一个新节点
    void prepend(wstring val)
    {
        Node *newNode = new Node(val);
        if (head == nullptr)
        { // 如果链表为空
            head = tail = newNode;
        }
        else
        {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        this->lenth++;
    }

    void insertNode(int index, wstring a)
    {
        // 如果索引为负数，直接返回
        if (index < 0 || index > this->lenth)
        {
            wcout << L"索引超出范围" << endl;
            return;
        }

        Node *newNode = new Node(a);
        if (index == 0)
        {
            // 插入到头部
            this->prepend(a);
            return;
        }
        if (index == this->lenth)
        {
            // 插入到尾部
            this->append(a);
            return;
        }

        // 插入到中间位置
        Node *current = head;
        int currentIndex = 0;

        // 遍历链表找到指定索引的位置
        while (current != nullptr && currentIndex < index)
        {
            current = current->next;
            currentIndex++;
        }

        // 插入节点
        newNode->next = current;
        newNode->prev = current->prev;
        current->prev->next = newNode;
        current->prev = newNode;

        this->lenth++; // 别忘了更新链表长度
    }

    int deleteNode(int index)
    {
        Node *current = head;
        int currentIndex = 0;

        // 遍历链表找到指定索引的节点
        while (current != nullptr && currentIndex < index)
        {
            current = current->next;
            currentIndex++;
        }

        // 如果找不到该索引对应的节点
        if (current == nullptr)
        {
            wcout << L"索引超出范围" << endl;
            return 0;
        }

        // 删除头节点
        if (current == head)
        {
            head = current->next;
            if (head != nullptr)
            {
                head->prev = nullptr;
            }
            else
            {
                // 如果链表只有一个节点，删除后需要更新尾指针
                tail = nullptr;
            }
        }
        // 删除尾节点
        else if (current == tail)
        {
            tail = current->prev;
            if (tail != nullptr)
            {
                tail->next = nullptr;
            }
        }
        // 删除中间节点
        else
        {
            current->prev->next = current->next;
            current->next->prev = current->prev;
        }
        delete current;
        this->lenth--;
        return 1;
    }
    void deleteNodeS(int index, int num)
    {
        for (int i = 0; i < num; i++)
        {
            if (!this->deleteNode(index))
            {
                wcout << L"字符串长度不足" << endl;
                return;
            }
        }
    }
    // 从头到尾打印链表
    void printList()
    {
        Node *current = head;
        while (current != nullptr)
        {
            wcout << current->data << L" ";
            current = current->next;
        }
        wcout << endl;
    }

    // 倒置链表
    void invert()
    {
        Node *current = head;
        Node *temp = nullptr;

        // 交换 prev 和 next 指针
        while (current != nullptr)
        {
            temp = current->prev;
            current->prev = current->next;
            current->next = temp;
            current = current->prev;
        }

        // 交换头尾指针
        if (temp != nullptr)
        {
            head = temp->prev;
        }
    }

    bool is_in(wstring a)
    {
        Node *current = head;

        // 遍历链表找到指定单词
        while (current != nullptr)
        {
            if (a == current->data)
            {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool is_symmetry()
    {
        Node *former = head;
        Node *latter = tail;
        int count = 0;
        while (count <= this->lenth)
        {
            if (former->data != latter->data)
            {
                return false;
            }
            former = former->next;
            latter = latter->prev;
            count += 2;
        }
        return true;
    }

    // 构建词典
    map<wstring, int> build_diction()
    {
        Node *current = head;
        map<wstring, int> diction;
        // 遍历链表找到指定单词并计数
        while (current != nullptr)
        {
            diction[current->data]++;
            current = current->next;
        }
        return diction;
    }

    int get_lenth()
    {
        return this->lenth;
    }

    // 清空链表
    void clear()
    {
        int len = this->lenth;
        for (int i = 0; i < len; i++)
        {
            this->deleteNode(0);
        }
    }
};

int main()
{
    setlocale(LC_ALL, ""); // 设置本地化支持
    DoublyLinkedList *paper = new DoublyLinkedList();
    paper->input();
    int k = 0;
    while (k != -1)
    {
        wcout << L"1:打印当前字符串" << endl
              << L"2:插入单词" << endl
              << L"3:删除单词" << endl
              << L"4:倒置当前字符串" << endl
              << L"5:判断当前字符串是否是回文串" << endl
              << L"6:计算字符串词数" << endl
              << L"7:查找某个字符串是否存在" << endl
              << L"8:销毁当前字符串" << endl
              << L"9:生成词典" << endl
              << L"10:重新输入字符串" << endl
              << L"11:删除某个位置的多个单词" << endl
              << L"-1:退出操作" << endl
              << L"请选择你想进行的操作：" << endl;
        while (!(wcin >> k))
        {
            wcin.clear(); // 清除错误输入
            wcin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略缓冲区中的多余字符
            wcout << L"无效输入，请输入数字：" << endl;
        }
        wstring word;
        int index;
        map<wstring, int> diction;
        switch (k)
        {
        case 1:
            paper->printList();
            break;

        case 2:
        {
            wcout << L"请输入你要插入的单词" << endl;
            wcin >> word;
            wcout << L"请输入你要插入的位置索引" << endl;
            wcin >> index;
            paper->insertNode(index, word);
            break;
        }

        case 3:
        {
            wcout << L"请输入你要删除的单词的位置索引" << endl;
            wcin >> index;
            paper->deleteNode(index);
            break;
        }

        case 4:
        {
            paper->invert();
            wcout << L"已完成倒置" << endl;
            break;
        }

        case 5:
        {
            bool a = paper->is_symmetry();
            wcout << (a ? L"是回文串" : L"不是回文串") << endl;
            break;
        }

        case 6:
        {
            int lenth = paper->get_lenth();
            wcout << lenth << endl;
            break;
        }

        case 7:
        {
            wcout << L"请输入你要查找的词语：" << endl;
            wcin >> word;
            bool b = paper->is_in(word);
            wcout << (b ? L"存在" : L"不存在") << endl;
            break;
        }

        case 8:
        {
            delete paper;
            wcout << L"销毁字符串请不要再对该字符串进行操作" << endl;
            break;
        }

        case 9:
        {
            map<wstring, int> diction = paper->build_diction();
            for (auto it = diction.begin(); it != diction.end(); ++it)
            {
                wcout << it->first << L": " << it->second << endl;
            }
            break;
        }
        case 10:
        {
            paper->input();
            break;
        }
        case 11:
        {
            int index, num;
            wcout << L"请输入索引" << endl;
            wcin >> index;
            wcout << L"请输入你要删除的单词数量" << endl;
            wcin >> num;
            paper->deleteNodeS(index, num);
            break;
        }
        case -1:
            return 0;
        default:
            wcout << L"无效的选项，请重新选择。" << endl;
            break;
        }
        wcout << endl;
    }
}
