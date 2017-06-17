#include "big_integer.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

big_integer::big_integer() : sign(false) {

} // assume that == 0, when length == 0

big_integer::big_integer(big_integer const &other) : value(other.value), sign(other.sign) { }

big_integer::big_integer(int a) {
    if (a != 0)
        value.push_back(std::abs(1ll * a));
    sign = (a < 0);
}

big_integer::big_integer(const std::string &str) : sign(false) {
    for (size_t i = (str[0] == '-' ? 1 : 0); i < str.length(); ++i)
        *this = *this * 10 + static_cast<int>(str[i] - '0');
    sign = (str[0] == '-');
    normalise();
}

big_integer &big_integer::operator=(const big_integer &other) {
    value = other.value;
    sign = other.sign;
    return *this;
}

big_integer &big_integer::operator+=(const big_integer &rhs) {
    big_integer l = convert_c2(),              //0000...: +; 1111... : -
                r = rhs.convert_c2();
    size_t len = std::max(l.length(), r.length());
    l.resize(len);
    r.resize(len);
    bool carry = 0;
    for (size_t i = 0; i < len; ++i) {
        uint64_t buf = uint64_t(l[i]) + r[i] + carry;
        l[i] = buf;
        carry = static_cast<bool>(buf >> 31 >> 1);
    }
    if (!l.is_zero()) {
        l.sign = static_cast<bool>(l.value.back() >> 31);   //the last digit is non-zero!!!
        if (l.sign) {
            l = l.invert().incAbs();                          //conversion from complementation2 to sign-bit
        }
        l.normalise();
    }
    return *this = l;
}

big_integer &big_integer::operator-=(const big_integer &rhs) {
    return operator+=(-rhs);
}

big_integer &big_integer::operator*=(const big_integer &rhs) {
    big_integer res;
    if (is_zero() || rhs.is_zero()) {
        return *this = res;
    }
    size_t len = std::max(length(), rhs.length());
    res.value.resize(2*len);
    uint64_t carry, mul, buf;
    for (size_t i = 0; i < length(); ++i) {
        carry = 0;
        for (size_t j = 0; j < rhs.length(); ++j) {
            mul = uint64_t((*this)[i]) * rhs[j];
            buf = (mul & UINT32_MAX) + carry + res[i+j];
            res[i + j] = buf;
            carry = (mul >> 31 >> 1) + (buf >> 31 >> 1);
        }
        if (carry)
            res[i + rhs.length()] += carry;
    }
    res.sign = sign ^ rhs.sign;
    res.normalise();
    return *this = res;
}

big_integer &big_integer::operator/=(const big_integer &rhs) {
    if (rhs.is_zero())
        throw std::runtime_error("Division by zero");
    big_integer res;
    if (length() < rhs.length())
        return *this = res;

    big_integer remainder(*this);
    big_integer divisor(rhs);

    //normalisation
    uint32_t const k = static_cast<uint32_t>((UINT32_MAX + 1ll) / (divisor.value.back() + 1ll));
    mul_long_short(remainder, remainder, k);
    mul_long_short(divisor, divisor, k);
    //~~~

    remainder.value.push_back(0);
    size_t pref_len = divisor.length() + 1;
    size_t dividend_len = remainder.length();
    res.value.resize(dividend_len - pref_len + 1);
    big_integer qd;
    uit quotient_digit;
    for (size_t i = pref_len, j = res.value.size() - 1; i <= dividend_len; ++i, --j) {
        quotient_digit = trial(remainder, divisor);
        mul_long_short(qd, divisor, quotient_digit);
        while (!prefix_compare(remainder, qd, pref_len)) {
            quotient_digit--;
            mul_long_short(qd, divisor, quotient_digit);
        }
        res.value[j] = quotient_digit;
        prefix_sub(remainder, qd, pref_len);
        if (remainder.value.back() == 0)
            remainder.value.pop_back();
    }

    res.sign = sign ^ rhs.sign;
    res.normalise();
    return *this = res;
}

///for long division:
void big_integer::mul_long_short(big_integer & res, big_integer const & a, uit const q_k) {
    size_t n = a.length();
    res.value.resize(n + 1);
    uint64_t mul, buf, carry = 0;
    for (size_t i = 0; i < n; ++i) {
        mul = uint64_t(a[i]) * q_k;
        buf = (mul & UINT32_MAX) + carry;
        res[i] = buf;
        carry = (mul >> 31 >> 1) + (buf >> 31 >> 1);
    }
    res[n] = carry;
    res.normalise();
}

bool big_integer::prefix_compare(big_integer const & r, big_integer const & qd, size_t const pref_len) {
    int start = static_cast<int>(r.length() - pref_len);
    uit qd_digit;
    for (int i = static_cast<int>(r.length() - 1), j = static_cast<int>(pref_len - 1); i >= start; --i, --j) {
        qd_digit = (j < int(qd.length()) ? qd[j] : 0);
        if (r[i] != qd_digit)
            return r[i] > qd_digit;
    }
    return true;
}

void big_integer::prefix_sub(big_integer & r, big_integer const &qd, size_t const pref_len) { //pref_len = m + 1, m - size of divisor
    size_t start = r.length() - pref_len;
    bool borrow = false;
    uit qd_digit, sub;
    for (size_t i = 0; i < pref_len; ++i) {
        qd_digit = (i < qd.length() ? qd[i] : 0);
        sub = r[start + i] - qd_digit - borrow;
        borrow = r[start + i] < qd_digit + borrow;
        r[start + i] = sub;
    }
}

uit big_integer::trial(big_integer const & remainder, big_integer const & divisor) {
    uint64_t a = (static_cast<uint64_t>(remainder.value.back()) << 31 << 1) | remainder[remainder.length() - 2];
    return std::min(static_cast<uit>(a / divisor.value.back()), UINT32_MAX);
}

big_integer &big_integer::operator%=(const big_integer &rhs) {
    return *this = big_integer(*this - *this / rhs * rhs);
}

big_integer& big_integer::operator&=(big_integer const& rhs)
{
    big_integer l = convert_c2(),
                r = rhs.convert_c2();
    size_t len = std::max(l.length(), r.length());
    l.resize(len);
    r.resize(len);
    for (size_t i = 0; i < l.length(); ++i) {
        l[i] &= r[i];
    }
    l.sign &= r.sign;
    if (l.sign)
        l = l.invert().incAbs();
    l.normalise();
    return *this = l;
}

big_integer& big_integer::operator|=(big_integer const& rhs)
{
    big_integer l = convert_c2(),
                r = rhs.convert_c2();
    size_t len = std::max(l.length(), r.length());
    l.resize(len);
    r.resize(len);
    for (size_t i = 0; i < l.length(); ++i) {
        l[i] |= r[i];
    }
    l.sign |= r.sign;
    if (l.sign)
        l = l.invert().incAbs();
    l.normalise();
    return *this = l;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    big_integer l = convert_c2(),
            r = rhs.convert_c2();
    size_t len = std::max(l.length(), r.length());
    l.resize(len);
    r.resize(len);
    for (size_t i = 0; i < l.length(); ++i) {
        l[i] ^= r[i];
    }
    l.sign ^= r.sign;
    if (l.sign)
        l = l.invert().incAbs();
    l.normalise();
    return *this = l;
}

big_integer& big_integer::operator<<=(int rhs)
{
    int shift = rhs & ((1 << 5) - 1);
    if (shift) {
        value.push_back(0);
        uit q[2] = {0};
        for (size_t i = 0; i < length(); ++i) {
            q[1] = value[i] >> (32 - shift);
            value[i] <<= shift;
            value[i] |= q[0];
            std::swap(q[0], q[1]);
        }
    }
    shift = rhs >> 5;
    value.insert(value.begin(), shift, 0);
    normalise();
    return *this;
}

big_integer& big_integer::operator>>=(int rhs)
{
    bool add = false;
    if (sign) {
        add = false;
        for (int i = 0; i < (rhs >> 5); ++i) {
            if (value[i] != 0) {
                add = true;
                break;
            }
        }
        int shift = rhs & ((1 << 5) - 1);
        add |= value[rhs >> 31 >> 1] & ((1 << shift) - 1);
    }
    int shift = rhs >> 5;
    if (shift >= int(length())) {
        value.resize(0);
        sign = false;
        return *this;
    }
    big_integer temp(*this);
    temp.value.erase(temp.value.begin(), temp.value.begin() + shift);
    shift = rhs & ((1 << 5) - 1);
    if (shift) {
        uit q[2] = {0};
        for (size_t i = length(); i--; ) {
            q[1] = temp.value[i] << (32 - shift);
            temp.value[i] >>= shift;
            temp.value[i] |= q[0];
            std::swap(q[0], q[1]);
        }
    }
    if (add)
        temp = temp.incAbs();
    temp.normalise();
    return *this = temp;
}

big_integer big_integer::operator+() const
{
    return *this;
}

big_integer big_integer::operator-() const
{
    big_integer res(*this);
    if (!res.is_zero())
        res.sign ^= 1;
    return res;
}

big_integer big_integer::operator~() const
{
    big_integer res(*this);
    res++;
    res.sign ^= 1;
    return res;
}

big_integer& big_integer::operator++()
{
    return *this += 1;
}

big_integer big_integer::operator++(int)
{
    big_integer res = *this;
    ++*this;
    return res;
}

big_integer& big_integer::operator--()
{
    return *this -= 1;
}

big_integer big_integer::operator--(int)
{
    big_integer res = *this;
    --*this;
    return res;
}
/// private methods:
uit& big_integer::operator[](size_t pos) {
    return value[pos];
}

uit const & big_integer::operator[](size_t pos) const {
    return value[pos];
}

size_t big_integer::length() const {
    return value.size();
}

void big_integer::normalise() {
    while (value.size() && value.back() == 0) {
        value.pop_back();
    }
    if (value.size() == 0)
        sign = false;
}

bool big_integer::is_zero() const {
    return value.size() == 0;
}

big_integer big_integer::incAbs() const {
    big_integer res(*this);
    bool carry = 1;
    for (size_t i = 0; i < res.length(); ++i) {
        uint64_t buf = uint64_t(res[i]) + carry;
        res[i] = buf;
        carry = static_cast<bool>(buf >> 31 >> 1);
    }
    return res;
}

big_integer big_integer::invert() const {
    big_integer res(*this);
    for (size_t i = 0; i < res.length(); ++i) {
        res[i] = ~res[i];
    }
    return res;
}

big_integer big_integer::convert_c2() const {
    big_integer temp(*this);
    temp.value.push_back(0);
    if (!sign)
        return temp;
    temp = temp.invert().incAbs();
    return temp;
}

void big_integer::resize(size_t len) {
    while (length() < len)
        value.push_back(sign ? UINT32_MAX : 0);
}

///for comparison operations:
int8_t big_integer::cmp(big_integer const& a, big_integer const& b) {
    if (a.sign != b.sign) {
        return (int8_t) (a.sign ? -1 : 1);
    }
    big_integer x(a);
    big_integer y(b);
    size_t l = std::max(a.length(), b.length());
    x.value.resize(l);
    y.value.resize(l);
    int8_t absCmp = 0;
    for (size_t i = l; i--; ) {
        if (x[i] > y[i]) {
            absCmp = 1;
            break;
        } else if (x[i] < y[i]) {
            absCmp = -1;
            break;
        }
    }
    if (x.sign)
        return -absCmp;
    else
        return absCmp;
}

uit big_integer::to_uint32(big_integer const & a) {
    if (a.is_zero())
        return 0;
    return a.value[0];
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

big_integer operator+(big_integer a, big_integer const& b)
{
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b)
{
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b)
{
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b)
{
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b)
{
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b)
{
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b)
{
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b)
{
    return a ^= b;
}

big_integer operator<<(big_integer a, int b)
{
    return a <<= b;
}

big_integer operator>>(big_integer a, int b)
{
    return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b)
{
    return big_integer::cmp(a, b) == 0;
}

bool operator!=(big_integer const& a, big_integer const& b)
{
    return big_integer::cmp(a, b) != 0;
}

bool operator<(big_integer const& a, big_integer const& b)
{
    return big_integer::cmp(a, b) == -1;
}

bool operator>(big_integer const& a, big_integer const& b)
{
    return big_integer::cmp(a, b) == 1;
}

bool operator<=(big_integer const& a, big_integer const& b)
{
    return big_integer::cmp(a, b) <= 0;
}

bool operator>=(big_integer const& a, big_integer const& b)
{
    return big_integer::cmp(a, b) >= 0;
}

std::string to_string(big_integer const& a)
{
    std::string res;
    if (a.is_zero())
        return res += '0';
    big_integer temp(a);
    while (!temp.is_zero()) {
        res.push_back(static_cast<char>(big_integer::to_uint32(temp % 10) + '0'));
        temp /= 10;
    }
    if (a.sign)
        res.push_back('-');
    std::reverse(res.begin(), res.end());
    return res;
}