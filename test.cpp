#include<gtest/gtest.h>
#include<gmock/gmock.h>
#include "queue.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

class Memory : public IMemory
{
    std::vector<void *> vec;

public:
    MOCK_METHOD(void *, malloc, (size_t size), (override));
    MOCK_METHOD(void, free, (void *ptr), (override));

    void *allocate(size_t size)
    {
        void *ptr{std::malloc(size)};

        if (ptr != nullptr)
        {
            vec.push_back(ptr);
        }

        return ptr;
    }

    void release(void *ptr)
    {
        vec.erase(std::remove(vec.begin(), vec.end(), ptr), vec.end());
        std::free(ptr);
    }

    ~Memory() 
    { 
        EXPECT_EQ(0, vec.size()); 
    }
        
};

template <typename T>
class QueueFixture : public ::testing::Test
{
    const std::tuple<
        std::vector<int>,
        std::vector<float>,
        std::vector<std::string>>
        allValues{
            {1, 2, 3, 4, 5},
            {1.5f, 2.5f, 3.5f, 4.5f, 5.5f},
            {"A1", "B2", "C3", "D4", "E5"}};

protected:
    const std::vector<T> values{std::get<std::vector<T>>(allValues)};
    NiceMock<Memory> mock;
    CircularQueue<T> *queue;

    void SetUp(void) override
    {

        EXPECT_CALL(mock, malloc(_))
            .WillRepeatedly(Invoke(&mock, &Memory::allocate));

        EXPECT_CALL(mock, free(_))
            .WillRepeatedly(Invoke(&mock, &Memory::release));

        queue = new CircularQueue<T>(5, mock);
        EXPECT_EQ(0, queue->counter());

        for (size_t i = 1; i <= values.size(); i++)
        {
            queue->write(values[i - 1]);
            EXPECT_EQ(i, queue->counter());
        }
    }

    void TearDown(void) override 
    {
        delete queue;
    }
};

using TestTypes = ::testing::Types<int, float, std::string>;
TYPED_TEST_SUITE(QueueFixture, TestTypes);

TYPED_TEST(QueueFixture, Read_Data)
{
    for(size_t i = 0; i < this->values.size(); i++)
    {
        EXPECT_EQ(this->values.size() - i, this->queue->counter());
        EXPECT_EQ(this->values[i], this->queue->read());
        
    }
}

TYPED_TEST(QueueFixture, Overwriting_Test)
{
    EXPECT_TRUE(this->queue->isFull());
    EXPECT_EQ(this->values.size(), this->queue->counter());
    this->queue->write(this->values[0]);
    EXPECT_EQ(this->values.size(), this->queue->counter());
    EXPECT_EQ(this->values[1], this->queue->read());
    EXPECT_EQ(this->values.size() - 1, this->queue->counter());
}

TYPED_TEST(QueueFixture, Size_test)
{
    EXPECT_THROW(CircularQueue<TypeParam>(2, this->mock), std::invalid_argument);
    EXPECT_NO_THROW(CircularQueue<TypeParam>(3, this->mock));
}

TYPED_TEST(QueueFixture, Movable_test)
{   
    CircularQueue<TypeParam> temp{std::move(*this->queue)};
    EXPECT_EQ(this->values.size(), temp.counter());
    EXPECT_EQ(0, this->queue->counter());

    *this->queue = std::move(temp);
    EXPECT_EQ(this->values.size(), this->queue->counter());
    EXPECT_EQ(0, temp.counter());
}

TYPED_TEST(QueueFixture, Counter_test)
{
    this->queue->empty();

    EXPECT_EQ(this->queue->counter(), 0);
}

TYPED_TEST(QueueFixture, Average_test)
{
    if constexpr (std::is_arithmetic_v<TypeParam>)
    {
        TypeParam num_values = sizeof(this->values) / sizeof(this->values[0]);

        for (const auto& value : this->values) 
        {
            this->queue->write(value);
        }

        TypeParam sum = 0;
        for (const auto& value : this->values) 
        {
            sum += value;
        }

        double expected_average = static_cast<double>(sum) / this->values.size();

        double actual_average = this->queue->average();

        EXPECT_EQ(actual_average, expected_average);
    }

}

