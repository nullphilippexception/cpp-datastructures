#ifndef VECTOR_H
#define VECTOR_H

#include<iostream>
#include<stdexcept>
#include<initializer_list>

template<typename T>
class Vector {
	
	public: // new classes  -----------------------------------------------------------------------------------------
	class ConstIterator;
	class Iterator;
	
	public: // using declarations -----------------------------------------------------------------------------------
	using value_type = T;
	using size_type = std::size_t;
	using pointer = value_type*;
	using const_reference = const value_type&;
	using reference = value_type&;
	using const_pointer = const value_type*;
	using difference_type = std::ptrdiff_t;
	using const_iterator = Vector::ConstIterator;
	using iterator = Vector::Iterator;

	private: // Instanzvariablen -----------------------------------------------------------------------------------
	size_type sz;
	size_type max_sz;
	pointer values;
	
	public: // Konstruktoren & Destruktor -----------------------------------------------------------------------------------
	Vector() : sz {0}, max_sz {0}, values {nullptr} {} 
	
	Vector(size_type n) : sz {0}, max_sz {0}, values {nullptr} {
		reserve(n);
	}
	
	Vector(std::initializer_list <value_type> l) : sz {0}, max_sz {0}, values {nullptr} { 
		reserve(l.size()); 
		for(const auto& v : l)
		{
			values[sz++] = v;
		}
	}
	
	Vector(const Vector& v) : sz{0}, max_sz{0}, values {nullptr} {
		reserve(v.capacity()); 
		for(int i {0}; i < v.sz; i++) 
		{
			values[i] = v.values[i];
		}
		sz = v.sz;
	}
	
	~Vector() {delete[] values;}
	
	public: // Methoden -----------------------------------------------------------------------------------
	std::size_t size() const { 
		return sz;
	}
	
	bool empty() const { 
		if(sz == 0) return true;
		return false; 
	}
	
	void clear() {
		sz = 0; // Elemente noch da, aber ned mehr verwendet
	}
	
	void reserve(size_type n) {
		if(max_sz >= n) return;
		pointer next {new value_type[n]};
		for(size_type i {0}; i < sz; i++)
		{
			next[i] = values[i];
		}
		delete[] values;
		values = next;
		max_sz = n;
	}
	
	void shrink_to_fit() { 
		max_sz = sz;
	}
	
	void push_back(const_reference val) {
		if(sz == max_sz)
		{
			reserve(max_sz*1.61+5); // die +5 nur da keine Mindestgr��e
		}
		values[sz++] = val;
	}
	
	void pop_back() { 
		if(sz == 0) throw std::runtime_error("Vector is empty, can't pop_back()");
		sz--; 
	}
	
	std::size_t capacity() const { 
		return max_sz;
	}
	
	public: // Operatoren -----------------------------------------------------------------------------------
	Vector& operator=(const Vector& v) { 
		if(max_sz < v.max_sz) reserve(v.max_sz);
		sz = v.sz;
		max_sz = v.max_sz;
		
		for(int i {0}; i < v.sz; i++)
		{
			values[i] = v.values[i];
		}
		return *this; // alter Zustand von this wird tats�chlich vernichtet
	}
	
	value_type& operator[](size_t index) { 
		if(index < 0 || index >= sz) throw std::runtime_error("Asked for non-existing element");
		return values[index]; 
	}
	
	const value_type& operator[](size_t index) const { 
		// check for error
		if(index < 0 || index >= sz) throw std::runtime_error("Asked for non-existing element");
		return values[index]; 
	}
	
	public: // iterator methoden f�r vektor ---------------------------------------------------------------------------------------------
	iterator begin() {
		if(empty()) return end();
		//else
		Iterator it {values};
		return it;
	}
	
	iterator end() {
		Iterator it {values+sz};
		return it;
	}
	
	const_iterator begin() const {
		if(empty()) return end();
		// else
		ConstIterator it {values};
		return it;
	}
	
	const_iterator end() const {
		ConstIterator it {values+sz};
		return it;
	}
    
	iterator insert(const_iterator pos, const_reference val) {
		auto diff = pos-begin();
		if(diff < 0 || static_cast<size_type>(diff) > sz) throw std::runtime_error("Iterator out of bounds");
		size_type current{static_cast<size_type>(diff)};
		if(sz >= max_sz) reserve((max_sz+5) * 2); // ACHTUNG SONDERFALL BEHANDELN
		for(size_type i {sz}; (i--) > current;) 
		{
			values[i+1] = values[i];
		}
		values[current] = val;
		++sz;
		return iterator {values+current};
	}
	
	iterator erase(const_iterator pos) {
		auto diff = pos - begin();
		if(diff < 0 || static_cast<size_type>(diff) >= sz) throw std::runtime_error("Iterator out of bounds");
		size_type current {static_cast<size_type>(diff)};
		for(size_type i{current}; i < sz - 1; ++i)
		{
			values[i] = values[i+1];
		}
		--sz;
		return iterator{values+current};
	}
	
	public: // Iteratoren ----------------------------------------------------------------------------------------
	class ConstIterator {
		public: // using declarations
		using value_type = Vector::value_type;
		using reference = Vector::const_reference;
		using pointer = Vector::const_pointer;
		using difference_type = Vector::difference_type;
		using iterator_category = std::forward_iterator_tag;
		
		private: // instanzvariablen
		pointer ptr;
		
		public: // konstruktoren
		ConstIterator() : ptr {nullptr} {}
		ConstIterator(pointer p) : ptr{p} {}
		
		public: // friend methoden
		friend Vector::difference_type operator- (const Vector::ConstIterator& lop, const Vector::ConstIterator& rop) {
			return lop.ptr - rop.ptr;
		}
		
		public: // methoden
		reference operator*() const {
			return *ptr;
		}
		
		pointer operator->() const {
			return ptr;
		}
		
		bool operator==(const const_iterator& ci) const {
			if(ptr == ci.ptr) return true; 
			return false; // else
		}
		
		bool operator!=(const const_iterator& ci) const {
			if(ptr != ci.ptr) return true; 
			return false; // else
		}
		
		const_iterator& operator++() { 
			ptr++;
			return *this;
		}
		
		const_iterator operator++(int) { 
			ConstIterator old_it {ptr};
			ptr++;
			return old_it;
		}
		
	};
	
	class Iterator {
		public: // using declarations
		using value_type = Vector::value_type;
		using reference = Vector::reference;
		using pointer = Vector::pointer;
		using difference_type = Vector::difference_type;
		using iterator_category = std::forward_iterator_tag;
		
		private: // instanzvariablen
		pointer ptr;
		
		public: // konstruktoren
		Iterator() : ptr{nullptr} {}
		Iterator(pointer p) : ptr{p} {}
		
		public: // methoden
		reference operator*() const {
			return *ptr;
		}
		
		pointer operator->() const {
			return ptr;
		}
		
		bool operator==(const const_iterator& ci) const { 
			if(ci == (static_cast<const_iterator>(*this))) return true; 
			return false; // false
		}
		
		bool operator!=(const const_iterator& ci) const { 
			if(ci != (static_cast<const_iterator>(*this))) return true; 
			return false; // else
		}
		
		iterator& operator++() { 
			ptr++;
			return *this;
		}
		
		iterator operator++(int) { 
			Iterator old_it {ptr};
			ptr++;
			return old_it;
		}
		
		operator const_iterator() const { 
			ConstIterator new_it{ptr};
			return new_it;
		}

	} ;
  
};

template<typename T>
std::ostream& operator<<(std::ostream& o, const Vector<T>& v) {
	o << "[";
	for(int i{0}; i < v.size(); i++)
	{
		o << v[i];
		if(i < v.size() - 1) o << ", ";
	}
	o << "]";
	return o;
}


#endif

