#ifndef QUEUE_H
#define QUEUE_H

#include<iostream>
#include<cstddef>

struct IMemory
{
    virtual void *malloc(size_t size) = 0;
    virtual void free(void *ptr) = 0;
    virtual ~IMemory() = default;
};

template <typename T>
class CircularQueue
{
    struct Node
    {
        T data;
        Node *next;
    };

    Node *head;
    Node *tail;
    size_t size;
    size_t capacity;
    IMemory &memory;

    CircularQueue(const CircularQueue &other);
    CircularQueue &operator=(const CircularQueue &other);


public:
    CircularQueue(size_t capacity, IMemory &mem) : memory(mem), head(nullptr), tail(nullptr), size(0), capacity(capacity)
    {
        if (capacity < 3)
        {
            throw std::invalid_argument("The size must be at least 3");
        }

        // Create nodes and link them in a circular manner
        for (size_t i = 0; i < capacity; ++i)
        { 
            Node *newNode{static_cast<Node *>(memory.malloc(sizeof(Node)))};
            if (newNode == nullptr)
            {
                // Handle memory allocation failure
                throw std::bad_alloc();
            }
            if (i == 0)
            {
                head = tail = newNode;
            }
            else
            {
                tail->next = newNode;
                tail = newNode;
            }
        }
        tail->next = head;
        tail = head;

    }
    

    CircularQueue(CircularQueue &&that) noexcept: memory{that.memory}, size{that.size}, head{that.head}, tail{that.tail}
    {   
        that.size = 0;
        that.head = nullptr;
        that.tail = nullptr;
    }

    CircularQueue &operator=(CircularQueue &&that) noexcept
    {   
        if (this != &that)
        {
            memory = that.memory;
            size = that.size;
            head = that.head;
            tail = that.tail;

            that.size = 0;
            that.head = nullptr;
            that.tail = nullptr;
        }

    return *this;
    }

    void write(T data)
    {
        (void)new(&tail->data) T(data);
        tail = tail->next;

        if (isFull())
        {
            head = head->next;
        }
        else
        {
            size++;
        }

    }

    T read()
    {
        T data{head->data};

        if (size > 0)
        {
            head = head->next;
            --size;
        }

        return data;
    }

    bool isFull()
    {
        return size == capacity;
    }

    size_t counter()
    {
        return size;
    }

    void empty()
    {
        size = 0;
        tail = head;
    }

    template <typename U = T, typename = std::enable_if_t<std::is_arithmetic<U>::value>>

    double average(void)
    {
    if (size == 0) 
    {
        return 0; 
    }

    double sum = 0.0;
    Node* current = head;

    for (size_t i = 0; i < size; ++i) 
    {
        sum += static_cast<double>(current->data);
        current = current->next;
    }

    double avg = sum / static_cast<double>(size);
    return avg;
    }

    void resize()
    {

    }

    ~CircularQueue()
    {
        for (int i = 0; i < capacity; i++)
        {
            Node *temp = head;
            head = head->next;
            memory.free(temp);
        }
        tail = nullptr;
        size = 0;
    }
};    

#endif