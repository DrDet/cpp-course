//
// Created by drden on 15.06.2017.
//

#ifndef HW3_SMALL_OBJECT_COPY_ON_WRITE_OPTS_FOR_BIGINT_VECTOR_H
#define HW3_SMALL_OBJECT_COPY_ON_WRITE_OPTS_FOR_BIGINT_VECTOR_H
#include <vector>
#include <memory>

typedef unsigned uit;
using std::shared_ptr;
using std::vector;

class Vector {
private:
    static const size_t SMALL_SIZE = 4;
    struct dataStorage {
        shared_ptr< std::vector <uit> > big;
        uit small[SMALL_SIZE];
        dataStorage() : big(nullptr) { };
        ~dataStorage() {
            big = nullptr;
        };
    };
    dataStorage _data;
    size_t _size;
    bool isSmall;

    void make_big();
public:
    Vector();

    Vector(Vector const & other) noexcept;
    Vector& operator=(Vector const & other) noexcept;

    void make_editable();

    size_t size() const;
    uit back() const;
    void resize(size_t n);
    void push_back(uit const x);
    void pop_back();
    void push_front(uit const x, size_t n);

    void pop_front(size_t n);
    uit& operator[](size_t idx);
    uit const & operator[](size_t idx) const;
};


#endif //HW3_SMALL_OBJECT_COPY_ON_WRITE_OPTS_FOR_BIGINT_VECTOR_H
