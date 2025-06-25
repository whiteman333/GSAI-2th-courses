#include <iostream>
#include <string>
using namespace std;

// 定义节点类
template <typename T>
class Node
{
public:
    T data;     // 节点数据
    Node *next; // 指向下一个节点的指针
    Node *prev;

    // 构造函数，初始化节点数据和指针
    Node(T value = T()) : data(value), next(nullptr), prev(nullptr) {}
};

// 定义双向链表类
template <typename T>
class Link_list
{
private:
    Node<T> *head; // 链表头指针
    Node<T> *tail; // 链表尾指针

public:
    // 构造函数，初始化链表为空
    Link_list()
    {
        head = tail = new Node<T>();
    }

    // 析构函数，释放链表内存
    ~Link_list()
    {
        if (!isEmpty())
        {
            Node<T> *current = head;   // 当前节点指针
            Node<T> *nextnode;         // 下一个节点指针
            while (current != nullptr) // 遍历链表
            {
                nextnode = current->next; // 保存下一个节点
                delete current;           // 删除当前节点
                current = nextnode;       // 移动到下一个节点
            }
        }
    }
    // 向链表末尾添加新节点
    Node<T> *get_tail()
    {
        return tail;
    }
    void clear()
    {
        Node<T> *current = head->next;
        while (current != nullptr)
        {
            Node<T> *nextNode = current->next;
            delete current; // 删除当前节点
            current = nextNode;
        }
        head->next = nullptr;
        tail = head; // 确保 tail 被正确设置为 head
    }

    void append(T value)
    {
        Node<T> *linknode = new Node<T>(value); // 创建新节点
        if (head == tail)                       // 如果链表为空
        {
            head->next = tail = linknode; // 设置头指针为新节点
            linknode->prev = head;
        }
        else
        {
            tail->next = linknode;
            linknode->prev = tail;
            tail = linknode;
        }
    }

    bool isEmpty()
    {
        return tail == head;
    }
    // 获取链表头指针
    Node<T> *get_head()
    {
        if (head == nullptr)
        {
            return nullptr; // 如果未初始化，返回 nullptr
        }
        return head;
    }
};

// 定义栈类
template <typename T>
class Stack
{
private:
    Node<T> *topNode; // 栈顶指针

public:
    // 构造函数，初始化栈为空
    Stack() : topNode(nullptr) {}

    // 压栈操作
    void push(T value)
    {
        Node<T> *newnode = new Node<T>(value); // 创建新节点
        newnode->next = topNode;               // 将新节点的next指向当前栈顶
        topNode = newnode;                     // 更新栈顶为新节点
    }

    // 弹栈操作
    void pop()
    {
        if (isEmpty()) // 检查栈是否为空
        {
            cout << "弹出错误，栈已空" << endl; // 输出错误信息
            return;
        }
        Node<T> *temp = topNode; // 保存当前栈顶节点
        topNode = topNode->next; // 更新栈顶为下一个节点
        delete temp;             // 删除原栈顶节点
    }

    // 获取栈顶元素
    T top()
    {
        if (isEmpty()) // 检查栈是否为空
        {
            throw runtime_error("查看失败，栈已空"); // 抛出异常
        }
        return topNode->data; // 返回栈顶元素
    }

    // 检查栈是否为空
    bool isEmpty()
    {
        return topNode == nullptr; // 返回栈空状态
    }
};

// 定义树类
template <typename T>
struct Tree_node
{
    T data;
    Link_list<Tree_node<T> *> child;
    Tree_node(T value) : data(value), child() {}
    ~Tree_node()
    {
        Node<Tree_node *> *current = child.get_head()->next; // 获取子节点链表的第一个元素
        while (current != nullptr)
        {
            delete current->data;    // 删除子节点
            current = current->next; // 移动到下一个子节点
        }
    }
};

template <typename T>
class Tree
{
private:
    Tree_node<T> *root;

public:
};

// 定义前缀节点结构，使用 wchar_t 支持汉字
typedef struct word_node
{
    wchar_t c;
    int weight;

    word_node()
    {
        c = L'\0';
        weight = -1;
    }

    word_node(wchar_t a, int w) : c(a), weight(w) {}
} word_node;

// 定义前缀树类
class Prefix_tree
{
private:
    Tree_node<word_node> *root;

public:
    // 构造函数
    Prefix_tree()
    {
        root = new Tree_node<word_node>(word_node());
    }

    // 析构函数
    ~Prefix_tree()
    {
        if (root != nullptr)
        {
            delete root;
        }
    }

    // 清空前缀树
    void clear()
    {
        delete root;
        root = new Tree_node<word_node>(word_node());
    }

    // 向前缀树中添加一个单词及其权重，word 为宽字符串
    void add_word(const wstring &word, int w)
    {
        Tree_node<word_node> *current_node = root;
        Tree_node<word_node> *next_node = nullptr;

        for (size_t i = 0; i < word.size(); ++i)
        {
            wchar_t c = word[i];
            bool found = false;

            // 遍历当前节点的子节点链表，查找字符 c
            Node<Tree_node<word_node> *> *child = current_node->child.get_head()->next;
            while (child != nullptr)
            {
                if (child->data->data.c == c)
                {
                    next_node = child->data;
                    found = true;
                    break;
                }
                child = child->next;
            }

            if (!found)
            {
                // 如果未找到字符 c 的子节点，创建一个新的子节点
                if (i == word.size() - 1)
                {
                    next_node = new Tree_node<word_node>(word_node(c, w)); // 末尾字符，设置 weight
                }
                else
                {
                    next_node = new Tree_node<word_node>(word_node(c, -1)); // 非末尾字符，weight = -1
                }
                current_node->child.append(next_node); // 将新节点添加到当前节点的子链表
            }
            else
            {
                // 如果是末尾字符，更新 weight
                if (i == word.size() - 1)
                {
                    next_node->data.weight = w;
                }
            }

            // 更新当前节点为下一个节点
            current_node = next_node;
        }
    }

    // 获取一个单词的权重，word 为宽字符串
    int get_weight(const wstring &word) const
    {
        Tree_node<word_node> *current_node = root;
        Tree_node<word_node> *next_node = nullptr;

        for (size_t i = 0; i < word.size(); ++i)
        {
            wchar_t c = word[i];
            bool found = false;

            // 遍历当前节点的子节点链表，查找字符 c
            Node<Tree_node<word_node> *> *child = current_node->child.get_head()->next;
            while (child != nullptr)
            {
                if (child->data->data.c == c)
                {
                    next_node = child->data;
                    found = true;
                    break;
                }
                child = child->next;
            }

            if (!found)
            {
                return -1; // 未找到对应的字符
            }

            if (i == word.size() - 1)
            {
                return next_node->data.weight; // 返回末尾字符的权重
            }

            // 更新当前节点为下一个节点
            current_node = next_node;
        }

        return -1; // 如果 word 为空，返回 -1
    }

    // 获取根节点
    Tree_node<word_node> *get_root() const
    {
        return root;
    }
};