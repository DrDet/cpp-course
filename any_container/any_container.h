//
// Created by drden on 20.01.2018.
//

#ifndef ANY_CONTAINER_ANY_CONTAINER_H
#define ANY_CONTAINER_ANY_CONTAINER_H

#include <cstddef>
#include <type_traits>
#include <cassert>

#include "any_iterator.h"
using any_iterator_impl::any_iterator;

constexpr size_t SMALL_SIZE = 64;
typedef std::aligned_storage<SMALL_SIZE, SMALL_SIZE>::type storage_t;

template<typename T>
struct is_small {
    static constexpr bool value = sizeof(T) <= SMALL_SIZE && (SMALL_SIZE % alignof(T) == 0) &&
                                  std::is_nothrow_move_constructible<T>::value;
};

template<typename T, typename IT_TAG>
struct impl_functions {
    template <typename CONTAINER>
    static typename std::enable_if<is_small<CONTAINER>::value, CONTAINER&>::type
    get(storage_t& _this) {
        return reinterpret_cast<CONTAINER&>(_this);
    }

    template <typename CONTAINER>
    static typename std::enable_if<!is_small<CONTAINER>::value, CONTAINER&>::type
    get(storage_t& _this) {
        return *reinterpret_cast<CONTAINER*&>(_this);
    }

    template <typename CONTAINER>
    static typename std::enable_if<is_small<CONTAINER>::value, CONTAINER const &>::type
    get(storage_t const & _this) {
        return reinterpret_cast<CONTAINER const&>(_this);
    }

    template <typename CONTAINER>
    static typename std::enable_if<!is_small<CONTAINER>::value, CONTAINER const &>::type
    get(storage_t const & _this) {
        return *reinterpret_cast<CONTAINER const* const&>(_this);
    }

    template <typename CONTAINER>
    static any_iterator<T, IT_TAG>
    _begin(storage_t& _this) {
        return any_iterator<T, IT_TAG>(get<CONTAINER>(_this).begin());
    };

    template <typename CONTAINER>
    static any_iterator<T, IT_TAG>
    _end(storage_t& _this) {
        return any_iterator<T, IT_TAG>(get<CONTAINER>(_this).end());
    };

    template <typename CONTAINER>
    static any_iterator<const T, IT_TAG>
    _cbegin(storage_t const & _this) {
        return any_iterator<const T, IT_TAG>(get<CONTAINER>(const_cast<storage_t&>(_this)).begin());
    };

    template <typename CONTAINER>
    static any_iterator<const T, IT_TAG>
    _cend(storage_t const & _this) {
        return any_iterator<const T, IT_TAG>(get<CONTAINER>(const_cast<storage_t&>(_this)).end());
    };

    template<typename CONTAINER>
    static size_t
    _get_size(storage_t const& _this) {
        return get<CONTAINER>(_this).size();
    }

    template<typename CONTAINER>
    static typename std::enable_if<is_small<CONTAINER>::value>::type
    _destructor(storage_t& _this) {
        reinterpret_cast<CONTAINER&>(_this).~CONTAINER();
    }

    template<typename CONTAINER>
    static typename std::enable_if<!is_small<CONTAINER>::value>::type
    _destructor(storage_t& _this) {
        delete reinterpret_cast<CONTAINER*&>(_this);
    }

    template <typename CONTAINER>
    static any_iterator<T, IT_TAG>
    _insert(storage_t& _this, any_iterator<const T, IT_TAG> it, T const & val) {
        assert(it.get_ops() == _cend<CONTAINER>(_this).get_ops());
        typename CONTAINER::const_iterator pos;
        if (any_iterator_impl::is_small<typename CONTAINER::const_iterator>::value) {
            pos = reinterpret_cast<typename CONTAINER::const_iterator&>(it.get_data());
        } else {
            pos = *reinterpret_cast<typename CONTAINER::const_iterator*&>(it.get_data());
        }
        return any_iterator<T, IT_TAG>(get<CONTAINER>(_this).insert(pos, val));
    };

    template <typename CONTAINER>
    static any_iterator<T, IT_TAG>
    _erase(storage_t& _this, any_iterator<const T, IT_TAG> it) {
        assert(it.get_ops() == _cend<CONTAINER>(_this).get_ops());
        typename CONTAINER::const_iterator pos;
        if (any_iterator_impl::is_small<typename CONTAINER::const_iterator>::value)
            pos = reinterpret_cast<typename CONTAINER::const_iterator&>(it.get_data());
        else
            pos = *reinterpret_cast<typename CONTAINER::const_iterator*&>(it.get_data());
        return any_iterator<T, IT_TAG>(get<CONTAINER>(_this).erase(pos));
    };

    template <typename CONTAINER>
    static typename std::enable_if<is_small<CONTAINER>::value>::type
    _copy(storage_t& _this, storage_t const & other) {
        new (&reinterpret_cast<CONTAINER&>(_this)) CONTAINER(reinterpret_cast<CONTAINER const&>(other));
    }

    template <typename CONTAINER>
    static typename std::enable_if<!is_small<CONTAINER>::value>::type
    _copy(storage_t& _this, storage_t const & other) {
        reinterpret_cast<CONTAINER*&>(_this) = new CONTAINER(get<CONTAINER>(other));
    }

    template<typename CONTAINER>
    static typename std::enable_if<is_small<CONTAINER>::value>::type
    _move(storage_t& _this, storage_t&& other) {
        new (&get<CONTAINER>(_this)) CONTAINER(std::move(get<CONTAINER>(other)));
    }

    template<typename CONTAINER>
    static typename std::enable_if<!is_small<CONTAINER>::value>::type
    _move(storage_t& _this, storage_t && other) {
        reinterpret_cast<CONTAINER*&>(_this) = reinterpret_cast<CONTAINER*&>(other);
        reinterpret_cast<CONTAINER*&>(other) = nullptr;
    }

    static any_iterator<T, IT_TAG> empty_begin(storage_t&) { assert(false); }
    static any_iterator<T, IT_TAG> empty_end(storage_t&) { assert(false); }
    static any_iterator<const T, IT_TAG> empty_cbegin(storage_t const &) { assert(false); };
    static any_iterator<const T, IT_TAG> empty_cend(storage_t const &) { assert(false); };
    static size_t empty_get_size(storage_t const &) { assert(false); };
    static void empty_destructor(storage_t&) { };
    static any_iterator<T, IT_TAG> empty_insert(storage_t&, any_iterator<const T, IT_TAG>, T const &) { assert(false); };
    static any_iterator<T, IT_TAG> empty_erase(storage_t&, any_iterator<const T, IT_TAG>) { assert(false); };
    static void empty_copy(storage_t&, storage_t const &) { assert(false); };
    static void empty_move(storage_t&, storage_t&&) { assert(false); };
};

template <typename T, typename IT_TAG>
struct funcs {
    using begin_t = any_iterator<T, IT_TAG> (*)(storage_t&);
    using end_t = any_iterator<T, IT_TAG> (*)(storage_t&);
    using cbegin_t = any_iterator<const T, IT_TAG> (*)(storage_t const &);
    using cend_t = any_iterator<const T, IT_TAG> (*)(storage_t const &);
    using get_size_t = size_t(*)(storage_t const &);
    using destructor_t = void(*)(storage_t&);
    using insert_t = any_iterator<T, IT_TAG> (*)(storage_t&, any_iterator<const T, IT_TAG>, T const &);
    using erase_t = any_iterator<T, IT_TAG> (*)(storage_t&, any_iterator<const T, IT_TAG>);

    using copy_t = void (*)(storage_t&, storage_t const &);
    using move_t = void (*)(storage_t&, storage_t&&);

    begin_t _begin;
    end_t _end;
    cbegin_t _cbegin;
    cend_t _cend;
    get_size_t _get_size;
    destructor_t _destructor;
    insert_t _insert;
    erase_t _erase;
    copy_t _copy;
    move_t _move;

    funcs(begin_t _begin, end_t _end, cbegin_t _cbegin, cend_t _cend, get_size_t _get_size, destructor_t _destructor, insert_t _insert, erase_t _erase, copy_t _copy, move_t _move)
            : _begin(_begin), _end(_end), _cbegin(_cbegin), _cend(_cend), _get_size(_get_size), _destructor(_destructor), _insert(_insert), _erase(_erase), _copy(_copy), _move(_move)
    {}

    template <typename CONTAINER>
    static funcs const * get_instance() {
        static const funcs instance(
                impl_functions<T, IT_TAG>::template _begin<CONTAINER>,
                impl_functions<T, IT_TAG>::template _end<CONTAINER>,
                impl_functions<T, IT_TAG>::template _cbegin<CONTAINER>,
                impl_functions<T, IT_TAG>::template _cend<CONTAINER>,
                impl_functions<T, IT_TAG>::template _get_size<CONTAINER>,
                impl_functions<T, IT_TAG>::template _destructor<CONTAINER>,
                impl_functions<T, IT_TAG>::template _insert<CONTAINER>,
                impl_functions<T, IT_TAG>::template _erase<CONTAINER>,
                impl_functions<T, IT_TAG>::template _copy<CONTAINER>,
                impl_functions<T, IT_TAG>::template _move<CONTAINER>
        );
        return &instance;
    }

    static funcs const * get_empty_instance() {
        static const funcs empty_instance(
                impl_functions<T, IT_TAG>::empty_begin,
                impl_functions<T, IT_TAG>::empty_end,
                impl_functions<T, IT_TAG>::empty_cbegin,
                impl_functions<T, IT_TAG>::empty_cend,
                impl_functions<T, IT_TAG>::empty_get_size,
                impl_functions<T, IT_TAG>::empty_destructor,
                impl_functions<T, IT_TAG>::empty_insert,
                impl_functions<T, IT_TAG>::empty_erase,
                impl_functions<T, IT_TAG>::empty_copy,
                impl_functions<T, IT_TAG>::empty_move
        );
        return &empty_instance;
    }
};

template <typename T, typename IT_TAG>
class any_container {
private:
    storage_t _this;
    funcs<T, IT_TAG> const * fs;

    template <typename CONTAINER>
    typename std::enable_if<is_small<CONTAINER>::value>::type
    get_this(CONTAINER const & other) {
//        std::cerr << "small";
        new (&_this) CONTAINER(other);
    }

    template <typename CONTAINER>
    typename std::enable_if<!is_small<CONTAINER>::value>::type
    get_this(CONTAINER const & other) {
//        std::cerr << "big";
        reinterpret_cast<CONTAINER*&>(_this) = new CONTAINER(other);
    }

public:
    any_container() : fs(funcs<T, IT_TAG>::get_empty_instance()) {}

    any_container(any_container const & other) : fs(other.fs) {
        fs->_copy(_this, other._this);
    }

    any_container(any_container&& other) : fs(other.fs) {
        fs->_move(_this, std::move(other._this));
    }

    any_container& operator=(const any_container& other){
        any_container tmp(other);
        swap(tmp);
        return *this;
    }

    any_container& operator=(any_container&& other) noexcept{
        if(this == &other) {
            return *this;
        }
        fs->_destructor(_this);
        fs = other.fs;
        fs->_move(_this, std::move(other._this));
        return *this;
    }

    void swap(any_container &other) noexcept {
        any_container tmp(std::move(other));
        other = std::move(*this);
        *this = std::move(tmp);
    }

    template<typename CONTAINER>
    any_container(CONTAINER const & other) : fs(funcs<T, IT_TAG>::template get_instance<CONTAINER>()) {
        get_this(other);
    }

    any_iterator<T, IT_TAG> begin() {
        return fs->_begin(_this);
    }

    any_iterator<T, IT_TAG> end() {
        return fs->_end(_this);
    };

    any_iterator<const T, IT_TAG> cbegin() const {
        return fs->_cbegin(_this);
    };

    any_iterator<const T, IT_TAG> cend() const {
        return fs->_cend(_this);
    };

    size_t size() const {
        return fs->_get_size(_this);
    }

    any_iterator<T, IT_TAG> insert(any_iterator<const T, IT_TAG> it, T const & val) {
        return fs->_insert(_this, it, val);
    };

    any_iterator<T, IT_TAG> erase(any_iterator<const T, IT_TAG> it) {
        return fs->_erase(_this, it);
    };

    ~any_container() {
        fs->_destructor(_this);
    }
};

//

#endif //ANY_CONTAINER_ANY_CONTAINER_H
