//
// Created by drden on 15.06.2017.
//

#include <cassert>
#include <cstring>
#include "Vector.h"

using std::make_shared;

void Vector::make_editable() {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    if (isSmall || _data.big.unique())
        return;
    assert(_data.big != nullptr);
    _data.big = make_shared< vector<uit> >(*_data.big);
}

void Vector::make_big() {
    if (!isSmall)
        return;
    assert(_size <= SMALL_SIZE && _data.big == nullptr);
    _data.big = make_shared< vector <uit> >(_size);
    for (size_t i = 0; i < _size; ++i)
        (*_data.big)[i] = _data.small[i];
    isSmall = false;
}

Vector::Vector() : _size(0), isSmall(true) { }


Vector::Vector(Vector const &other) noexcept {
    if (!other.isSmall) {
        _data.big = other._data.big;            ///ref_cnt++
        _size = other._size;
        isSmall &= false;
    } else {        //откатываюсь в small, если надо
        assert(_data.big == nullptr);
        for (size_t i = 0; i < other._size; ++i)
            _data.small[i] = other._data.small[i];
        _size = other._size;
        isSmall |= true;
        _data.big = nullptr;
    }
}

Vector& Vector::operator=(Vector const &other) noexcept {
    if (!other.isSmall) {
        _data.big = other._data.big;            ///ref_cnt++
        _size = other._size;
        isSmall &= false;
    } else {        //откатываюсь в small, если надо
        assert(other._data.big == nullptr);
        for (size_t i = 0; i < other._size; ++i)
            _data.small[i] = other._data.small[i];
        _size = other._size;
        isSmall |= true;
        _data.big = nullptr;
    }
    return *this;
}

size_t Vector::size() const {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    return _size;
}

uit Vector::back() const {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    assert(_size != 0);
    return (isSmall ? _data.small[_size - 1] : (*_data.big)[_size - 1]);
}

void Vector::resize(size_t n) {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    if (isSmall && n <= SMALL_SIZE) {
        for (size_t i = _size; i < SMALL_SIZE; ++i)
            _data.small[i] = 0;
        _size = n;
        return;
    }
    assert(isSmall || _data.big.unique());
    make_big();
    _data.big->resize(n);
    _size = _data.big->size();
}

void Vector::push_back(uit const x) {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    if (isSmall && _size < SMALL_SIZE) {
        _data.small[_size] = x;
        _size++;
        return;
    }
    assert(isSmall || _data.big.unique());
    make_big();
    _data.big->push_back(x);
    _size = _data.big->size();
}

void Vector::pop_back() {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    assert(_size != 0);
    if (isSmall) {
        _size--;
        return;
    }
    assert(isSmall || _data.big.unique());
    _data.big->pop_back();
    _size = _data.big->size();
}

void Vector::push_front(uit const x, size_t n) {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    if (isSmall && _size + n <= SMALL_SIZE) {
        uit buf[SMALL_SIZE];
        std::memcpy(buf, _data.small, sizeof(uit) * _size);
        for (size_t i = 0; i < n; ++i)
            _data.small[i] = x;
        for (size_t i = 0; i < _size; ++i)
            _data.small[i + n] = buf[i];
        _size += n;
        return;
    }
    assert(isSmall || _data.big.unique());
    make_big();
    _data.big->insert(_data.big->begin(), n, x);
    _size = _data.big->size();
}

void Vector::pop_front(size_t n) {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    assert(_size >= n);
    if (isSmall) {
        for (size_t i = 0; i < _size - n; ++i)
            _data.small[i] = _data.small[i + n];
        _size -= n;
        return;
    }
    assert(isSmall || _data.big.unique());
    _data.big->erase(_data.big->begin(), _data.big->begin() + n);
    _size = _data.big->size();
}

uit &Vector::operator[](size_t idx) {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    assert(idx < _size);
    if (isSmall)
        return _data.small[idx];
    assert(isSmall || _data.big.unique());
    return (*_data.big)[idx];
}

uit const &Vector::operator[](size_t idx) const {
    assert(isSmall || _data.big != nullptr);    ///invariant
    assert(!isSmall || _data.big == nullptr);   ///invariant
    assert(idx < _size);
    if (isSmall)
        return _data.small[idx];
    return (*_data.big)[idx];
}