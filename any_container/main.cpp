#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <gtest/gtest.h>

#include "any_container.h"

using fw = std::forward_iterator_tag;
using bidir = std::bidirectional_iterator_tag;
using racc = std::random_access_iterator_tag;

template <typename T>
using vector = std::vector<T>;

template <typename T>
using list = std::list<T>;

template <typename T>
using deque = std::deque<T>;

template <typename T, typename IT_TAG, template <typename> class CONTAINER>
CONTAINER<T> cont(any_container<T, IT_TAG> const & x) {
    CONTAINER<T> a;
    for (auto it = x.cbegin(); it != x.cend(); ++it) {
        a.push_back(*it);
    }
    return a;
};

TEST(correctness, empty)
{
    any_container<int, racc> x;
}

TEST(any_range, vector)
{
    vector<int> a{1, 2, 3, 4, 5};
    any_container<int, racc> x(a);
    for (auto it = x.begin(); it != x.end(); ++it) {
        static int ans = 1;
        EXPECT_EQ(ans, *it);
        ++ans;
    }
    for (auto it = x.cbegin(); it != x.cend(); ++it) {
        static int ans = 1;
        EXPECT_EQ(ans, *it);
        ++ans;
    }
}

TEST(any_range, list)
{
    list<int> a{1, 2, 3, 4, 5};
    any_container<int, bidir> x(a);
    for (auto it = x.begin(); it != x.end(); ++it) {
        static int ans = 1;
        EXPECT_EQ(ans, *it);
        ++ans;
    }
    for (auto it = x.cbegin(); it != x.cend(); ++it) {
        static int ans = 1;
        EXPECT_EQ(ans, *it);
        ++ans;
    }
}

TEST(any_range, deque)
{
    deque<int> a{1, 2, 3, 4, 5};
    any_container<int, racc> x(a);
    for (auto it = x.begin(); it != x.end(); ++it) {
        static int ans = 1;
        EXPECT_EQ(ans, *it);
        ++ans;
    }
    for (auto it = x.cbegin(); it != x.cend(); ++it) {
        static int ans = 1;
        EXPECT_EQ(ans, *it);
        ++ans;
    }
}

TEST(size, vector_list_deque)
{
    vector<int> v; list<int> l; deque<int> d;
    for (int i = 0; i < 10; ++i) {
        any_container<int, racc> vv(v);
        any_container<int, bidir> ll(l);
        any_container<int, racc> dd(d);
        EXPECT_EQ(true, vv.size() == i && ll.size() == i && dd.size() == i);
        v.push_back(i);
        l.push_back(i);
        d.push_back(i);
    }
}

TEST(insert_erase_by_citer, vector)
{
    vector<int> v{1, 2, 3, 4, 5};
    any_container<int, racc> x(v);
    x.insert(x.cbegin(), 0);
    EXPECT_EQ(vector<int>({0, 1, 2, 3, 4, 5}), (cont<int, racc, vector>(x)));
    x.erase(++++++++++x.cbegin());
    EXPECT_EQ(vector<int>({0, 1, 2, 3, 4}), (cont<int, racc, vector>(x)));
}

TEST(insert_erase_by_citer, list)
{
    list<int> v{1, 2, 3, 4, 5};
    any_container<int, bidir> x(v);
    x.insert(x.cbegin(), 0);
    EXPECT_EQ(list<int>({0, 1, 2, 3, 4, 5}), (cont<int, bidir, list>(x)));
    x.erase(++++++++++x.cbegin());
    EXPECT_EQ(list<int>({0, 1, 2, 3, 4}), (cont<int, bidir, list>(x)));
}

TEST(insert_erase_by_citer, deque)
{
    deque<int> v{1, 2, 3, 4, 5};
    any_container<int, racc> x(v);
    x.insert(x.cbegin(), 0);
    EXPECT_EQ(deque<int>({0, 1, 2, 3, 4, 5}), (cont<int, racc, deque>(x)));
    x.erase(++++++++++x.cbegin());
    EXPECT_EQ(deque<int>({0, 1, 2, 3, 4}), (cont<int, racc, deque>(x)));
}

TEST(insert_erase_by_iter, vector)
{
    vector<int> v{1, 2, 3, 4, 5};
//    any_iterator<int, racc> it(v.cbegin());  //compilation error
    any_container<int, racc> x(v);
    EXPECT_TRUE(x.begin() == x.cbegin());
    x.insert(x.begin(), 0);
    EXPECT_EQ(vector<int>({0, 1, 2, 3, 4, 5}), (cont<int, racc, vector>(x)));
    x.erase(++++++++++x.begin());
    EXPECT_EQ(vector<int>({0, 1, 2, 3, 4}), (cont<int, racc, vector>(x)));
}

TEST(insert_erase_by_iter, list)
{
    list<int> v{1, 2, 3, 4, 5};
    any_container<int, bidir> x(v);
    x.insert(x.begin(), 0);
    EXPECT_EQ(list<int>({0, 1, 2, 3, 4, 5}), (cont<int, bidir, list>(x)));
    x.erase(++++++++++x.begin());
    EXPECT_EQ(list<int>({0, 1, 2, 3, 4}), (cont<int, bidir, list>(x)));
}

TEST(insert_erase_by_iter, deque)
{
    deque<int> v{1, 2, 3, 4, 5};
    any_container<int, racc> x(v);
    x.insert(x.begin(), 0);
    EXPECT_EQ(deque<int>({0, 1, 2, 3, 4, 5}), (cont<int, racc, deque>(x)));
    x.erase(++++++++++x.begin());
    EXPECT_EQ(deque<int>({0, 1, 2, 3, 4}), (cont<int, racc, deque>(x)));
}