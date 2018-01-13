//
// Created by drden on 07.01.2018.
//

#ifndef PERSISTENT_SET_SHARED_PTR_H
#define PERSISTENT_SET_SHARED_PTR_H

#include <cstddef>
#include <algorithm>

template<typename T>
class shared_ptr
{
private:
    T* ptr;
    size_t* ref_cnt;

    void inc_cnt() {
        if (!ptr) return;
        ++(*ref_cnt);
    }

    void dec_cnt() {
        if (!ptr) return;
        --(*ref_cnt);
        if (*ref_cnt == 0) {
            delete ptr;
            delete ref_cnt;
        }
    }

public:
    shared_ptr() noexcept : ptr(nullptr), ref_cnt(nullptr) {}
    explicit shared_ptr(T* new_ptr) : ptr(new_ptr) {
        try
        {
            ref_cnt = new size_t(1);
        }
        catch (...)
        {
            delete ptr;
            throw;
        }
    }

    shared_ptr(std::nullptr_t const &) noexcept : shared_ptr() {}

    shared_ptr(shared_ptr const & other) noexcept : ptr(other.ptr), ref_cnt(other.ref_cnt) {
        assert(!ref_cnt || *ref_cnt > 0);
        inc_cnt();
    }

    shared_ptr(shared_ptr&& other) noexcept : ptr(other.ptr), ref_cnt(other.ref_cnt) {
        assert(!ref_cnt || *ref_cnt > 0);
        other.ptr = nullptr;
        other.ref_cnt = nullptr;
    }

    shared_ptr& operator=(std::nullptr_t const &) noexcept {
        assert(!ref_cnt || *ref_cnt > 0);
        dec_cnt();
        ptr = nullptr;
        ref_cnt = nullptr;
        return *this;
    }

    shared_ptr& operator=(shared_ptr const & rhs) noexcept {
        assert(!ref_cnt || *ref_cnt > 0);
        if (this == &rhs) return *this;
        dec_cnt();
        ptr = rhs.ptr;
        ref_cnt = rhs.ref_cnt;
        inc_cnt();
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& rhs) noexcept {
        assert(!ref_cnt || *ref_cnt > 0);
        if (this == &rhs) return *this;
        dec_cnt();
        ptr = rhs.ptr;
        ref_cnt = rhs.ref_cnt;
        rhs.ptr = nullptr;
        rhs.ref_cnt = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept {
        assert(!ref_cnt || *ref_cnt > 0);
        return ptr != nullptr;
    }

    T operator*() const noexcept {
        assert(!ref_cnt || *ref_cnt > 0);
        return *ptr;
    }

    T* operator->() const noexcept {
        assert(!ref_cnt || *ref_cnt > 0);
        return ptr;
    }

    T* get() const noexcept {
        assert(!ref_cnt || *ref_cnt > 0);
        return ptr;
    }

    ~shared_ptr() {
        assert(!ref_cnt || *ref_cnt > 0);
        dec_cnt();
    }

    friend void swap(shared_ptr& a, shared_ptr& b) noexcept {
        std::swap(a.ptr, b.ptr);
        std::swap(a.ref_cnt, b.ref_cnt);
    }
};

#endif //PERSISTENT_SET_SHARED_PTR_H
