#include <test_iterator.hpp>

#include <common.hpp>

#include <cstdlib>
#include <stdexcept>
#include <algorithm>

namespace masonc::test::iterator
{
    test_container::test_container(int count)
    {
        this->count = count;
        data = new int[count];

        for (int i = 0; i < count; i += 1) {
            data[i] = i;
        }
    }

    test_container::~test_container()
    {
        delete[] data;
    }

    masonc::iterator<int> test_container::begin()
    {
        return masonc::iterator<int>{ data };
    }

    masonc::iterator<int> test_container::end()
    {
        return masonc::iterator<int>{ data + count };
    }

    test_container_2::test_container_2(int count, int step)
    {
        assume(count % step == 0);

        this->count = count;
        this->step = step;
        data = new int[count];

        for (int i = 0; i < count; i += step) {
            data[i] = i;
        }
    }

    test_container_2::~test_container_2()
    {
        delete[] data;
    }

    test_iterator<int> test_container_2::begin()
    {
        test_iterator_data iterator_data{ step };
        return test_iterator<int>{ data, &iterator_data };
    }

    test_iterator<int> test_container_2::end()
    {
        test_iterator_data iterator_data{ step };
        return test_iterator<int>{ data + count, &iterator_data };
    }

    void test_forward_iteration()
    {
        test_container container{ 10 };
        int i = 0;

        for (auto it = container.begin(); it != container.end(); it++) {
            if (*it != i) {
                throw std::runtime_error{ "iterator forward iteration test failed" };
                //std::exit(-1);
            }

            i += 1;
        }
    }

    void test_reverse_iteration()
    {
        test_container container{ 10 };
        int i = 9;

        auto it = container.end();
        while (it != container.begin()) {
            it--;

            if (*it != i) {
                throw std::runtime_error{ "iterator reverse iteration test failed" };
                //std::exit(-1);
            }

            i -= 1;
        }
    }

    void test_STL_algorithms()
    {
        test_container container{ 100 };
        auto search_element = std::find(container.begin(), container.end(), 20);

        if (search_element == container.end() || *search_element != 20) {
            throw std::runtime_error{ "iterator STL algorithm test failed" };
            //std::exit(-1);
        }
    }

    void test_iterator_data_iteration()
    {
        test_container_2 container{ 100, 5 };
        int i = 0;

        for (auto it = container.begin(); it != container.end(); it++) {
            if (*it != i) {
                throw std::runtime_error{ "iterator data test failed" };
                //std::exit(-1);
            }

            i += 5;
        }
    }
}