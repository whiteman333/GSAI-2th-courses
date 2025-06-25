#include <iostream>
using namespace std;

// 定义节点类
template <typename T>
class Node
{
public:
    T data;     // 节点数据
    Node *next; // 指向下一个节点的指针

    // 构造函数，初始化节点数据和指针
    Node(T value) : data(value), next(nullptr) {}
};

// 定义链表类
template <typename T>
class Link_list
{
private:
    Node<T> *head; // 链表头指针

public:
    // 构造函数，初始化链表为空
    Link_list() : head(nullptr) {}

    // 析构函数，释放链表内存
    ~Link_list()
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

    // 向链表末尾添加新节点
    void append(T value)
    {
        Node<T> *linknode = new Node<T>(value); // 创建新节点
        if (this->head == nullptr)              // 如果链表为空
        {
            head = linknode; // 设置头指针为新节点
        }
        else
        {
            Node<T> *temp = head;         // 临时指针从头开始遍历
            while (temp->next != nullptr) // 找到链表最后一个节点
            {
                temp = temp->next;
            }
            temp->next = linknode; // 将新节点连接到最后一个节点
        }
    }

    // 获取链表头指针
    Node<T> *get_head() { return head; }
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
