//
// Created by drden on 09.01.2018.
//

#ifndef PERSISTENT_SET_LINKED_PTR_H
#define PERSISTENT_SET_LINKED_PTR_H

#include <cstddef>
#include <algorithm>

template <typename T>
class linked_ptr
{
private:
    T* ptr;
    mutable linked_ptr const* prev;
    mutable linked_ptr const* next;

    void add_to(linked_ptr const & cur) {
        if (!cur.ptr) {
            prev = nullptr;
            next = nullptr;
            return;
        }
        prev = &cur;
        next = cur.next;
        cur.next = this;
        if (next)
            next->prev = this;
    }

    void remove() {
        if (!ptr) {
            assert(!prev & !next);
            return;
        }
        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;
        if (!prev && !next)
            delete ptr;
    }
public:
    linked_ptr() noexcept : ptr(nullptr), prev(nullptr), next(nullptr) {}
    linked_ptr(T* new_ptr) noexcept : ptr(new_ptr), prev(nullptr), next(nullptr) {}

    linked_ptr(std::nullptr_t const &) noexcept : linked_ptr() {}

    linked_ptr(linked_ptr const & other) noexcept : ptr(other.ptr) {
        add_to(other);
    }

    linked_ptr(linked_ptr&& other) noexcept : ptr(other.ptr), prev(other.prev), next(other.next) {
        if (other.prev) other.prev->next = this;
        if (other.next) other.next->prev = this;
        other.ptr = nullptr;
        other.prev = other.next = nullptr;
    }

    linked_ptr& operator=(std::nullptr_t const &) noexcept {
        remove();
        ptr = nullptr;
        prev = next = nullptr;
        return *this;
    }

    linked_ptr& operator=(linked_ptr const & rhs) noexcept {
        if (this == &rhs) return *this;
        remove();
        ptr = rhs.ptr;
        add_to(rhs);
        return *this;
    }

    linked_ptr& operator=(linked_ptr&& rhs) noexcept {
        if (this == &rhs) return *this;
        remove();
        ptr = rhs.ptr;
        prev = rhs.prev;
        next = rhs.next;
        if (rhs.prev) rhs.prev->next = this;
        if (rhs.next) rhs.next->prev = this;
        rhs.ptr = nullptr;
        rhs.prev = rhs.next = nullptr;
        return *this;
    }

    operator bool() const noexcept {
        return ptr != nullptr;
    }

    T operator*() const noexcept {
        return *ptr;
    }

    T* operator->() const noexcept {
        return ptr;
    }

    T* get() const noexcept {
        return ptr;
    }

    ~linked_ptr() {
        remove();
    }

    friend void swap(linked_ptr& a, linked_ptr& b) noexcept {
        if (&a == &b) return;
        if (a.next == &b || b.next == &a) {
            linked_ptr& x = (b.next == &a ? b : a);
            linked_ptr& y = (b.next == &a ? a : b);
            if (x.prev) x.prev->next = &y;
            if (y.next) y.next->prev = &x;
            x.next = y.next;
            y.prev = x.prev;
            x.prev = &y;
            y.next = &x;
            return;
        }
        std::swap(a.ptr, b.ptr);
        if (a.prev) a.prev->next = &b;
        if (a.next) a.next->prev = &b;
        if (b.prev) b.prev->next = &a;
        if (b.next) b.next->prev = &a;
        std::swap(a.prev, b.prev);
        std::swap(a.next, b.next);
    }
};

#endif //PERSISTENT_SET_LINKED_PTR_H
