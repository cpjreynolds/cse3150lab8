#ifndef LLIST_HPP
#define LLIST_HPP

#include <initializer_list>
#include <utility>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

// I know its bad form in a header file but oh well.
using std::shared_ptr;
using std::weak_ptr;

template<typename T>
struct linked_list {
    class node;

    using node_ptr = shared_ptr<node>;
    using weak_node = weak_ptr<node>;

    class node : public std::enable_shared_from_this<node> {
    private:
        struct private_tag {};

    public:
        T data;
        node_ptr next;

        T& operator*() { return data; };
        T* operator->() { return &data; };
        T& operator=(const T& v) { data = v; };
        T& operator=(T&& v) { data = std::move(v); };

        weak_node get_weak() { return this->weak_from_this(); };
        node_ptr get_strong() { return this->shared_from_this(); };

        static node_ptr make(T&& data, node_ptr next = nullptr)
        {
            return std::make_shared<node>(std::forward<T>(data),
                                          std::move(next), private_tag{});
        }

        node(T&& data, node_ptr next, private_tag)
            : data{std::forward<T>(data)}, next{next} {};
    };

    /*
    class weak_node {
        weak_node_ptr data;

    public:
        T& operator*() { return (*data.lock()).operator*(); }
        T& operator=(const T& v) { (*data.lock()) = v; }
        T& operator=(T&& v) { (*data.lock()) = std::move(v); }
    };
    */

    struct list_iter {
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = void;
        using pointer = T*;
        using reference = T&;

        reference operator*() { return **next; }
        pointer operator->() { return next.operator->()->operator->(); }
        list_iter& operator=(const T& v) { *next = v; }
        list_iter& operator=(T&& v) { *next = std::move(v); }
        // preincrement
        list_iter& operator++()
        {
            if (next) {
                this->next = this->next->next;
            }
            return *this;
        }
        friend auto operator<=>(const list_iter&, const list_iter&) = default;
        friend bool operator==(const list_iter&, const list_iter&) = default;

        node_ptr& get_node() { return next; }

        list_iter() : next{nullptr} {};
        list_iter(node_ptr p) : next{p} {};

        node_ptr next;
    };

    linked_list(){};
    linked_list(std::initializer_list<T> init)
    {
        for (auto&& i : init) {
            this->push(i);
        }
    }

    list_iter begin() { return list_iter{root}; };
    list_iter end() { return list_iter{}; };

    size_t size() { return len; };
    bool empty() { return len == 0; };

    std::vector<weak_node> get_weak_nodes()
    {
        std::vector<weak_node> ret;
        for (auto it = this->begin(); it != this->end(); ++it) {
            ret.push_back(it.next->get_weak());
        }
        return ret;
    }

    // return a copy of the node_ptr at index 'n'
    node_ptr at(size_t idx)
    {
        if (idx >= len) {
            throw std::runtime_error("index out of range");
        }
        auto it = begin();
        for (auto i = 0u; i < idx; ++i, ++it)
            ;
        return it.next;
    }

    // capturing by value for simplicity
    void push(T v)
    {
        std::exchange(root, node::make(std::move(v), root));
        ++len;
    };
    // remove the `idx`th node, (and therefore all subsequent nodes)
    node_ptr pop(size_t idx = 0)
    {
        if (idx >= len) {
            throw std::runtime_error("pop() index out of range");
        }
        else if (idx == 0) {
            len = 0;
            return std::exchange(root, nullptr);
        }
        else {
            // len = len - idx;
            node_ptr& n = root;
            while (idx != 0) {
                n = root->next;
                --idx;
                --len;
            }
            return std::exchange(n, nullptr);
        }
    }

    friend std::ostream& operator<<(std::ostream& os, linked_list& self)
        requires(requires(T& x) { os << x; })
    {

        os << '[';
        for (auto n : self) {
            os << n << " ";
        }
        os << ']';
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, weak_node& n)
    {
        return os << *n;
    }

private:
    size_t len = 0;
    node_ptr root{nullptr};
};

#endif
