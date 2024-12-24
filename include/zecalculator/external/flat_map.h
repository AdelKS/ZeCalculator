// taken from https://github.com/tzlaine/flat_map

// -*- C++ -*-

// Copyright (C) 2019-2022 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef REFERENCE_IMPLEMENTATION_FLAT_MAP_
#define REFERENCE_IMPLEMENTATION_FLAT_MAP_

// NOTE: This implementation has only been tested against libstdc++ and libc++.

#include <algorithm>
#include <iterator>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__)
#include <bits/uses_allocator.h>
#endif

#define CPP20_CONCEPTS 201703L < __cplusplus && defined(__cpp_lib_ranges)
#define CMCSTL2_CONCEPTS                                                       \
    201703L <= __cplusplus && __has_include(<stl2/ranges.hpp>)
#define USE_CONCEPTS CPP20_CONCEPTS || CMCSTL2_CONCEPTS

#if defined(CPP20_CONCEPTS)
#include <ranges>
#elif CMCSTL2_CONCEPTS
#include <stl2/ranges.hpp>
#include <stl2/algorithm.hpp>
#endif


namespace std {

#if defined(CMCSTL2_CONCEPTS) && !defined(CPP20_CONCEPTS)
    using namespace experimental;
#endif

    template<typename _T1, typename _T2>
    struct __ref_pair
    {
        static_assert(is_reference<_T1>{} && is_reference<_T2>{});

        using __pair_type = pair<std::remove_cvref_t<_T1>, std::remove_cvref_t<_T2>>;
        using __pair_of_references_type =
            pair<std::remove_cvref_t<_T1> &, std::remove_cvref_t<_T2> &>;
        using __const_pair_of_references_type =
            pair<std::remove_cvref_t<_T1> const &, std::remove_cvref_t<_T2> const &>;

        __ref_pair(_T1 __t1, _T2 __t2) : first(__t1), second(__t2) {}
        __ref_pair(__ref_pair const & __other) :
            first(__other.first), second(__other.second)
        {}
        __ref_pair(__ref_pair && __other) :
            first(__other.first), second(__other.second)
        {}
        __ref_pair const & operator=(__ref_pair const & __other) const
        {
            first = __other.first;
            second = __other.second;
            return *this;
        }
        __ref_pair const & operator=(__ref_pair && __other) const
        {
            first = __other.first;
            second = __other.second;
            return *this;
        }

        __ref_pair const & operator=(__pair_type const & __other) const
        {
            first = __other.first;
            second = __other.second;
            return *this;
        }
        __ref_pair const & operator=(__pair_type && __other) const
        {
            first = std::move(__other.first);
            second = std::move(__other.second);
            return *this;
        }

        operator __pair_type() const { return __pair_type(first, second); }
        operator __pair_of_references_type() const
        {
            return __pair_of_references_type(first, second);
        }
        operator __const_pair_of_references_type() const
        {
            return __const_pair_of_references_type(first, second);
        }
        bool operator==(__ref_pair __rhs) const
        {
            return first == __rhs.first && second == __rhs.second;
        }
        bool operator!=(__ref_pair __rhs) const { return !(*this == __rhs); }
        bool operator<(__ref_pair __rhs) const
        {
            if (first < __rhs.first)
                return true;
            if (__rhs.first < first)
                return false;
            return second < __rhs.second;
        }

        _T1 first;
        _T2 second;
    };

    template<typename _T1, typename _T2>
    void
    swap(__ref_pair<_T1, _T2> const & __lhs, __ref_pair<_T1, _T2> const & __rhs)
    {
        using std::swap;
        swap(__lhs.first, __rhs.first);
        swap(__lhs.second, __rhs.second);
    }

    template<class _KeyRef, class _TRef, class _KeyIter, class _MappedIter>
    struct __flat_map_iterator
    {
        static_assert(is_reference<_KeyRef>{} && is_reference<_TRef>{});

        using iterator_category = random_access_iterator_tag;
        using value_type =
            pair<std::remove_cvref_t<_KeyRef>, std::remove_cvref_t<_TRef>>;
        using difference_type =
            typename iterator_traits<_KeyIter>::difference_type;
        using reference = __ref_pair<_KeyRef, _TRef>;

        struct __arrow_proxy
        {
            reference * operator->() noexcept { return &__value_; }
            reference const * operator->() const noexcept { return &__value_; }
            explicit __arrow_proxy(reference __value) noexcept :
                __value_(std::move(__value))
            {}

        private:
            reference __value_;
        };
        using pointer = __arrow_proxy;

        __flat_map_iterator() {}
        __flat_map_iterator(_KeyIter __key_it, _MappedIter __mapped_it) :
            __key_it_(__key_it), __mapped_it_(__mapped_it)
        {}
        template<class _TRef2, class _MappedIter2>
        __flat_map_iterator(
            __flat_map_iterator<_KeyRef, _TRef2, _KeyIter, _MappedIter2>
                __other,
            enable_if_t<
                is_convertible<_TRef2, _TRef>::value &&
                    is_convertible<_MappedIter2, _MappedIter>::value,
                int *> = nullptr) :
            __key_it_(__other.__key_it_), __mapped_it_(__other.__mapped_it_)
        {}

        reference operator*() const noexcept { return __ref(); }
        pointer operator->() const noexcept { return __arrow_proxy(__ref()); }

        reference operator[](difference_type __n) const noexcept
        {
            return reference(*(__key_it_ + __n), *(__mapped_it_ + __n));
        }

        __flat_map_iterator operator+(difference_type __n) const noexcept
        {
            return __flat_map_iterator(__key_it_ + __n, __mapped_it_ + __n);
        }
        __flat_map_iterator operator-(difference_type __n) const noexcept
        {
            return __flat_map_iterator(__key_it_ - __n, __mapped_it_ - __n);
        }

        __flat_map_iterator & operator++() noexcept
        {
            ++__key_it_;
            ++__mapped_it_;
            return *this;
        }
        __flat_map_iterator operator++(int) noexcept
        {
            __flat_map_iterator tmp(*this);
            ++__key_it_;
            ++__mapped_it_;
            return tmp;
        }

        __flat_map_iterator & operator--() noexcept
        {
            --__key_it_;
            --__mapped_it_;
            return *this;
        }
        __flat_map_iterator operator--(int) noexcept
        {
            __flat_map_iterator tmp(*this);
            --__key_it_;
            --__mapped_it_;
            return tmp;
        }

        __flat_map_iterator & operator+=(difference_type __n) noexcept
        {
            __key_it_ += __n;
            __mapped_it_ += __n;
            return *this;
        }
        __flat_map_iterator & operator-=(difference_type __n) noexcept
        {
            __key_it_ -= __n;
            __mapped_it_ -= __n;
            return *this;
        }

        _KeyIter __key_iter() const { return __key_it_; }
        _MappedIter __mapped_iter() const { return __mapped_it_; }

        friend bool
        operator==(__flat_map_iterator __lhs, __flat_map_iterator __rhs)
        {
            return __lhs.__key_it_ == __rhs.__key_it_;
        }
        friend bool
        operator!=(__flat_map_iterator __lhs, __flat_map_iterator __rhs)
        {
            return !(__lhs == __rhs);
        }

        friend bool
        operator<(__flat_map_iterator __lhs, __flat_map_iterator __rhs)
        {
            return __lhs.__key_it_ < __rhs.__key_it_;
        }
        friend bool
        operator<=(__flat_map_iterator __lhs, __flat_map_iterator __rhs)
        {
            return __lhs == __rhs || __lhs < __rhs;
        }
        friend bool
        operator>(__flat_map_iterator __lhs, __flat_map_iterator __rhs)
        {
            return __rhs < __lhs;
        }
        friend bool
        operator>=(__flat_map_iterator __lhs, __flat_map_iterator __rhs)
        {
            return __lhs == __rhs || __rhs < __lhs;
        }

        friend typename __flat_map_iterator::difference_type
        operator-(__flat_map_iterator __lhs, __flat_map_iterator __rhs)
        {
            return __lhs.__key_it_ - __rhs.__key_it_;
        }

    private:
        template<
            class _KeyRef2,
            class _TRef2,
            class _KeyIter2,
            class _MappedIter2>
        friend struct __flat_map_iterator;

        reference __ref() const { return reference(*__key_it_, *__mapped_it_); }

        _KeyIter __key_it_;
        _MappedIter __mapped_it_;
    };


    // NOTE: This overload was necessary, since iter_swap(it1, it2) calls
    // swap(*it1, *it2).  All std::swap() overloads expect lvalues, and
    // flat_map's iterators produce proxy rvalues when dereferenced.
    template<class _KeyRef, class _TRef>
    inline void
    swap(pair<_KeyRef, _TRef> && __lhs, pair<_KeyRef, _TRef> && __rhs)
    {
        using std::swap;
        swap(__lhs.first, __rhs.first);
        swap(__lhs.second, __rhs.second);
    }


    struct sorted_unique_t
    {
        explicit sorted_unique_t() = default;
    };
    inline constexpr sorted_unique_t sorted_unique{};

    template<
        class _Key,
        class _T,
        class _Compare = less<_Key>,
        class _KeyContainer = vector<_Key>,
        class _MappedContainer = vector<_T>>
    class flat_map
    {
        template<typename _Alloc>
        using __uses = enable_if_t<
            uses_allocator<_KeyContainer, _Alloc>::value &&
            uses_allocator<_MappedContainer, _Alloc>::value>;

        template<typename _Container, typename = void>
        struct __has_begin_end : false_type
        {};
        template<typename _Container>
        struct __has_begin_end<
            _Container,
            void_t<
                decltype(std::begin(declval<_Container>())),
                decltype(std::end(declval<_Container>()))>> : true_type
        {};
        template<typename _Container>
        using __container = enable_if_t<__has_begin_end<_Container>::value>;

    public:
        // types:
        using key_type = _Key;
        using mapped_type = _T;
        using value_type = pair<const key_type, mapped_type>;
        using key_compare = _Compare;
        using reference = pair<const key_type &, mapped_type &>;
        using const_reference = pair<const key_type &, const mapped_type &>;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using iterator = __flat_map_iterator<
            const key_type &,
            mapped_type &,
            typename _KeyContainer::const_iterator,
            typename _MappedContainer::iterator>; // see 21.2
        using const_iterator = __flat_map_iterator<
            const key_type &,
            const mapped_type &,
            typename _KeyContainer::const_iterator,
            typename _MappedContainer::const_iterator>; // see 21.2
        using reverse_iterator = __flat_map_iterator<
            const key_type &,
            mapped_type &,
            typename _KeyContainer::const_reverse_iterator,
            typename _MappedContainer::reverse_iterator>; // see 21.2
        using const_reverse_iterator = __flat_map_iterator<
            const key_type &,
            const mapped_type &,
            typename _KeyContainer::const_reverse_iterator,
            typename _MappedContainer::const_reverse_iterator>; // see 21.2
        using key_container_type = _KeyContainer;
        using mapped_container_type = _MappedContainer;

        class value_compare
        {
            friend flat_map;

        private:
            key_compare __comp;
            value_compare(key_compare __c) : __comp(__c) {}

        public:
            bool operator()(const_reference __x, const_reference __y) const
            {
                return __comp(__x.first, __y.first);
            }
        };

        struct containers
        {
            key_container_type keys;
            mapped_container_type values;
        };

        // ??, construct/copy/destroy
        flat_map() : flat_map(key_compare()) {}
        flat_map(
            key_container_type __key_cont,
            mapped_container_type __mapped_cont) :
            __c{std::move(__key_cont), std::move(__mapped_cont)},
            __compare(key_compare())
        {
            __mutable_iterator __first(__c.keys.begin(), __c.values.begin());
            __mutable_iterator __last(__c.keys.end(), __c.values.end());
#if defined(USE_CONCEPTS)
            ranges::sort(__first, __last, value_comp());
#else
            sort(__first, __last, value_comp());
#endif
        }
        template<class _Alloc, class _Enable = __uses<_Alloc>>
        flat_map(
            const key_container_type & __key_cont,
            const mapped_container_type & __mapped_cont,
            const _Alloc & __a) :
            __c{key_container_type(__key_cont, __a),
                mapped_container_type(__mapped_cont, __a)},
            __compare()
        {
            __mutable_iterator __first(__c.keys.begin(), __c.values.begin());
            __mutable_iterator __last(__c.keys.end(), __c.values.end());
#if defined(USE_CONCEPTS)
            ranges::sort(__first, __last, value_comp());
#else
            sort(__first, __last, value_comp());
#endif
        }
        template<class _Container, class _Enable = __container<_Container>>
        explicit flat_map(
            const _Container & __cont,
            const key_compare & __comp = key_compare()) :
            flat_map(std::begin(__cont), std::end(__cont), __comp)
        {}
        template<
            class _Container,
            class _Alloc,
            class _Enable1 = __container<_Container>,
            class _Enable2 = __uses<_Alloc>>
        flat_map(const _Container & __cont, const _Alloc & __a) :
            flat_map(std::begin(__cont), std::end(__cont), __a)
        {}
        flat_map(
            sorted_unique_t,
            key_container_type __key_cont,
            mapped_container_type __mapped_cont) :
            __c{std::move(__key_cont), std::move(__mapped_cont)},
            __compare(key_compare())
        {}
        template<class _Alloc, class _Enable = __uses<_Alloc>>
        flat_map(
            sorted_unique_t,
            const key_container_type & __key_cont,
            const mapped_container_type & __mapped_cont,
            const _Alloc & __a) :
            __c{key_container_type(__key_cont, __a),
                mapped_container_type(__mapped_cont, __a)},
            __compare()
        {}
        template<class _Container, class _Enable = __container<_Container>>
        flat_map(
            sorted_unique_t __s,
            const _Container & __cont,
            const key_compare & __comp = key_compare()) :
            flat_map(__s, std::begin(__cont), std::end(__cont), __comp)
        {}
        template<
            class _Container,
            class _Alloc,
            class _Enable1 = __container<_Container>,
            class _Enable2 = __uses<_Alloc>>
        flat_map(
            sorted_unique_t __s,
            const _Container & __cont,
            const _Alloc & __a) :
            __c{key_container_type(__a), mapped_container_type(__a)},
            __compare()
        {
            __c.keys.reserve(__cont.size());
            __c.values.reserve(__cont.size());
            insert(__s, std::begin(__cont), std::end(__cont));
        }
        explicit flat_map(const key_compare & __comp) : __c(), __compare(__comp)
        {}
        template<class _Alloc, class _Enable = __uses<_Alloc>>
        flat_map(const key_compare & __comp, const _Alloc & __a) :
            __c{key_container_type(__a), mapped_container_type(__a)},
            __compare(__comp)
        {}
        template<class _Alloc, class _Enable = __uses<_Alloc>>
        explicit flat_map(const _Alloc & __a) :
            __c{key_container_type(__a), mapped_container_type(__a)},
            __compare()
        {}
        template<class _InputIterator>
        flat_map(
            _InputIterator __first,
            _InputIterator __last,
            const key_compare & __comp = key_compare()) :
            __c(), __compare(__comp)
        {
            insert(__first, __last);
        }
        template<
            class _InputIterator,
            class _Alloc,
            class _Enable = __uses<_Alloc>>
        flat_map(
            _InputIterator __first,
            _InputIterator __last,
            const key_compare & __comp,
            const _Alloc & __a) :
            __c{key_container_type(__a), mapped_container_type(__a)},
            __compare(__comp)
        {
            insert(__first, __last);
        }
        template<
            class _InputIterator,
            class _Alloc,
            class _Enable = __uses<_Alloc>>
        flat_map(
            _InputIterator __first, _InputIterator __last, const _Alloc & __a) :
            flat_map(__first, __last, key_compare(), __a)
        {}
        template<class _InputIterator>
        flat_map(
            sorted_unique_t __s,
            _InputIterator __first,
            _InputIterator __last,
            const key_compare & __comp = key_compare()) :
            __c(), __compare(__comp)
        {
            insert(__s, __first, __last);
        }
        template<
            class _InputIterator,
            class _Alloc,
            class _Enable = __uses<_Alloc>>
        flat_map(
            sorted_unique_t __s,
            _InputIterator __first,
            _InputIterator __last,
            const key_compare & __comp,
            const _Alloc & __a) :
            __c{key_container_type(__a), mapped_container_type(__a)},
            __compare(__comp)
        {
            insert(__s, __first, __last);
        }
        template<
            class _InputIterator,
            class _Alloc,
            class _Enable = __uses<_Alloc>>
        flat_map(
            sorted_unique_t __s,
            _InputIterator __first,
            _InputIterator __last,
            const _Alloc & __a) :
            flat_map(__s, __first, __last, key_compare(), __a)
        {}
        flat_map(
            initializer_list<value_type> && __il,
            const key_compare & __comp = key_compare()) :
            flat_map(__il, __comp)
        {}
        template<class _Alloc, class _Enable = __uses<_Alloc>>
        flat_map(
            initializer_list<value_type> && __il,
            const key_compare & __comp,
            const _Alloc & __a) :
            flat_map(std::begin(__il), std::end(__il), __comp, __a)
        {}
        template<class _Alloc, class _Enable = __uses<_Alloc>>
        flat_map(initializer_list<value_type> && __il, const _Alloc & __a) :
            flat_map(std::begin(__il), std::end(__il), key_compare(), __a)
        {}
        flat_map(
            sorted_unique_t __s,
            initializer_list<value_type> && __il,
            const key_compare & __comp = key_compare()) :
            flat_map(__s, __il, __comp)
        {}
        template<class _Alloc, class _Enable = __uses<_Alloc>>
        flat_map(
            sorted_unique_t __s,
            initializer_list<value_type> && __il,
            const key_compare & __comp,
            const _Alloc & __a) :
            flat_map(__s, std::begin(__il), std::end(__il), __comp, __a)
        {}
        template<class _Alloc, class _Enable = __uses<_Alloc>>
        flat_map(
            sorted_unique_t __s,
            initializer_list<value_type> && __il,
            const _Alloc & __a) :
            flat_map(__s, std::begin(__il), std::end(__il), key_compare(), __a)
        {}
        flat_map & operator=(initializer_list<value_type> __il)
        {
            flat_map __tmp(__il, __compare);
            swap(__tmp);
            return *this;
        }

        // iterators
        iterator begin() noexcept
        {
            return iterator(__c.keys.begin(), __c.values.begin());
        }
        const_iterator begin() const noexcept
        {
            return const_iterator(__c.keys.begin(), __c.values.begin());
        }
        iterator end() noexcept
        {
            return iterator(__c.keys.end(), __c.values.end());
        }
        const_iterator end() const noexcept
        {
            return const_iterator(__c.keys.end(), __c.values.end());
        }
        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator(__c.keys.rbegin(), __c.values.rbegin());
        }
        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(
                __c.keys.rbegin(), __c.values.rbegin());
        }
        reverse_iterator rend() noexcept
        {
            return reverse_iterator(__c.keys.rend(), __c.values.rend());
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(__c.keys.rend(), __c.values.rend());
        }

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }
        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        // ??, capacity
        [[nodiscard]] bool empty() const noexcept { return __c.keys.empty(); }
        size_type size() const noexcept { return __c.keys.size(); }
        size_type max_size() const noexcept
        {
            return std::min<size_type>(
                __c.keys.max_size(), __c.values.max_size());
        }

        // ??, element access
        mapped_type & operator[](const key_type & __x)
        {
            return try_emplace(__x).first->second;
        }
        mapped_type & operator[](key_type && __x)
        {
            return try_emplace(std::move(__x)).first->second;
        }
        mapped_type & at(const key_type & __x)
        {
            auto __it = __key_find(__x);
            if (__it == __c.keys.end())
                throw out_of_range("Value not found by flat_map.at()");
            return *__project(__it);
        }
        const mapped_type & at(const key_type & __x) const
        {
            auto __it = __key_find(__x);
            if (__it == __c.keys.end())
                throw out_of_range("Value not found by flat_map.at()");
            return *__project(__it);
        }

        // ??, modifiers
        template<
            class... _Args,
            class _Enable = enable_if_t<is_constructible<
                pair<key_type, mapped_type>,
                _Args &&...>::value>>
        pair<iterator, bool> emplace(_Args &&... __args)
        {
            pair<key_type, mapped_type> __p(std::forward<_Args>(__args)...);
            return try_emplace(std::move(__p.first), std::move(__p.second));
        }
        template<
            class... _Args,
            class _Enable = enable_if_t<is_constructible<
                pair<key_type, mapped_type>,
                _Args &&...>::value>>
        iterator emplace_hint([[maybe_unused]] const_iterator __position, _Args &&... __args)
        {
            return emplace(std::forward<_Args>(__args)...).first;
        }
        pair<iterator, bool> insert(const value_type & __x)
        {
            return emplace(__x);
        }
        pair<iterator, bool> insert(value_type && __x)
        {
            return emplace(std::move(__x));
        }
        iterator insert(const_iterator __position, const value_type & __x)
        {
            return emplace_hint(__position, __x);
        }
        iterator insert(const_iterator __position, value_type && __x)
        {
            return emplace_hint(__position, std::move(__x));
        }
        template<
            class _P,
            class _Enable = enable_if_t<
                is_constructible<pair<key_type, mapped_type>, _P &&>::value>>
        pair<iterator, bool> insert(_P && __x)
        {
            return emplace(std::forward<_P>(__x));
        }
        template<
            class _P,
            class _Enable = enable_if_t<
                is_constructible<pair<key_type, mapped_type>, _P &&>::value>>
        iterator insert(const_iterator __position, _P && __x)
        {
            return emplace_hint(__position, std::forward<_P>(__x));
        }
        template<class _InputIterator>
        void insert(_InputIterator __first, _InputIterator __last)
        {
            auto const __prev_size = size();
            for (auto __it = __first; __it != __last; ++__it) {
                __c.keys.push_back(__it->first);
                __c.values.push_back(__it->second);
            }

            __mutable_iterator __inserted_first(
                __c.keys.begin() + __prev_size,
                __c.values.begin() + __prev_size);
            __mutable_iterator __inserted_last(
                __c.keys.end(), __c.values.end());
#if defined(USE_CONCEPTS)
            ranges::sort(__inserted_first, __inserted_last, value_comp());
#else
            sort(__inserted_first, __inserted_last, value_comp());
#endif

            if (!__prev_size)
                return;

            __mutable_iterator __mutable_first(
                __c.keys.begin(), __c.values.begin());
#if defined(USE_CONCEPTS)
            ranges::inplace_merge(
                __mutable_first,
                __inserted_first,
                __inserted_last,
                value_comp());
#else
            inplace_merge(
                __mutable_first,
                __inserted_first,
                __inserted_last,
                value_comp());
#endif
        }
        template<class _InputIterator>
        void
        insert(sorted_unique_t, _InputIterator __first, _InputIterator __last)
        {
            auto const __prev_size = size();
            for (auto __it = __first; __it != __last; ++__it) {
                __c.keys.push_back(__it->first);
                __c.values.push_back(__it->second);
            }

            __mutable_iterator __inserted_first(
                __c.keys.begin() + __prev_size,
                __c.values.begin() + __prev_size);
            __mutable_iterator __inserted_last(
                __c.keys.end(), __c.values.end());

            if (!__prev_size)
                return;

            __mutable_iterator __mutable_first(
                __c.keys.begin(), __c.values.begin());
#if defined(USE_CONCEPTS)
            ranges::inplace_merge(
                __mutable_first,
                __inserted_first,
                __inserted_last,
                value_comp());
#else
            inplace_merge(
                __mutable_first,
                __inserted_first,
                __inserted_last,
                value_comp());
#endif
        }
        void insert(initializer_list<value_type> __il)
        {
            insert(__il.begin(), __il.end());
        }
        void insert(sorted_unique_t __s, initializer_list<value_type> __il)
        {
            insert(__s, __il.begin(), __il.end());
        }

        containers extract() &&
        {
            __scoped_clear _(this);
            return std::move(__c);
        }
        void replace(
            key_container_type && __key_cont,
            mapped_container_type && __mapped_cont)
        {
            __scoped_clear _(this);
            __c.keys = std::move(__key_cont);
            __c.values = std::move(__mapped_cont);
            _.__release();
        }

        template<
            class... _Args,
            class _Enable =
                enable_if_t<is_constructible<mapped_type, _Args &&...>::value>>
        pair<iterator, bool>
        try_emplace(const key_type & __k, _Args &&... __args)
        {
            auto __it = __key_lower_bound(__k);
            if (__it == __c.keys.end() || __compare(*__it, __k) ||
                __compare(__k, *__it)) {
                auto __values_it = __c.values.emplace(
                    __project(__it), std::forward<_Args>(__args)...);
                __it = __c.keys.insert(__it, __k);
                return pair<iterator, bool>(iterator(__it, __values_it), true);
            }
            return pair<iterator, bool>(iterator(__it, __project(__it)), false);
        }
        template<
            class... _Args,
            class _Enable =
                enable_if_t<is_constructible<mapped_type, _Args &&...>::value>>
        pair<iterator, bool> try_emplace(key_type && __k, _Args &&... __args)
        {
            auto __it = __key_lower_bound(__k);
            if (__it == __c.keys.end() || __compare(*__it, __k) ||
                __compare(__k, *__it)) {
                auto __values_it = __c.values.emplace(
                    __project(__it), std::forward<_Args>(__args)...);
                __it = __c.keys.insert(__it, std::forward<key_type>(__k));
                return pair<iterator, bool>(iterator(__it, __values_it), true);
            }
            return pair<iterator, bool>(iterator(__it, __project(__it)), false);
        }
        template<
            class... _Args,
            class _Enable =
                enable_if_t<is_constructible<mapped_type, _Args &&...>::value>>
        iterator try_emplace(
            [[maybe_unused]] const_iterator __hint, const key_type & __k, _Args &&... __args)
        {
            return try_emplace(__k, std::forward<_Args>(__args)...).first;
        }
        template<
            class... _Args,
            class _Enable =
                enable_if_t<is_constructible<mapped_type, _Args &&...>::value>>
        iterator
        try_emplace([[maybe_unused]] const_iterator __hint, key_type && __k, _Args &&... __args)
        {
            return try_emplace(
                       std::forward<key_type>(__k),
                       std::forward<_Args>(__args)...)
                .first;
        }

        template<
            class _M,
            class _Enable = enable_if_t<
                is_assignable<mapped_type &, _M>::value &&
                is_constructible<mapped_type, _M &&>::value>>
        pair<iterator, bool> insert_or_assign(const key_type & __k, _M && __obj)
        {
            auto __it = __key_lower_bound(__k);
            if (__it == __c.keys.end() || __compare(*__it, __k) ||
                __compare(__k, *__it)) {
                auto __values_it =
                    __c.values.insert(__project(__it), std::forward<_M>(__obj));
                __it = __c.keys.insert(__it, __k);
                return pair<iterator, bool>(iterator(__it, __values_it), true);
            }
            auto __values_it = __project(__it);
            *__values_it = std::forward<_M>(__obj);
            return pair<iterator, bool>(iterator(__it, __values_it), false);
        }
        template<
            class _M,
            class _Enable = enable_if_t<
                is_assignable<mapped_type &, _M>::value &&
                is_constructible<mapped_type, _M &&>::value>>
        pair<iterator, bool> insert_or_assign(key_type && __k, _M && __obj)
        {
            auto __it = __key_lower_bound(__k);
            if (__it == __c.keys.end() || __compare(*__it, __k) ||
                __compare(__k, *__it)) {
                auto __values_it =
                    __c.values.insert(__project(__it), std::forward<_M>(__obj));
                __it = __c.keys.insert(__it, std::forward<key_type>(__k));
                return pair<iterator, bool>(iterator(__it, __values_it), true);
            }
            auto __values_it = __project(__it);
            *__values_it = std::forward<_M>(__obj);
            return pair<iterator, bool>(iterator(__it, __values_it), false);
        }
        template<
            class _M,
            class _Enable = enable_if_t<
                is_assignable<mapped_type &, _M>::value &&
                is_constructible<mapped_type, _M &&>::value>>
        iterator
        insert_or_assign([[maybe_unused]] const_iterator hint, const key_type & __k, _M && __obj)
        {
            return insert_or_assign(__k, std::forward<_M>(__obj)).first;
        }
        template<
            class _M,
            class _Enable = enable_if_t<
                is_assignable<mapped_type &, _M>::value &&
                is_constructible<mapped_type, _M &&>::value>>
        iterator
        insert_or_assign([[maybe_unused]] const_iterator hint, key_type && __k, _M && __obj)
        {
            return insert_or_assign(
                       std::forward<key_type>(__k), std::forward<_M>(__obj))
                .first;
        }

        iterator erase(iterator __position)
        {
            return iterator(
                __c.keys.erase(__position.__key_iter()),
                __c.values.erase(__position.__mapped_iter()));
        }
        iterator erase(const_iterator __position)
        {
            return iterator(
                __c.keys.erase(__position.__key_iter()),
                __c.values.erase(__position.__mapped_iter()));
        }
        size_type erase(const key_type & __x)
        {
            auto __it = __key_find(__x);
            if (__it == __c.keys.end())
                return size_type(0);
            __c.values.erase(__project(__it));
            __c.keys.erase(__it);
            return size_type(1);
        }
        iterator erase(const_iterator __first, const_iterator __last)
        {
            return iterator(
                __c.keys.erase(__first.__key_iter(), __last.__key_iter()),
                __c.values.erase(
                    __first.__mapped_iter(), __last.__mapped_iter()));
        }

        void swap(flat_map & __fm) noexcept(
            is_nothrow_swappable<key_compare>::value
        )
        {
            using std::swap;
            swap(__compare, __fm.__compare);
            swap(__c.keys, __fm.__c.keys);
            swap(__c.values, __fm.__c.values);
        }
        void clear() noexcept
        {
            __c.keys.clear();
            __c.values.clear();
        }

        // observers
        key_compare key_comp() const { return __compare; }
        value_compare value_comp() const { return value_compare(__compare); }
        const key_container_type & keys() const noexcept { return __c.keys; }
        const mapped_container_type & values() const noexcept
        {
            return __c.values;
        }

        // map operations
        iterator find(const key_type & __x)
        {
            auto __it = __key_find(__x);
            return iterator(__it, __project(__it));
        }
        const_iterator find(const key_type & __x) const
        {
            auto __it = __key_find(__x);
            return const_iterator(__it, __project(__it));
        }
        template<class _K>
        iterator find(const _K & __x)
        {
            auto __it = __key_find(__x);
            return iterator(__it, __project(__it));
        }
        template<class _K>
        const_iterator find(const _K & __x) const
        {
            auto __it = __key_find(__x);
            return iterator(__it, __project(__it));
        }
        size_type count(const key_type & __x) const
        {
            auto __it = __key_find(__x);
            return size_type(__it == __c.keys.end() ? 0 : 1);
        }
        template<class _K>
        size_type count(const _K & __x) const
        {
            auto __it = __key_find(__x);
            return size_type(__it == __c.keys.end() ? 0 : 1);
        }
        bool contains(const key_type & __x) const
        {
            return count(__x) == size_type(1);
        }
        template<class _K>
        bool contains(const _K & __x) const
        {
            return count(__x) == size_type(1);
        }
        iterator lower_bound(const key_type & __x)
        {
            auto __it = __key_lower_bound(__x);
            return iterator(__it, __project(__it));
        }
        const_iterator lower_bound(const key_type & __x) const
        {
            auto __it = __key_lower_bound(__x);
            return const_iterator(__it, __project(__it));
        }
        template<class _K>
        iterator lower_bound(const _K & __x)
        {
            auto __it = __key_lower_bound(__x);
            return iterator(__it, __project(__it));
        }
        template<class _K>
        const_iterator lower_bound(const _K & __x) const
        {
            auto __it = __key_lower_bound(__x);
            return const_iterator(__it, __project(__it));
        }
        iterator upper_bound(const key_type & __x)
        {
            auto __it = __key_upper_bound(__x);
            return iterator(__it, __project(__it));
        }
        const_iterator upper_bound(const key_type & __x) const
        {
            auto __it = __key_upper_bound(__x);
            return const_iterator(__it, __project(__it));
        }
        template<class _K>
        iterator upper_bound(const _K & __x)
        {
            auto __it = __key_upper_bound(__x);
            return iterator(__it, __project(__it));
        }
        template<class _K>
        const_iterator upper_bound(const _K & __x) const
        {
            auto __it = __key_upper_bound(__x);
            return const_iterator(__it, __project(__it));
        }
        pair<iterator, iterator> equal_range(const key_type & __k)
        {
            iterator const __first = lower_bound(__k);
            iterator const __last =
                find_if(__first, end(), [&](auto const & __x) {
                    return __compare(__k, __x.first);
                });
            return pair<iterator, iterator>(__first, __last);
        }
        pair<const_iterator, const_iterator>
        equal_range(const key_type & __k) const
        {
            const_iterator const __first = lower_bound(__k);
            const_iterator const __last =
                find_if(__first, end(), [&](auto const & __x) {
                    return __compare(__k, __x.first);
                });
            return pair<const_iterator, const_iterator>(__first, __last);
        }
        template<class _K>
        pair<iterator, iterator> equal_range(const _K & __k)
        {
            iterator const __first = lower_bound(__k);
            iterator const __last =
                find_if(__first, end(), [&](auto const & __x) {
                    return __compare(__k, __x.first);
                });
            return pair<iterator, iterator>(__first, __last);
        }
        template<class _K>
        pair<const_iterator, const_iterator> equal_range(const _K & __k) const
        {
            const_iterator const __first = lower_bound(__k);
            const_iterator const __last =
                find_if(__first, end(), [&](auto const & __x) {
                    return __compare(__k, __x.first);
                });
            return pair<const_iterator, const_iterator>(__first, __last);
        }

        friend bool operator==(const flat_map & __x, const flat_map & __y)
        {
#if defined(USE_CONCEPTS)
            return ranges::equal(__x, __y);
#else
            return equal(__x.begin(), __x.end(), __y.begin(), __y.end());
#endif
        }
        friend bool operator!=(const flat_map & __x, const flat_map & __y)
        {
            return !(__x == __y);
        }
        friend bool operator<(const flat_map & __x, const flat_map & __y)
        {
#if defined(USE_CONCEPTS)
            return ranges::lexicographical_compare(
                __x, __y, [](auto __lhs, auto __rhs) { return __lhs < __rhs; });
#else
            return lexicographical_compare(
                __x.begin(), __x.end(), __y.begin(), __y.end());
#endif
        }
        friend bool operator>(const flat_map & __x, const flat_map & __y)
        {
            return __y < __x;
        }
        friend bool operator<=(const flat_map & __x, const flat_map & __y)
        {
            return !(__y < __x);
        }
        friend bool operator>=(const flat_map & __x, const flat_map & __y)
        {
            return !(__x < __y);
        }

        friend void
        swap(flat_map & __x, flat_map & __y) noexcept(noexcept(__x.swap(__y)))
        {
            return __x.swap(__y);
        }

    private:
        containers __c;        // exposition only
        key_compare __compare; // exposition only
        // exposition only
        struct __scoped_clear
        {
            explicit __scoped_clear(flat_map * __fm) : __fm_(__fm) {}
            ~__scoped_clear()
            {
                if (__fm_)
                    __fm_->clear();
            }
            void __release() { __fm_ = nullptr; }

        private:
            flat_map * __fm_;
        };

        using __key_iter_t = typename _KeyContainer::iterator;
        using __key_const_iter_t = typename _KeyContainer::const_iterator;
        using __mapped_iter_t = typename _MappedContainer::iterator;
        using __mapped_const_iter_t = typename _MappedContainer::const_iterator;

        using __mutable_iterator = __flat_map_iterator<
            key_type &,
            mapped_type &,
            __key_iter_t,
            __mapped_iter_t>;

        __mapped_iter_t __project(__key_iter_t __key_it)
        {
            return __c.values.begin() + (__key_it - __c.keys.begin());
        }
        __mapped_const_iter_t __project(__key_const_iter_t __key_it) const
        {
            return __c.values.begin() + (__key_it - __c.keys.begin());
        }

        template<typename _K>
        __key_iter_t __key_lower_bound(const _K & __k)
        {
#if defined(USE_CONCEPTS)
            return ranges::lower_bound(__c.keys, __k, __compare);
#else
            return std::lower_bound(
                __c.keys.begin(), __c.keys.end(), __k, __compare);
#endif
        }
        template<typename _K>
        __key_const_iter_t __key_lower_bound(const _K & __k) const
        {
#if defined(USE_CONCEPTS)
            return ranges::lower_bound(__c.keys, __k, __compare);
#else
            return std::lower_bound(
                __c.keys.begin(), __c.keys.end(), __k, __compare);
#endif
        }
        template<typename _K>
        __key_iter_t __key_upper_bound(const _K & __k)
        {
#if defined(USE_CONCEPTS)
            return ranges::upper_bound(__c.keys, __k, __compare);
#else
            return std::upper_bound(
                __c.keys.begin(), __c.keys.end(), __k, __compare);
#endif
        }
        template<typename _K>
        __key_const_iter_t __key_upper_bound(const _K & __k) const
        {
#if defined(USE_CONCEPTS)
            return ranges::upper_bound(__c.keys, __k, __compare);
#else
            return std::upper_bound(
                __c.keys.begin(), __c.keys.end(), __k, __compare);
#endif
        }
        template<typename _K>
        __key_iter_t __key_find(const _K & __k)
        {
            auto __it = __key_lower_bound(__k);
            if (__it != __c.keys.end() &&
                (__compare(*__it, __k) || __compare(__k, *__it)))
                __it = __c.keys.end();
            return __it;
        }
        template<typename _K>
        __key_const_iter_t __key_find(const _K & __k) const
        {
            auto __it = __key_lower_bound(__k);
            if (__it != __c.keys.end() &&
                (__compare(*__it, __k) || __compare(__k, *__it)))
                __it = __c.keys.end();
            return __it;
        }
    };
}

#endif
