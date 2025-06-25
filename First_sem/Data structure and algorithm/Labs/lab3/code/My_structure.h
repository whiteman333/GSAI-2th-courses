#include <iostream>
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
