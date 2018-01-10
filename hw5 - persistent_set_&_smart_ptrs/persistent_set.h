//
// Created by drden on 27.12.2017.
//
#ifndef PERSISTENT_SET_PERSISTENT_SET_H
#define PERSISTENT_SET_PERSISTENT_SET_H

#include <memory>
#include "smart_pointers/shared_ptr.h"
#include "smart_pointers/linked_ptr.h"

using std::get;

template <typename T, template <typename> class smart_pointer = linked_ptr>
class persistent_set
{
private:
    struct Node_base;
    struct Node;

    typedef Node_base* node_ptr;

    smart_pointer<Node_base> root;  //inv: root - dummy; root->val = INF

    //returns <v, parent_v, new_root>
    // v - node, which downhill stops at
    std::tuple<smart_pointer<Node_base>, smart_pointer<Node_base>, smart_pointer<Node_base>>
    downhill(smart_pointer<Node_base> v, T val, bool copy = false, smart_pointer<Node_base> caller = nullptr) const
    {
        smart_pointer<Node_base> cur = v;
        if (copy) {
            cur = smart_pointer<Node_base>( v->is_dummy() ? new Node_base(*v) : new Node(*dynamic_cast<Node*>(v.get())) );
        }
        smart_pointer<Node_base>& to = (v->is_dummy() || val < dynamic_cast<Node*>(cur.get())->val ? cur->l : cur->r);
        if (!v->is_dummy() && dynamic_cast<Node*>(cur.get())->val == val || !to)
            return std::make_tuple(cur, caller, cur);
        auto res = downhill(to, val, copy, cur);
        to = get<2>(res);
        get<2>(res) = cur;
        return res;
    };

    node_ptr get_next(node_ptr x) const {
        node_ptr cur = root.get();
        node_ptr next = nullptr;
        while (cur) {
            if (cur->is_dummy() || !x->is_dummy() && dynamic_cast<Node*>(x)->val < dynamic_cast<Node*>(cur)->val) {
                next = cur;
                cur = cur->l.get();
            } else {
                cur = cur->r.get();
            }
        }
        return next;
    }

    node_ptr get_prev(node_ptr x) const {
        node_ptr cur = root.get();
        node_ptr prev = nullptr;
        while (cur) {
            if (cur->is_dummy() || !x->is_dummy() && dynamic_cast<Node*>(x)->val <= dynamic_cast<Node*>(cur)->val) {
                cur = cur->l.get();
            } else {
                prev = cur;
                cur = cur->r.get();
            }
        }
        return prev;
    }

    void remove_light(smart_pointer<Node_base>& v, smart_pointer<Node_base>& parent_v) {
        (parent_v->l.get() == v.get() ? parent_v->l : parent_v->r) = (v->l ? v->l : v->r);
    }

public:
    struct iterator;

    persistent_set() : root(new Node_base()) {}

    persistent_set(persistent_set const& other) noexcept : root(other.root) {}

    persistent_set(persistent_set&& other) noexcept : root(other.root) {
        other.root = nullptr;
    }

    persistent_set& operator=(persistent_set const& rhs) noexcept {
        root = rhs.root;
        return *this;
    }

    persistent_set& operator=(persistent_set&& rhs) noexcept {
        root = rhs.root;
        rhs.root = nullptr;
        return *this;
    }

    iterator find(T x) const noexcept {
        auto tmp = downhill(root, x);
        if (!get<0>(tmp)->is_dummy() && dynamic_cast<Node*>(get<0>(tmp).get())->val == x)
            return iterator(this, get<0>(tmp).get());
        return iterator(this, root.get());
    }

    std::pair<iterator, bool> insert(T x) {
        auto tmp = downhill(root, x, true);
        root = get<2>(tmp);
        if (!get<0>(tmp)->is_dummy() && dynamic_cast<Node*>(get<0>(tmp).get())->val == x) {
            return {iterator(this, get<0>(tmp).get()), false};
        }
        if (get<0>(tmp)->is_dummy() || x < dynamic_cast<Node*>(get<0>(tmp).get())->val) {
            get<0>(tmp)->l = smart_pointer<Node_base>(new Node(x));
            return {iterator(this, get<0>(tmp)->l.get()), true};
        } else {
            get<0>(tmp)->r = smart_pointer<Node_base>(new Node(x));
            return {iterator(this, get<0>(tmp)->r.get()), true};
        }
    };

    void erase(iterator it) {
        assert(dynamic_cast<Node*>(it.ptr));
        auto tmp = downhill(root, dynamic_cast<Node*>(it.ptr)->val, true);
        root = get<2>(tmp);
        smart_pointer<Node_base>& v = get<0>(tmp);
        smart_pointer<Node_base>& parent_v = get<1>(tmp);
        if (!(v->l && v->r))
            remove_light(v, parent_v);
        else
        {
            T next = dynamic_cast<Node*>(get_next(v.get()))->val;
            auto down_to_next = downhill(v->r, next, true, v);
            v->r = get<2>(down_to_next);
            dynamic_cast<Node*>(v.get())->val = std::move(next);
            remove_light(get<0>(down_to_next), get<1>(down_to_next));
        }
    }

    iterator begin() const noexcept {
        node_ptr cur = root.get();
        for (; cur->l; cur = cur->l.get());
        return iterator(this, cur);
    }

    iterator end() const noexcept {
        return iterator(this, root.get());
    }

    friend void swap(persistent_set& a, persistent_set& b) noexcept {
        std::swap(a.root, b.root);
    }
};

template<typename T, template <typename> class smart_pointer>
struct persistent_set<T, smart_pointer>::Node_base
{
    smart_pointer<Node_base> l;
    smart_pointer<Node_base> r;

    Node_base() : l(nullptr), r(nullptr) {}
    Node_base(Node_base const & other) : l(other.l), r(other.r) {}

    virtual bool is_dummy() {
        return true;
    }

    virtual ~Node_base() {}
};

template<typename T, template <typename> class smart_pointer>
struct persistent_set<T, smart_pointer>::Node : Node_base
{
    T val;

    Node(Node const & other) : Node_base(other), val(other.val) {}

    Node(T val) : Node_base(), val(val) {}

    bool is_dummy() {
        return false;
    }
};

template<typename T, template <typename> class smart_pointer>
struct persistent_set<T, smart_pointer>::iterator
{
    persistent_set const* owner;
    node_ptr ptr;

    iterator(persistent_set const* owner, node_ptr ptr) noexcept : owner(owner), ptr(ptr) {}

    T const& operator*() const noexcept {
        return dynamic_cast<Node*>(ptr)->val;
    }

    iterator& operator++() noexcept {
        ptr = owner->get_next(ptr);
        return *this;
    }

    iterator& operator--() noexcept {
        ptr = owner->get_prev(ptr);
        return *this;
    }

    iterator operator++(int) noexcept {
        iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    iterator operator--(int) noexcept {
        iterator tmp(*this);
        --(*this);
        return tmp;
    }

    friend bool operator==(iterator const & a, iterator const & b) noexcept {
        return a.ptr == b.ptr;
    }

    friend bool operator!=(iterator const & a, iterator const & b) noexcept {
        return a.ptr != b.ptr;
    }

    friend void swap(iterator& a, iterator& b) noexcept {
        std::swap(a.owner, b.owner);
        std::swap(a.ptr, b.ptr);
    }
};
#endif //PERSISTENT_SET_PERSISTENT_SET_H