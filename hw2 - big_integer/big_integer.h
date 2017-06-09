#ifndef HW2_BIG_INTEGER_BIG_INTEGER_H
#define HW2_BIG_INTEGER_BIG_INTEGER_H
#include <string>
#include <vector>
typedef uint32_t uit;

using std::vector;

class big_integer {
private:
    vector<uit> value;
    bool sign;

    uit& operator[](size_t pos);
    uit const & operator[](size_t pos) const;
    void normalise();
    void resize(size_t len);
    size_t length() const;
    bool is_zero() const;
    big_integer incAbs() const;
    big_integer invert() const;
    big_integer convert_c2() const;

    ///for comparsion operations
    static int8_t cmp(big_integer const& a, big_integer const& b);
    ///~~~~~~~~~~~~~~~~~~~~~~~~~

    ///for long division:
    static big_integer mul_long_short(big_integer const & a, uit const q_k);
    static bool prefix_compare(big_integer const & r, big_integer const & qd, size_t const pref_len);
    static void prefix_sub(big_integer & r, big_integer const & qd, size_t const pref_len);
    static uit trial(big_integer const & remainder, big_integer const & divisor);
    ///~~~~~~~~~~~~~~~~~~~~~~~~~

    static uit to_uint32(big_integer const & a);

public:
    big_integer();
    big_integer(big_integer const& other);
    big_integer(int a);
    explicit big_integer(std::string const& str);
    //~big_integer(); default

    big_integer& operator=(big_integer const& other);

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(int rhs);
    big_integer& operator>>=(int rhs);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend std::string to_string(big_integer const& a);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
//void swap(big_integer& a, big_integer& b);
#endif //HW2_BIG_INTEGER_BIG_INTEGER_H
