#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include <cstddef>

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
        Node *next{nullptr};
    };

    size_t size{0};
    size_t capacity{0};
    IMemory &memory;
    Node *head{nullptr};
    Node *tail{nullptr};

    // Minimum allowed capacity for the queue
    static constexpr size_t CAPACITY_MIN{3};

public:
    // Disable copy constructor and copy assignment operator
    CircularQueue(const CircularQueue &other) = delete;
    CircularQueue &operator=(const CircularQueue &other) = delete;

    // Constructor to create a circular queue with the given capacity and memory reference
    CircularQueue(size_t _capacity, IMemory &mem) : capacity{_capacity}, memory{mem}
    {
        if (capacity < CAPACITY_MIN)
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
                while (head != nullptr)
                {
                    tail = head;
                    head = head->next;
                    tail->~Node();
                    memory.free(tail);
                }

                throw std::bad_alloc();
            }

            (void)new (newNode) Node;

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

    // Move constructor to move another queue to this queue instance
    CircularQueue(CircularQueue &&that) noexcept : size{that.size}, capacity{that.capacity}, memory{that.memory}, head{that.head}, tail{that.tail}
    {
        that.size = 0;
        that.capacity = 0;
        that.head = nullptr;
        that.tail = nullptr;
    }

    // Move assignment operator to assign another queue to this queue instance
    CircularQueue &operator=(CircularQueue &&that) noexcept
    {
        if (this != &that)
        {
            // Release the resources held by this instance
            for (size_t i = 0; i < size; i++)
            {
                tail = head;
                head = head->next;
                tail->~Node();
                memory.free(tail);
            }

            memory = that.memory;
            capacity = that.capacity;
            size = that.size;
            head = that.head;
            tail = that.tail;

            that.size = 0;
            that.capacity = 0;
            that.head = nullptr;
            that.tail = nullptr;
        }

        return *this;
    }

    // Write an element to the queue
    void write(const T &data)
    {
        tail->data = data;
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

    // Read and remove an element from the queue
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

    // Check if the queue is full
    bool isFull()
    {
        return size == capacity;
    }

    // Get the number of elements in the queue
    size_t counter()
    {
        return size;
    }

    // Empty the queue
    void empty()
    {
        size = 0;
        tail = head;
    }

    // Calculate the average of the elements in the queue
    template <typename U = T, typename = std::enable_if_t<std::is_arithmetic<U>::value>>
    double average(void)
    {
        double sum = 0.0;
        Node *current = head;

        for (size_t i = 0; i < size; ++i)
        {
            sum += static_cast<double>(current->data);
            current = current->next;
        }

        return ((size > 0) ? (sum / size) : sum);
    }

    // Resize the queue to the new capacity
    bool resize(size_t newCapacity)
    {
        bool status{false};

        if (newCapacity >= CAPACITY_MIN)
        {
            if (newCapacity == capacity)
            {
                status = true;
            }
            else if (newCapacity > capacity) // Add NUM nodes to the circular linked list
            {
                status = true;
                Node *before{head}; // Points to the node before tail
                const size_t NUM = newCapacity - capacity;

                while (before->next != tail)
                {
                    before = before->next;
                }

                Node *node{nullptr};
                for (size_t i = 0; i < NUM; i++)
                {
                    node = static_cast<Node *>(memory.malloc(sizeof(Node)));

                    if (node != nullptr)
                    {
                        (void)new (node) Node;
                        node->next = before->next;
                        before->next = node;
                    }
                    else
                    {
                        while (before->next != tail)
                        {
                            node = before->next;
                            before->next = node->next;
                            node->~Node();
                            memory.free(node);
                        }
                        status = false;
                        break;
                    }
                }
                if (before->next != tail)
                {
                    capacity = newCapacity;
                    tail = node;
                }
            }
            else // Remove NUM nodes from the circular linked list
            {
                const size_t NUM = capacity - newCapacity;

                if (NUM > 0)
                {
                    Node *before = head;

                    while (before->next != tail)
                    {
                        before = before->next;
                    }

                    for (size_t i = 0; i < NUM; ++i)
                    {
                        Node *nodeToRemove = tail->next;

                        tail->next = nodeToRemove->next;

                        nodeToRemove->~Node();
                        memory.free(nodeToRemove);
                    }

                    if (size > newCapacity)
                    {
                        size = newCapacity;
                    }

                    capacity = newCapacity;
                    status = true;
                }
            }
        }

        return status;
    }

    ~CircularQueue()
    {
        for (int i = 0; i < capacity; i++)
        {
            tail = head;
            head = head->next;
            tail->~Node();
            memory.free(tail);
        }
    }
};

#endif