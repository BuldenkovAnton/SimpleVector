#pragma once
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <utility>


class ReserveProxyObj {
public:
	ReserveProxyObj(size_t new_capacity)
		: reserve_(new_capacity) {}

		size_t GetReserve() const {
		return reserve_;
	}
private:
	size_t reserve_ = 0;
};


template <typename Type>
class SimpleVector {
public:
	using Iterator = Type*;
	using ConstIterator = const Type*;

	SimpleVector() noexcept = default;

	explicit SimpleVector(ReserveProxyObj obj)
		: items_(new Type[obj.GetReserve()])
		, size_(0)
		, capacity_(obj.GetReserve()) {}


	SimpleVector(size_t size, const Type& value = Type{})
	: size_(size)
	, capacity_(size) {
		if (size == 0) return;
		items_ = new Type[size];
		std::fill(items_, items_ + size_, value);
	}

	SimpleVector(std::initializer_list<Type> init)
	: size_(init.size())
	, capacity_(init.size()) {
		if (init.size() == 0) return;
		items_ = new Type[init.size()];
		std::move(init.begin(), init.end(), begin());
	}

	~SimpleVector() {
		delete[] items_;
		size_ = 0;
		capacity_ = 0;
	}

	size_t GetSize() const noexcept {
		return size_;
	}

	size_t GetCapacity() const noexcept {
		return capacity_;
	}

	bool IsEmpty() const noexcept {
		return size_ == 0;
	}

	Type& operator[](size_t index) noexcept {
		return items_[index];
	}

	const Type& operator[](size_t index) const noexcept {
		return items_[index];
	}

	Type& At(size_t index) {
		if (index >= size_) throw std::out_of_range("Operation At: Out of range");
		return items_[index];
	}

	const Type& At(size_t index) const {
		if (index >= size_) throw std::out_of_range("Operation At: Out of range");
		return items_[index];
	}

	void Clear() noexcept {
		size_ = 0;
	}


	void Resize(size_t new_size) {
		if (new_size > capacity_) {
			capacity_ = std::max(new_size, size_ * 2);
			Type* tmp = new Type[capacity_];
			std::move(begin(), end(), tmp);
			std::swap(tmp, items_);
			delete[] tmp;
			std::fill(items_ + size_, items_ + new_size, Type{});
		}
		size_ = new_size;
	}

	Iterator begin() noexcept {
		return Iterator(items_);
	}

	Iterator end() noexcept {
		return Iterator(items_ + size_);
	}

	ConstIterator begin() const noexcept {
		return ConstIterator(items_);
	}

	ConstIterator end() const noexcept {
		return ConstIterator(items_ + size_);
	}

	ConstIterator cbegin() const noexcept {
		return ConstIterator(items_);
	}

	ConstIterator cend() const noexcept {
		return ConstIterator(items_ + size_);
	}

	SimpleVector(const SimpleVector& other)
		: items_(new Type[other.capacity_])
		, size_(other.size_)
		, capacity_(other.capacity_) {
			std::copy(other.begin(), other.end(), begin());
	}

	SimpleVector(SimpleVector&& other)
		: items_(other.items_)
		, size_(other.size_)
		, capacity_(other.capacity_) {
		other.items_ = nullptr;
		other.size_ = 0;
		other.capacity_ = 0;
	}

	SimpleVector& operator=(const SimpleVector& rhs) {
		if (this == &rhs) return *this;

		if (rhs.size_ <= capacity_) {
			std::copy(rhs.begin(), rhs.end(), begin());
			size_ = rhs.size_;
		}
		else {
			SimpleVector tmp(rhs);
			std::swap(tmp.items_, items_);
			std::swap(tmp.size_, size_);
			std::swap(tmp.capacity_, capacity_);
		}
		return *this;
	}

	SimpleVector& operator=(SimpleVector&& rhs) {
		if (this == &rhs) return *this;

		delete[] items_;
		items_ = rhs.items_;
		size_ = rhs.size_;
		capacity_ = rhs.capacity_;

		rhs.items_ = nullptr;
		rhs.size_ = 0;
		rhs.capacity_ = 0;
		return *this;
	}

	void PushBack(const Type& item) {
		InsertInVector(end(), item);
	}

	void PushBack(Type&& item) {
		InsertInVector(end(), std::move(item));
	}

	void PopBack() noexcept {
		assert(size_ != 0);
		size_--;
	}

	Iterator Insert(ConstIterator pos, const Type& value) {
		return InsertInVector(pos, value);
	}


	Iterator Insert(ConstIterator pos, Type&& value) {
		return InsertInVector(pos, std::move(value));
	}

	Iterator Erase(ConstIterator pos) {
		Iterator pos_return = const_cast<Iterator>(pos);
		std::move(pos_return + 1, end(), pos_return);
		size_--;
		return pos_return;
	}


	void swap(SimpleVector& other) noexcept {
		std::swap(items_, other.items_);
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
	}

	void Reserve(size_t new_capacity) {
		if (new_capacity > capacity_) {
			Type* tmp = new Type[new_capacity];
			std::move(begin(), begin() + size_, tmp);
			std::swap(tmp, items_);
			capacity_ = new_capacity;
			delete[] tmp;
		}
	}


private:
	Type* items_ = nullptr;
	size_t size_ = 0;
	size_t capacity_ = 0;


	Iterator  InsertInVector(ConstIterator pos, Type&& value){
		size_t position = pos - cbegin();
		if (size_ < capacity_) {
			std::move_backward(begin() + position, end() , end());
		}
		else {
			capacity_ == 0 ? capacity_ = 1 : capacity_ += capacity_;
			Type* tmp = new Type[capacity_];
			std::move(begin(), begin() + position, tmp);
			std::move(begin() + position, end(), tmp + position + 1);
			std::swap(tmp, items_);
			delete[] tmp;
		}
		*(begin() + position) = std::move(value);
		size_++;
		return  begin() + position;
	}
};



template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::equal(lhs.begin(), lhs.end(), rhs.begin()) && lhs.GetSize() == rhs.GetSize();
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
	return ReserveProxyObj(capacity_to_reserve);
}
