/*
 * StackWrapper.hpp
 *
 *  Criado em: 30/07/2014
 *      Autor: Giovani Milanez Espindola
 *  Contact: giovani.milanez@gmail.com
 */

#ifndef STACKWRAPPER_HPP_
#define STACKWRAPPER_HPP_

#include <iterator>
#include <initializer_list>
#include <openssl/safestack.h>

template<typename TypeT>
class PointerIterator :
	public std::iterator<std::random_access_iterator_tag, TypeT>
{
protected:
	TypeT** m_pData;
	int m_Pos;

public:
	typedef std::random_access_iterator_tag iterator_category;
	typedef
		typename std::iterator<std::random_access_iterator_tag, TypeT>::value_type
		value_type;
	typedef
		typename std::iterator<std::random_access_iterator_tag, TypeT>::difference_type
		difference_type;
	typedef
		typename std::iterator<std::random_access_iterator_tag, TypeT>::reference
		reference;
	typedef
		typename std::iterator<std::random_access_iterator_tag, TypeT>::pointer
		pointer;


	PointerIterator() : m_pData(nullptr), m_Pos(-1) {}


	template<typename T2>
	PointerIterator(const PointerIterator<T2>& r) : m_pData(&(*r)), m_Pos(r.m_Pos) {}

	PointerIterator(pointer* pData, int pos) : m_pData(pData), m_Pos(pos) {}

	template<typename T2>
	PointerIterator& operator=(const PointerIterator<T2>& r)
		{ m_pData = &(*r); m_Pos = r.m_Pos; return *this; }

	PointerIterator& operator++()
		{ ++m_Pos; return *this; }

	PointerIterator& operator--()
		{ --m_Pos; return *this; }

	PointerIterator operator++(int)
		{ return PointerIterator(m_Pos++); }

	PointerIterator operator--(int)
		{ return PointerIterator(m_Pos--); }

	PointerIterator operator+(const difference_type& n) const
		{ return PointerIterator(m_pData, m_Pos + n); }

	PointerIterator& operator+=(const difference_type& n)
		{ m_Pos += n; return *this; }

	PointerIterator operator-(const difference_type& n) const
		{ return PointerIterator(m_pData, m_Pos - n); }

	PointerIterator& operator-=(const difference_type& n)
		{ m_Pos -= n; return *this; }

	reference operator*() const
		{ return *m_pData[m_Pos]; }

	pointer operator->() const
		{ return m_pData[m_Pos]; }

	reference operator[](const difference_type& n) const
		{ return *m_pData[n]; }


	template<typename T>
	friend bool operator==(
		const PointerIterator<T>& r1,
		const PointerIterator<T>& r2);

	template<typename T>
	friend bool operator!=(
		const PointerIterator<T>& r1,
		const PointerIterator<T>& r2);

	template<typename T>
	friend bool operator<(
		const PointerIterator<T>& r1,
		const PointerIterator<T>& r2);

	template<typename T>
	friend bool operator>(
		const PointerIterator<T>& r1,
		const PointerIterator<T>& r2);

	template<typename T>
	friend bool operator<=(
		const PointerIterator<T>& r1,
		const PointerIterator<T>& r2);

	template<typename T>
	friend bool operator>=(
		const PointerIterator<T>& r1,
		const PointerIterator<T>& r2);

	template<typename T>
	friend typename PointerIterator<T>::difference_type operator+(
		const PointerIterator<T>& r1,
		const PointerIterator<T>& r2);

	template<typename T>
	friend typename PointerIterator<T>::difference_type operator-(
		const PointerIterator<T>& r1,
		const PointerIterator<T>& r2);
};

template<typename T>
bool operator==(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
	{ return (r1.m_pData[r1.m_Pos] == r2.m_pData[r2.m_Pos]); }

template<typename T>
bool operator!=(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
	{ return (r1.m_pData[r1.m_Pos] != r2.m_pData[r2.m_Pos]); }

template<typename T>
bool operator<(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
	{ return (r1.m_pData[r1.m_Pos] < r2.m_pData[r2.m_Pos]); }

template<typename T>
bool operator>(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
	{ return (r1.m_pData[r1.m_Pos] > r2.m_pData[r2.m_Pos]); }

template<typename T>
bool operator<=(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
	{ return (r1.m_pData[r1.m_Pos] <= r2.m_pData[r2.m_Pos]); }

template<typename T>
bool operator>=(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
	{ return (r1.m_pData[r1.m_Pos] >= r2.m_pData[r2.m_Pos]); }

template<typename T>
typename PointerIterator<T>::difference_type operator+(
	const PointerIterator<T>& r1,
	const PointerIterator<T>& r2)
{ return PointerIterator<T>(r1.m_pData[r1.m_Pos] + r2.m_pData[r2.m_Pos]); }

template<typename T>
typename PointerIterator<T>::difference_type operator-(
	const PointerIterator<T>& r1, const PointerIterator<T>& r2)
{ return r1.m_pData[r1.m_Pos] - r2.m_pData[r2.m_Pos]; }


template
	<
		typename T,
		void (*FreeFunc)(T*),  // free function, e.g. X509_free
		int (*CMP)(const T *, const T *) = nullptr
	>
class Asn1Stack
{
public:
	typedef PointerIterator<const T> const_iterator;
	typedef PointerIterator<T> iterator;

	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;

	Asn1Stack() :
		internal_(CMP != nullptr ? sk_new((int (*)(const void*, const void*))CMP) : sk_new_null())
	{
	}

	Asn1Stack(std::initializer_list<T*> init) :
		Asn1Stack()
	{
		for(auto e : init)
			push(e);
	}

	virtual ~Asn1Stack()
	{
		sk_pop_free(internal_, (void (*)(void *)) FreeFunc);
	}

	Asn1Stack(const Asn1Stack& src) :
		Asn1Stack(sk_dup(src.internal_))
	{
	}
	Asn1Stack& operator=(const Asn1Stack& rhs)
	{
		_STACK *tmp = sk_dup(rhs.internal_);
		sk_pop_free(internal_, (void (*)(void *)) FreeFunc);
		internal_ = tmp;
		return *this;
	}
	Asn1Stack(Asn1Stack&& src) :
		internal_(src.internal_)
	{
		src.internal_ = nullptr;
	}
	Asn1Stack& operator=(Asn1Stack&& rhs)
	{
		if (this == &rhs)
			return *this;
		sk_pop_free(internal_, (void (*)(void *)) FreeFunc);
		internal_ = rhs.internal_;
		rhs.internal_ = nullptr;
		return *this;
	}

//	operator _STACK *() const
//	{
//		return internal_;
//	}
//
//	operator T **() const
//	{
//		return internal_->data;
//	}

	int num() const
	{
		return sk_num(internal_);
	}

	void push(T* value)
	{
		sk_push(internal_, value);
	}

	void insert(T* value, int pos)
	{
		sk_insert(internal_, value, pos);
	}

	T* head() const
	{
		return value(num() - 1);
	}

	T* bottom() const
	{
		return value(0);
	}

	T* value(int pos) const
	{
		return (T*)sk_value(internal_, pos);
	}

	bool isSorted() const
	{
		return sk_is_sorted(internal_) == 1;
	}

	void sort()
	{
		if(CMP == nullptr)
			throw cryptowrapper::NullPointerException("No compare function provided for StackWrapper");

		sk_sort(internal_);
	}

	int find(T* value)
	{
		return sk_find(internal_, value);
	}

//	const T& operator[](size_t pos) const
//		{ return *value(pos); }

	T& operator[](size_t pos)
		{ return *value(pos); }

	const_iterator begin() const
		{ return const_iterator((T**)internal_->data, 0); }

	const_iterator end() const
		{ return const_iterator( (T**)internal_->data, num()); }

	const_reverse_iterator rbegin() const
		{ return const_reverse_iterator(end()); }

	const_reverse_iterator rend() const
		{ return const_reverse_iterator(begin()); }

	iterator begin()
		{ return iterator((T**)internal_->data, 0); }

	iterator end()
		{ return iterator((T**)internal_->data, num()); }

	reverse_iterator rbegin()
		{ return reverse_iterator(end()); }

	reverse_iterator rend()
		{ return reverse_iterator(begin()); }
private:
	_STACK* internal_;
};


#endif /* STACKWRAPPER_HPP_ */
