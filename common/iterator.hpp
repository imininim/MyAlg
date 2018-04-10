#include <stddef.h>

#include <iostream>

using namespace std;

namespace yangshu_common {
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag:public input_iterator_tag {};
struct bidirectional_iterator_tag:public forward_iterator_tag {};
struct random_access_iterator_tag:public bidirectional_iterator_tag {};

template<class Category, class T, class Distance = ptrdiff_t, class Pointer = T*, class Reference = T&>
struct iterator {
    typedef Category  iterator_category;
    typedef T         value_type;
    typedef Distance  difference_type;
    typedef Pointer   pointer;
    typedef Reference reference;
};

template<typename Iterator>
struct iterator_traits {
    typedef typename Iterator::iterator_category  iterator_category;
    typedef typename Iterator::value_type         value_type;
    typedef typename Iterator::difference_type    difference_type;
    typedef typename Iterator::pointer            pointer;
    typedef typename Iterator::reference          reference;
};

//原生指针
template<typename T>
struct iterator_traits<T*> {
    typedef random_access_iterator_tag  iterator_category;
    typedef T                           value_type;
    typedef ptrdiff_t                   difference_type;
    typedef T*                          pointer;
    typedef T&                          reference;
};

//原生指针const版本
template<typename T>
struct iterator_traits<const T*> {
    typedef random_access_iterator_tag  iterator_category;
    typedef T                           value_type;
    typedef ptrdiff_t                   difference_type;
    typedef const T*                    pointer;
    typedef const T&                    reference;
};

//查看迭代器类型
template<class Iterator>
inline typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator &) {
    typedef typename iterator_traits<Iterator>::iterator_category category;
    return category();
}

//查看迭代器 difference_type
template<typename Iterator>
inline typename iterator_traits<Iterator>::difference_type*
distance_type(const Iterator &) {
    return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
}

//查看迭代器 value_type
template<typename Iterator>
inline typename iterator_traits<Iterator>::value_type*
value_type(const Iterator &) {
    return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

template<typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator first, InputIterator last, input_iterator_tag) {
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last) {
        ++first; ++n;
    }
    return n;
}

template<typename RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
__distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag) {
    return last - first;
}

template<typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last) {
    return __distance(first, last, iterator_category(first));
}

template<class InputIterator, class Distance>
inline void __advance(InputIterator &i, Distance n, input_iterator_tag) {
    while (n--) ++i;
}

template<class BidirectionalIterator, class Distance>
inline void __advance(BidirectionalIterator &i, Distance n, bidirectional_iterator_tag) {
    if (n >= 0) {
        while (n--) ++i;
    } else {
        while (n++) --i;
    }
}

template<class RandomAccessIterator, class Distance>
inline void __advance(RandomAccessIterator &i, Distance n, random_access_iterator_tag) {
    i += n;
}

template<class InputIterator, class Distance>
inline void advance(InputIterator &i, Distance n) {
    __advance(i, n, iterator_category(i));
}
};
int main() {
    int *p;
    yangshu_common::distance_type<int*>(p);

    int a[10] = {0, 1, 2, 3, 4, 5 ,6 ,7, 8, 9};
    p = a;
    int *q = &(a[9]);
    cerr << yangshu_common::distance<int*>(p, q) << endl;
    cerr << *p << endl;
    yangshu_common::advance<int*, int>(p, 3);
    cerr << *p << endl;
    cerr << yangshu_common::distance<int*>(p, q) << endl;
    return 0;
}
