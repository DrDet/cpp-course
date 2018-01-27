//
// Created by drden on 20.01.2018.
//

#ifndef ANY_CONTAINER_ANY_ITERATOR_H
#define ANY_CONTAINER_ANY_ITERATOR_H

#include <type_traits>
#include <cstddef>
#include <iterator>

namespace any_iterator_impl {
    using std::endl;
    using std::is_same;
    using std::is_base_of;
    using std::enable_if;

//template <bool flag>
//using enable_if_t = std::enable_if<flag>::type;

//template <class T, class U> constexpr bool is_same_v
//        = std::is_same<T, U>::value;
//
//template <class Base, class Derived> constexpr bool is_base_of_v
//        = std::is_base_of<Base, Derived>::value;

    struct bad_iterator : std::exception {
    };

    static constexpr size_t SIZE = sizeof(void *);
    static constexpr size_t ALIGN = sizeof(void *);

    typedef std::aligned_storage<SIZE, ALIGN>::type storage_t;

    template <typename T, typename TAG>
    struct any_iterator;

    template<typename I>
    struct is_small {
        static constexpr bool value =
                sizeof(I) <= SIZE && alignof(I) <= ALIGN && std::is_nothrow_constructible<I>::value;
    };

//    template<typename T>
//    struct is_any_iterator {
//        static constexpr bool value = false;
//    };
//
//    template<typename U, typename V>
//    struct is_any_iterator<any_iterator<U, V>> {
//        static constexpr bool value = true;
//    };

    template<typename T>
    struct is_const_pointer {
        static constexpr bool value = false;
    };

    template<typename T>
    struct is_const_pointer<const T*> {
        static constexpr bool value = true;
    };

//template<typename I>
//inline constexpr bool is_small_v = is_small<I>::value;

    template<typename T>
    struct base_ops {
        template<typename I>
        static typename enable_if<is_small<I>::value>::type copy(storage_t const &from, storage_t &to) {
            new(&reinterpret_cast<I &>(to)) I(reinterpret_cast<I const &>(from));
        }

        template<typename I>
        static typename enable_if<!is_small<I>::value>::type copy(storage_t const &from, storage_t &to) {
            reinterpret_cast<I *&>(to) = new I(get<I>(from));
        }

        template<typename I>
        static typename enable_if<is_small<I>::value>::type mov(storage_t &&from, storage_t &to) {
            new(&get<I>(to)) I(std::move(get<I>(from)));
        }

        template<typename I>
        static typename enable_if<!is_small<I>::value>::type mov(storage_t &&from, storage_t &to) {
            reinterpret_cast<I *&>(to) = reinterpret_cast<I *&>(from);
//        new(&get<I>(to)) I(std::move(get<I>(from)));
            reinterpret_cast<I *&>(from) = nullptr;
        }

        template<typename I>
        static typename enable_if<is_small<I>::value>::type del(storage_t &st) {
            reinterpret_cast <I &>(st).~I();
        }

        template<typename I>
        static typename enable_if<!is_small<I>::value>::type del(storage_t &st) {
            delete (reinterpret_cast <I *&>(st));
        }

        template<typename I>
        static typename enable_if<is_small<I>::value, I &>::type get(storage_t &st) {
            return reinterpret_cast<I &>(st);
        }

        template<typename I>
        static typename enable_if<is_small<I>::value, I const &>::type get(storage_t const &st) {
            return reinterpret_cast<I const &>(st);
        }

        template<typename I>
        static typename enable_if<!is_small<I>::value, I &>::type get(storage_t &st) {
            return *reinterpret_cast<I *&>(st);
        }

        template<typename I>
        static typename enable_if<!is_small<I>::value, I const &>::type get(storage_t const &st) {
            return *reinterpret_cast<I *const &>(st);
        }

        template<typename I>
        static bool is_equal(storage_t const &a, storage_t const &b) {
            return get<I>(a) == get<I>(b);
        }

        template<typename I>
        static bool is_less(storage_t const &a, storage_t const &b) {
            return get<I>(a) < get<I>(b);
        }

        template<typename I>
        static ptrdiff_t diff(storage_t const &a, storage_t const &b) {
            return get<I>(a) - get<I>(b);
        }

        template<typename I>
        static T &get_value(storage_t const &st) {
            return const_cast<T&>(*get<I>(st));
        }

        template<typename I>
        static void inc(storage_t &st) {
            ++get<I>(st);
        }

        template<typename I>
        static void dec(storage_t &st) {
            --get<I>(st);
        }

        template<typename I>
        static void add_n(storage_t &st, ptrdiff_t n) {
            get<I>(st) += n;
        }

        static void copy_default(storage_t const &, storage_t &) {}

        static void mov_default(storage_t &&, storage_t &) {}

        static void del_default(storage_t &) {}

        static bool is_equal_default(storage_t const &, storage_t const &) { throw bad_iterator(); }

        static bool is_less_default(storage_t const &, storage_t const &) { throw bad_iterator(); }

        static ptrdiff_t diff_default(storage_t const &, storage_t const &) { throw bad_iterator(); }

        static T &get_value_default(storage_t const &) { throw bad_iterator(); }

        static void inc_default(storage_t &) { throw bad_iterator(); }

        static void dec_default(storage_t &) { throw bad_iterator(); }

        static void add_n_default(storage_t &, ptrdiff_t n) { throw bad_iterator(); }
    };


    template<typename T, typename Tag>
    struct func_ops;

    template<typename T>
    struct func_ops<T, std::forward_iterator_tag> {
        using copy_t = void (*)(storage_t const &, storage_t &);
        using move_t = void (*)(storage_t &&, storage_t &);
        using del_t = void (*)(storage_t &);
        using is_equal_t = bool (*)(storage_t const &, storage_t const &);
        //    using is_less_t = bool (*)(storage_t const&, storage_t const&);
        //    using diff_t = ptrdiff_t (*)(storage_t const&, storage_t const&);
        using get_value_t = T &(*)(storage_t const &);
        using inc_t = void (*)(storage_t &);
        //    using dec_t = void (*)(storage_t &);
        //    using add_n_t = void (*)(storage_t &);

        copy_t copier;
        move_t mover;
        del_t deleter;
        is_equal_t eq_checker;
        get_value_t value_getter;
        inc_t incrementer;

        func_ops(copy_t copier, move_t mover, del_t deleter, is_equal_t eq_checker, get_value_t value_getter,
                 inc_t incrementer) :
                copier(copier), mover(mover), deleter(deleter), eq_checker(eq_checker), value_getter(value_getter),
                incrementer(incrementer) {}

        template<typename I>
        static const func_ops *get_func_ops() {
            static const func_ops instance = {
                    base_ops<T>::template copy<I>,
                    base_ops<T>::template mov<I>,
                    base_ops<T>::template del<I>,
                    base_ops<T>::template is_equal<I>,
                    base_ops<T>::template get_value<I>,
                    base_ops<T>::template inc<I>
            };
            return &instance;
        }

        static const func_ops *get_default_func_ops() {
            static const func_ops instance = {
                    base_ops<T>::copy_default,
                    base_ops<T>::mov_default,
                    base_ops<T>::del_default,
                    base_ops<T>::is_equal_default,
                    base_ops<T>::get_value_default,
                    base_ops<T>::inc_default
            };
            return &instance;
        }
    };

    template<typename T>
    struct func_ops<T, std::bidirectional_iterator_tag> {
        using copy_t = void (*)(storage_t const &, storage_t &);
        using move_t = void (*)(storage_t &&, storage_t &);
        using del_t = void (*)(storage_t &);
        using is_equal_t = bool (*)(storage_t const &, storage_t const &);
        //    using is_less_t = bool (*)(storage_t const&, storage_t const&);
        //    using diff_t = ptrdiff_t (*)(storage_t const&, storage_t const&);
        using get_value_t = T &(*)(storage_t const &);
        using inc_t = void (*)(storage_t &);
        using dec_t = void (*)(storage_t &);
        //    using add_n_t = void (*)(storage_t &);
        copy_t copier;
        move_t mover;
        del_t deleter;
        is_equal_t eq_checker;
        get_value_t value_getter;
        inc_t incrementer;
        dec_t decrementer;

        func_ops(copy_t copier, move_t mover, del_t deleter, is_equal_t eq_checker, get_value_t value_getter,
                 inc_t incrementer,
                 dec_t decrementer) :
                copier(copier), mover(mover), deleter(deleter), eq_checker(eq_checker), value_getter(value_getter),
                incrementer(incrementer),
                decrementer(decrementer) {}

        template<typename I>
        static const func_ops *get_func_ops() {
            static const func_ops instance = {
                    base_ops<T>::template copy<I>,
                    base_ops<T>::template mov<I>,
                    base_ops<T>::template del<I>,
                    base_ops<T>::template is_equal<I>,
                    base_ops<T>::template get_value<I>,
                    base_ops<T>::template inc<I>,
                    base_ops<T>::template dec<I>
            };
            return &instance;
        }

        static const func_ops *get_default_func_ops() {
            static const func_ops instance = {
                    base_ops<T>::copy_default,
                    base_ops<T>::mov_default,
                    base_ops<T>::del_default,
                    base_ops<T>::is_equal_default,
                    base_ops<T>::get_value_default,
                    base_ops<T>::inc_default,
                    base_ops<T>::dec_default
            };
            return &instance;
        }
    };

    template<typename T>
    struct func_ops<T, std::random_access_iterator_tag> {
        using copy_t = void (*)(storage_t const &, storage_t &);
        using move_t = void (*)(storage_t &&, storage_t &);
        using del_t = void (*)(storage_t &);
        using is_equal_t = bool (*)(storage_t const &, storage_t const &);
        using is_less_t = bool (*)(storage_t const &, storage_t const &);
        using diff_t = ptrdiff_t (*)(storage_t const &, storage_t const &);
        using get_value_t = T &(*)(storage_t const &);
        using inc_t = void (*)(storage_t &);
        using dec_t = void (*)(storage_t &);
        using add_n_t = void (*)(storage_t &, ptrdiff_t n);
        copy_t copier;
        move_t mover;
        del_t deleter;
        is_equal_t eq_checker;
        is_less_t less_checker;
        diff_t differ;
        get_value_t value_getter;
        inc_t incrementer;
        dec_t decrementer;
        add_n_t adder;

        func_ops(copy_t copier, move_t mover, del_t deleter, is_equal_t eq_checker, is_less_t less_checker,
                 diff_t differ, get_value_t value_getter,
                 inc_t incrementer, dec_t decrementer, add_n_t adder) :
                copier(copier), mover(mover), deleter(deleter), eq_checker(eq_checker), less_checker(less_checker),
                differ(differ),
                value_getter(value_getter), incrementer(incrementer), decrementer(decrementer), adder(adder) {}

        template<typename I>
        static const func_ops *get_func_ops() {
            static const func_ops instance = {
                    base_ops<T>::template copy<I>,
                    base_ops<T>::template mov<I>,
                    base_ops<T>::template del<I>,
                    base_ops<T>::template is_equal<I>,
                    base_ops<T>::template is_less<I>,
                    base_ops<T>::template diff<I>,
                    base_ops<T>::template get_value<I>,
                    base_ops<T>::template inc<I>,
                    base_ops<T>::template dec<I>,
                    base_ops<T>::template add_n<I>
            };
            return &instance;
        }

        static const func_ops *get_default_func_ops() {
            static const func_ops instance{
                    base_ops<T>::copy_default,
                    base_ops<T>::mov_default,
                    base_ops<T>::del_default,
                    base_ops<T>::is_equal_default,
                    base_ops<T>::is_less_default,
                    base_ops<T>::diff_default,
                    base_ops<T>::get_value_default,
                    base_ops<T>::inc_default,
                    base_ops<T>::dec_default,
                    base_ops<T>::add_n_default
            };
            return &instance;
        }
    };

    template<typename T, typename Tag>
    struct any_iterator;

    template<typename T, typename Tag>
    struct any_iterator_base {
    };

    template<typename T>
    struct any_iterator_base<T, std::random_access_iterator_tag> {
        using derived = any_iterator<T, std::random_access_iterator_tag>;

        T &operator[](ptrdiff_t n) const {
            return *(static_cast<const derived &>(*this) + n);
        }
    };

    template<typename T, typename Tag>
    struct any_iterator : any_iterator_base<T, Tag> {
        template <typename, typename> friend struct any_iterator;
    private:
//        typedef func_ops<T, Tag> func_ops<T, Tag>;
        using func_ops_t = func_ops<typename std::remove_const<T>::type, Tag>;
        const func_ops_t *ops;
        storage_t data;

        template<typename I>
        typename enable_if<is_small<I>::value>::type ctor(I it) {
            new(&data) I(std::move(it));
        }

        template<typename I>
        typename enable_if<!is_small<I>::value>::type ctor(I it) {
//        cout << *it << endl;
            reinterpret_cast<I *&>(data) = new I(std::move(it));
        }

    public:

        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef T *pointer;
        typedef T &reference;
        typedef Tag iterator_category;

        storage_t& get_data() {
            return data;
        }

        const func_ops_t * get_ops() const {
            return ops;
        }

        any_iterator() noexcept:
                ops(func_ops_t::get_default_func_ops()) {}

        any_iterator(const any_iterator &other) :
                ops(other.ops) {
            ops->copier(other.data, data);
        }

        template <typename U, typename = typename
        std::enable_if<
                std::is_same<
                        typename std::remove_const<T>::type,
                        typename std::remove_const<U>::type>
                ::value &&
                std::is_const<T>::value &&
                !std::is_const<U>::value>::type>
        any_iterator(any_iterator<U, Tag> const & other) : ops(other.ops) {
            ops->copier(other.data, data);
        };

        any_iterator(any_iterator &&other) :
                ops(other.ops) {
            ops->mover(std::move(other.data), data);
        }

//        template<typename I, typename =
//        typename std::enable_if<
//               std::is_convertible<typename std::iterator_traits<typename std::decay<I>::type>::iterator_category*, Tag*>::value &&
//                !is_any_iterator<typename std::decay<I>::type>::value &&
//                !is_const_pointer<typename std::iterator_traits<std::remove_reference<I>::type>::pointer>::value || std::is_const<T>::value
//        >
//        ::type>
//        any_iterator(I it) noexcept(is_small<I>::value):
//                ops(func_ops_t::template get_func_ops<std::decay<I>::type>()) {
//            ctor(std::move(it));
//        }

        template<typename I, typename = typename std::enable_if<
                !is_const_pointer<typename std::iterator_traits<typename std::remove_reference<I>::type>::pointer>::value || std::is_const<T>::value
                >::type
        >
        any_iterator(I it) noexcept(is_small<I>::value):
                ops(func_ops_t::template get_func_ops<I>()) {
            ctor(std::move(it));
        }

//        template<typename I>
//        any_iterator(I&& it, typename std::enable_if<
//                std::is_convertible<typename std::iterator_traits<typename std::decay<I>::type>::iterator_category*, Tag*>::value
//                && !is_any_iterator<typename std::decay<I>::type>::value
//                && (!is_const_pointer<typename std::iterator_traits<std::remove_reference_t<I>>::pointer>::value || std::is_const_v<T>)
//        >::type* = nullptr)
//        noexcept(is_small_v<std::decay_t<I>>):
//        ops(func_ops_t::template get_func_ops<std::decay_t<I>>())
//                {
//                        inner_construct<std::decay_t<I>>(data, std::forward<I>(it));
//                }

        any_iterator &operator=(const any_iterator &other) {
            any_iterator tmp(other);
            swap(tmp);
            return *this;
        }

        void swap(any_iterator &other) noexcept {
            any_iterator tmp(std::move(other));
            other = std::move(*this);
            *this = std::move(tmp);
        }

        any_iterator &operator=(any_iterator &&other) noexcept {
            if (this == &other) {
                return *this;
            }
            ops->deleter(data);
            ops = other.ops;
            ops->mover(std::move(other.data), data);
            return *this;
        }

        template<typename I>
        any_iterator &operator=(I it) noexcept(is_small<I>::value) {
            *this = any_iterator(std::move(it));
            return *this;
        }

        ~any_iterator() {
            ops->deleter(data);
        }

        explicit operator bool() const noexcept {
            return ops != func_ops_t::get_default_func_ops();
        }

        T &operator*() const {
            return ops->value_getter(data);
        }

        T *operator->() const {
            return &ops->value_getter(data);
        }

        any_iterator &operator++() {
            ops->incrementer(data);
            return *this;
        }

        any_iterator operator++(int) {
            any_iterator tmp(*this);
            ops->incrementer(data);
            return tmp;
        }

        template<typename T2, typename Tag2, typename Cond>
        friend ptrdiff_t operator-(const any_iterator<T2, Tag2> &a, const any_iterator<T2, Tag2> &b);

        template<typename T2, typename Tag2, typename Cond>
        friend any_iterator<T2, Tag2> &operator--(any_iterator<T2, Tag2> &a);

        template<typename T2, typename Tag2, typename Cond>
        friend any_iterator<T2, Tag2> operator--(any_iterator<T2, Tag2> &a, int);

        template<typename T2, typename Tag2, typename Cond>
        friend any_iterator<T2, Tag2> &operator+=(any_iterator<T2, Tag2> &it, ptrdiff_t n);

        template<typename T2, typename Tag2, typename Cond>
        friend any_iterator<T2, Tag2> &operator-=(any_iterator<T2, Tag2> &it, ptrdiff_t n);

        template<typename T2, typename Tag2, typename Cond>
        friend any_iterator<T2, Tag2> operator+(any_iterator<T2, Tag2> it, ptrdiff_t n);

        template<typename T2, typename Tag2, typename Cond>
        friend any_iterator<T2, Tag2> operator+(ptrdiff_t n, any_iterator<T2, Tag2> it);

        template<typename T2, typename Tag2, typename Cond>
        friend any_iterator<T2, Tag2> operator-(any_iterator<T2, Tag2> it, ptrdiff_t n);

        template<typename T2, typename Tag2, typename Cond>
        friend bool operator<(const any_iterator<T2, Tag2> &a, const any_iterator<T2, Tag2> &b);

        template<typename T2, typename Tag2, typename Cond>
        friend bool operator<=(const any_iterator<T2, Tag2> &a, const any_iterator<T2, Tag2> &b);

        template<typename T2, typename Tag2, typename Cond>
        friend bool operator>(const any_iterator<T2, Tag2> &a, const any_iterator<T2, Tag2> &b);

        template<typename T2, typename Tag2, typename Cond>
        friend bool operator>=(const any_iterator<T2, Tag2> &a, const any_iterator<T2, Tag2> &b);

        template <typename T1, typename T2, typename IT_TAG1, typename IT_TAG2>
        friend bool operator==(any_iterator<T1, IT_TAG1> const & a, any_iterator<T2, IT_TAG2> const & b);

        template <typename T1, typename T2, typename IT_TAG1, typename IT_TAG2>
        friend bool operator!=(any_iterator<T1, IT_TAG1> const & a, any_iterator<T2, IT_TAG2> const & b);
    };

    template<typename T, typename Tag, typename = typename enable_if<is_same<Tag, std::random_access_iterator_tag>::value>::type>
    any_iterator<T, Tag> &operator+=(any_iterator<T, Tag> &it, ptrdiff_t n) {
        it.ops->adder(it.data, n);
        return it;
    }

    template<typename T, typename Tag, typename = typename enable_if<is_same<Tag, std::random_access_iterator_tag>::value>::type>
    any_iterator<T, Tag> &operator-=(any_iterator<T, Tag> &it, ptrdiff_t n) {
        it.ops->adder(it.data, -n);
        return it;
    }

    template<typename T, typename Tag, typename = typename enable_if<is_base_of<std::bidirectional_iterator_tag, Tag>::value>::type>
    any_iterator<T, Tag> &operator--(any_iterator<T, Tag> &it) {
        it.ops->decrementer(it.data);
        return it;
    }

    template<typename T, typename Tag, typename = typename enable_if<is_base_of<std::bidirectional_iterator_tag, Tag>::value>::type>
    any_iterator<T, Tag> operator--(any_iterator<T, Tag> &it, int) {
        any_iterator<T, Tag> tmp(it);
        it.ops->decrementer(it.data);
        return tmp;
    }

    template<typename T, typename Tag, typename = typename enable_if<is_base_of<std::bidirectional_iterator_tag, Tag>::value>::type>
    ptrdiff_t operator-(const any_iterator<T, Tag> &a, const any_iterator<T, Tag> &b) {
        if (a.ops != b.ops) {
            throw bad_iterator();
        }
        return a.ops->differ(a.data, b.data);
    }

    template<typename T, typename Tag, typename = typename enable_if<is_same<Tag, std::random_access_iterator_tag>::value>::type>
    any_iterator<T, Tag> operator+(any_iterator<T, Tag> it, ptrdiff_t n) {
        return it += n;
    }

    template<typename T, typename Tag, typename = typename enable_if<is_same<Tag, std::random_access_iterator_tag>::value>::type>
    any_iterator<T, Tag> operator-(any_iterator<T, Tag> it, ptrdiff_t n) {
        return it -= n;
    }

    template<typename T, typename Tag, typename = typename enable_if<is_same<Tag, std::random_access_iterator_tag>::value>::type>
    bool operator<(const any_iterator<T, Tag> &a, const any_iterator<T, Tag> &b) {
        if (a.ops != b.ops) {
            throw bad_iterator();
        }
        return a.ops->less_checker(a.data, b.data);
    }

    template<typename T, typename Tag, typename = typename enable_if<is_same<Tag, std::random_access_iterator_tag>::value>::type>
    bool operator>=(const any_iterator<T, Tag> &a, const any_iterator<T, Tag> &b) {
        return !(a < b);
    }

    template<typename T, typename Tag, typename = typename enable_if<is_same<Tag, std::random_access_iterator_tag>::value>::type>
    bool operator>(const any_iterator<T, Tag> &a, const any_iterator<T, Tag> &b) {
        return b < a;
    }

    template<typename T, typename Tag, typename = typename enable_if<is_same<Tag, std::random_access_iterator_tag>::value>::type>
    bool operator<=(const any_iterator<T, Tag> &a, const any_iterator<T, Tag> &b) {
        return !(a > b);
    }

    template <typename T1, typename T2, typename IT_TAG1, typename IT_TAG2>
    bool operator==(any_iterator<T1, IT_TAG1> const & a, any_iterator<T2, IT_TAG2> const & b) {
        return (a.ops == b.ops) && a.ops->eq_checker(a.data, b.data);
    };

    template <typename T1, typename T2, typename IT_TAG1, typename IT_TAG2>
    bool operator!=(any_iterator<T1, IT_TAG1> const & a, any_iterator<T2, IT_TAG2> const & b) {
        return !(a == b);
    };

    template<typename ValueType>
    using any_forward_iterator = any_iterator<ValueType, std::forward_iterator_tag>;

    template<typename ValueType>
    using any_bidirectional_iterator = any_iterator<ValueType, std::bidirectional_iterator_tag>;

    template<typename ValueType>
    using any_random_access_iterator = any_iterator<ValueType, std::random_access_iterator_tag>;
}

#endif //ANY_CONTAINER_ANY_ITERATOR_H
