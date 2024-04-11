#include <iostream>

#include "llist.hpp"

#ifndef TESTING

int main()
{
    linked_list<int> lst;

    for (int i = 1; i < 10; ++i) {
        lst.push(i);
    }

    std::cout << lst << std::endl;

    for (auto& i : lst) {
        i *= 2;
    }
    std::cout << lst << std::endl;
}

#else
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_SUITE("linked_list")
{
    TEST_CASE("default construct")
    {
        linked_list<int> lst;
        CHECK(lst.size() == 0);
        CHECK(lst.begin() == lst.end());
    }

    TEST_CASE("list construct")
    {
        linked_list<int> lst{1, 2, 3, 4, 5, 6};
        CHECK(lst.size() == 6);
        auto i = 6;
        for (auto e : lst) {
            CHECK(e == i--);
        }
    }

    TEST_CASE("weak_node array")
    {
        linked_list<int> lst{1, 2, 3, 4, 5, 6};

        auto weak = lst.get_weak_nodes();
        auto i = 6;
        for (auto e : weak) {
            CHECK(**(e.lock()) == i--);
        }
        // delete list.
        lst.pop(0);
        // assert all weak nodes are expired
        for (auto e : weak) {
            CHECK(e.expired());
        }
    }

    TEST_CASE("use_count")
    {
        linked_list<int> lst{1, 2, 3, 4, 5, 6};

        for (auto it = lst.begin(); it != lst.end(); ++it) {
            // one for the node, one for the iterator itself
            CHECK(it.get_node().use_count() == 2);
        }
    }

    TEST_CASE("deletion")
    {
        linked_list<int> lst{1, 2, 3, 4, 5, 6};
        lst.pop(3);
        CHECK(lst.size() == 3);
        auto i = 3;
        for (auto e : lst) {
            CHECK(e == i--);
        }
    }

    TEST_CASE("node lifetime")
    {
        linked_list<int> lst{1, 2, 3, 4, 5, 6};
        auto weakn = std::weak_ptr(lst.at(2));
        { // get an inner scope
            auto n = lst.at(2);
            CHECK(**n == 4);           // list is stored 'backwards' from init
                                       // because we push to the head.
            CHECK(n.use_count() == 2); // n and the list.
            // delete list.
            lst.pop(0);
            CHECK(n.use_count() == 1); // just n.
        }
        CHECK(weakn.expired()); // no owners any more.
    }
}

#endif
