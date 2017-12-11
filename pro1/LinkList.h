#pragma once
/*
 * 模板类:单链表
 *
 */
#include <memory>

template <typename ElemType>
class Node
{
public:
    ElemType _data;

    std::shared_ptr<Node<ElemType>> _next;

    Node(ElemType data) : _data(data)
    {
        _next = nullptr;
    };
};


template <typename ElemType>
class LinkList
{
private:

    unsigned int _size;
public:
    std::shared_ptr<Node<ElemType>> head;

    LinkList()
    {
        head = nullptr;
        _size = 0;
    }

    ElemType& operator[](unsigned int index);
    void insert(ElemType data);
    void push_back(ElemType data);
    unsigned int size() const;
    bool contains(ElemType data);
    void remove_all();
};

template <typename ElemType>
ElemType& LinkList<ElemType>::operator[](unsigned index)
{
    if (index < 0 || index >= _size)
        throw Error("LinkList operator[]: index out of range");
    auto current = head;
    int current_index = 0;
    while (current)
    {
        if (current_index == index)
        {
            return (current->_data);
        }
        current = current->_next;
        current_index++;
    }
}

/**
 * \brief 插入数据到链表末尾
 * \param data 
 */
template <typename ElemType>
void LinkList<ElemType>::insert(ElemType data)
{
    std::shared_ptr<Node<ElemType>> ins = std::make_shared<Node<ElemType>>(data);
    
    //Node<ElemType>* ins = new Node<ElemType>(data);
    _size++;
    if (head == nullptr)
    {
        head = ins;
        return;
    }
    auto current = head;
    while (current->_next)
    {
        current = current->_next;
    }
    current->_next = ins;
    return;
}

template <typename ElemType>
void LinkList<ElemType>::push_back(ElemType data)
{
    insert(data);
}

/**
 * \brief 获取链表的大小
 * \return 
 */
template <typename ElemType>
unsigned int LinkList<ElemType>::size() const
{
    return _size;
}

/**
 * \brief 移除所有元素
 */
template <typename ElemType>
void LinkList<ElemType>::remove_all()
{
    //    auto current = head;
    //
    //    while (current)
    //    {
    //        auto next = current->_next;
    //        delete current;
    //        current = next;
    //    }
    //    _size = 0;
    //    head = nullptr;
}

/**
 * \brief 判断链表中是否含有该元素
 * \param data 带查找的元素
 * \return 
 */
template <typename ElemType>
bool LinkList<ElemType>::contains(ElemType data)
{
    auto current = head;

    while (current)
    {
        if (current->_data == data)
            return true;
        current = current->_next;
    }
    return false;
}
