#pragma once

#include <cmath>
#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

namespace fefu {

	template <typename T>
	class allocator {
	public:
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = typename std::add_lvalue_reference<T>::type;
		using const_reference = typename std::add_lvalue_reference<const T>::type;
		using value_type = T;

		allocator() noexcept {
			this->unused_prop = 0;
		}

		allocator(const allocator& other) noexcept {
			this->unused_prop = other.unused_prop;
		}

		// for test
		uint32_t unused_prop;
		allocator(uint32_t val) : unused_prop(val) {}
		//~for test

		template <class U>
		allocator(const allocator<U>&) noexcept {}

		~allocator() = default;

		pointer allocate(size_type n) {
			return (pointer)::operator new(n * sizeof(value_type));
		}

		void deallocate(pointer p, size_type n) noexcept { ::operator delete(p, n * sizeof(value_type)); }
	};

	template <typename ValueType>
	class Node {
	public:
		using pointer = ValueType*;

		Node(pointer dptr, char* uptr, char* eptr) : dptr_(dptr), uptr_(uptr), eptr_(eptr) {}

		pointer dptr_;
		char* uptr_;
		char* eptr_;
	};

	template <typename ValueType>
	class hash_map_iterator {
		template <typename K, typename T, typename Hash, typename Pred,typename Alloc>
		friend class hash_map;

		template <typename>
		friend class hash_map_const_iterator;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = ValueType;
		using difference_type = std::ptrdiff_t;
		using reference = ValueType&;
		using pointer = ValueType*;

		hash_map_iterator() noexcept
			: node(nullptr, nullptr, nullptr) {
		}
		hash_map_iterator(const hash_map_iterator& other) noexcept
			: node(other.node.dptr_, other.node.uptr_, other.node.eptr_) {
		}

		reference operator*() const { 
			if (node.dptr_ == nullptr || node.eptr_ == nullptr || node.uptr_ == nullptr) {
				throw std::runtime_error("Uninit iterator");
			}
			return *node.dptr_;
		}
		pointer operator->() const {
			if (node.dptr_ == nullptr || node.eptr_ == nullptr || node.uptr_ == nullptr) {
				throw std::runtime_error("Uninit iterator");
			}
			return node.dptr_;
		}

		// prefix ++
		hash_map_iterator& operator++() {
			if (node.uptr_ != node.eptr_) {
				node.uptr_++;
				node.dptr_++;

				while (node.uptr_ != node.eptr_ && *node.uptr_ != 1) {
					node.dptr_++;
					node.uptr_++;
				}
			} else {
				throw std::runtime_error("Out of bounds");
			}

			return *this;
		}
		// postfix ++
		hash_map_iterator operator++(int) {
			hash_map_iterator result(*this);
			++(*this);
			return result;
		}

		friend bool operator==(const hash_map_iterator<ValueType>& lhs,
			const hash_map_iterator<ValueType>& rhs) {
			return &*lhs == &*rhs;
		}
		friend bool operator!=(const hash_map_iterator<ValueType>& lhs,
			const hash_map_iterator<ValueType>& rhs) {
			return &*lhs != &*rhs;
		}

	private:
		Node<ValueType> node;
	};

	template <typename ValueType>
	class hash_map_const_iterator {
		template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
		friend class hash_map;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = ValueType;
		using difference_type = std::ptrdiff_t;
		using reference = const ValueType&;
		using pointer = const ValueType*;

		hash_map_const_iterator() noexcept
			: node(nullptr, nullptr, nullptr) {
		}
		hash_map_const_iterator(const hash_map_const_iterator& other) noexcept
			: node(other.node.dptr_, other.node.uptr_, other.node.eptr_) {
		}
		hash_map_const_iterator(const hash_map_iterator<ValueType>& other) noexcept
			: node(other.node.dptr_, other.node.uptr_, other.node.eptr_) {
		}

		reference operator*() const {
			if (node.dptr_ == nullptr || node.eptr_ == nullptr || node.uptr_ == nullptr) {
				throw std::runtime_error("Uninit iterator");
			}
			return *node.dptr_;
		}
		pointer operator->() const {
			if (node.dptr_ == nullptr || node.eptr_ == nullptr || node.uptr_ == nullptr) {
				throw std::runtime_error("Uninit iterator");
			}
			return node.dptr_;
		}

		// prefix ++
		hash_map_const_iterator& operator++() {
			if (node.uptr_ != node.eptr_) {
				node.uptr_++;
				node.dptr_++;

				while (node.uptr_ != node.eptr_ && *node.uptr_ != 1) {
					node.dptr_++;
					node.uptr_++;
				}
			} else {
				throw std::runtime_error("Out of bounds");
			}

			return *this;
		}
		// postfix ++
		hash_map_const_iterator operator++(int) {
			hash_map_const_iterator result(*this);
			++(*this);
			return result;
		}

		friend bool operator==(const hash_map_const_iterator<ValueType>& lhs,
			const hash_map_const_iterator<ValueType>& rhs) {
			return &*lhs == &*rhs;
		}
		friend bool operator!=(const hash_map_const_iterator<ValueType>& lhs,
			const hash_map_const_iterator<ValueType>& rhs) {
			return &*lhs != &*rhs;
		}

	private:
		Node<ValueType> node;
	};

	template <typename K, typename T, typename Hash = std::hash<K>,
		typename Pred = std::equal_to<K>,
		typename Alloc = allocator<std::pair<const K, T>>>
		class hash_map {
		public:
			using key_type = K;
			using mapped_type = T;
			using hasher = Hash;
			using key_equal = Pred;
			using allocator_type = Alloc;
			using value_type = std::pair<const key_type, mapped_type>;
			using reference = value_type&;
			using const_reference = const value_type&;
			using iterator = hash_map_iterator<value_type>;
			using const_iterator = hash_map_const_iterator<value_type>;
			using size_type = std::size_t;

			hash_map() : hash_map(1) {}

			~hash_map() {
				if (data_ != nullptr) {
					for (size_type i = 0; i < capacity_; i++) {
						if (used_[i] == 1) {
							data_[i].second.~mapped_type();
						}
					}
					allocator_.deallocate(data_, capacity_);
					delete[] used_;
				}
			}

			explicit hash_map(size_type n)
				: hasher_(), allocator_(), pred_(), max_load_factor_(0.45f), length_(0) {
				
				capacity_ = std::max(static_cast<size_type>(1), n);
				used_ = new char[capacity_];
				data_ = allocator_.allocate(capacity_);
				std::fill_n(used_, capacity_, static_cast<char>(0));
			}

			template <typename InputIterator>
			hash_map(InputIterator first, InputIterator last, size_type n = 1)
				: hash_map(static_cast<size_type>(std::distance(first, last)) > n ? static_cast<size_type>(std::distance(first, last)) : n) {
				this->insert(first, last);
			}

			hash_map(const hash_map& other)
				: hasher_(), allocator_(), pred_(),
				max_load_factor_(0.45f),
				used_(new char[other.capacity_]),
				length_(other.length_),
				capacity_(other.capacity_) {
				data_ = allocator_.allocate(other.capacity_);

				for (size_type i = 0; i < other.capacity_; i++) {
					if (other.used_[i] == 1) {
						new(data_ + i) value_type(other.data_[i]);
					}
					used_[i] = other.used_[i];
				}
			}

			hash_map(hash_map&& other)
				: hash_map(1) {
				swap(other);
			}

			explicit hash_map(const allocator_type& a)
				: hasher_(), allocator_(a), pred_(), max_load_factor_(0.45f), length_(0) {

				capacity_ = 1;
				used_ = new char[capacity_];
				data_ = allocator_.allocate(capacity_);
				std::fill_n(used_, capacity_, static_cast<char>(0));
			}

			hash_map(const hash_map& other, const allocator_type& a)
				: hasher_(), allocator_(a), pred_(),
				max_load_factor_(0.45f),
				used_(new char[other.capacity_]),
				length_(other.length_),
				capacity_(other.capacity_) {
				data_ = allocator_.allocate(other.capacity_);

				for (size_type i = 0; i < other.capacity_; i++) {
					if (other.used_[i] == 1) {
						new(data_ + i) value_type(other.data_[i]);
					}
					used_[i] = other.used_[i];
				}
			}

			hash_map(hash_map&& other, const allocator_type& a)
				: hasher_(std::move(other.hasher_)), allocator_(a), pred_(std::move(other.pred_)),
				max_load_factor_(other.max_load_factor_), length_(other.length_) {

				capacity_ = other.capacity_;
				used_ = new char[capacity_];
				data_ = allocator_.allocate(capacity_);

				for (size_type i = 0; i < other.capacity_; i++) {
					if (other.used_[i] == 1) {
						new(data_ + i) value_type(std::move(other.data_[i]));
					}
					used_[i] = other.used_[i];
				}

				delete[] other.used_;
				other.allocator_.deallocate(other.data_, other.capacity_);
				other.allocator_ = allocator_type();
				other.max_load_factor_ = 0.45f;
				other.capacity_ = 1;
				other.used_ = new char[other.capacity_];
				other.data_ = allocator_.allocate(other.capacity_);
				std::fill_n(other.used_, other.capacity_, static_cast<char>(0));
				other.length_ = 0;
			}

			hash_map(std::initializer_list<value_type> l, size_type n = 1) 
				: hasher_(), allocator_(), pred_(), max_load_factor_(0.45f), length_(0) {

				capacity_ = std::max(l.size(), std::max(static_cast<size_type>(1), n));
				used_ = new char[capacity_];
				data_ = allocator_.allocate(capacity_);
				std::fill_n(used_, capacity_, static_cast<char>(0));

				this->insert(l.begin(), l.end());
			}


			/// Copy assignment operator.
			hash_map& operator=(const hash_map& other) {
				if (data_ != nullptr) {
					for (size_type i = 0; i < capacity_; i++) {
						if (used_[i] == 1) {
							data_[i].second.~mapped_type();
						}
					}
					allocator_.deallocate(data_, capacity_);
					delete[] used_;
				}

				max_load_factor_ = 0.45f;
				length_ = other.length_;
				capacity_ = other.capacity_;
				data_ = allocator_.allocate(other.capacity_);
				used_ = new char[other.capacity_];

				for (size_type i = 0; i < other.capacity_; i++) {
					if (other.used_[i] == 1) {
						new(data_ + i) value_type(other.data_[i]);
					}
					used_[i] = other.used_[i];
				}

				return *this;
			}

			/// Move assignment operator.
			hash_map& operator=(hash_map&& other) {
				if (data_ != nullptr) {
					for (size_type i = 0; i < capacity_; i++) {
						if (used_[i] == 1) {
							data_[i].second.~mapped_type();
						}
					}
					allocator_.deallocate(data_, capacity_);
					delete[] used_;
				}

				max_load_factor_ = 0.45f;
				capacity_ = 1;
				length_ = 0;
				data_ = allocator_.allocate(capacity_);
				used_ = new char[capacity_];
				std::fill_n(used_, capacity_, static_cast<char>(0));

				swap(other);

				return *this;
			}

			hash_map& operator=(std::initializer_list<value_type> l) {
				if (data_ != nullptr) {
					for (size_type i = 0; i < capacity_; i++) {
						if (used_[i] == 1) {
							data_[i].second.~mapped_type();
						}
					}
					allocator_.deallocate(data_, capacity_);
					delete[] used_;
				}

				max_load_factor_ = 0.45f;
				capacity_ = l.size();
				length_ = 0;
				data_ = allocator_.allocate(l.size());
				used_ = new char[l.size()]();
				std::fill_n(used_, capacity_, static_cast<char>(0));
				for (auto& vls : l) {
					this->operator[](vls.first) = vls.second;
				}

				return *this;
			}

			///  Returns the allocator object used by the %hash_map.
			allocator_type get_allocator() const noexcept { return allocator_; }

			// size and capacity:
			bool empty() const noexcept { return size() == 0; }
			size_type size() const noexcept { return length_; }
			size_type max_size() const noexcept {
				return std::numeric_limits<size_type>::max();
			}

			// iterators.
			iterator begin() noexcept {
				iterator rtn_iter = this->end();
				for (size_type i = 0; i < capacity_; i++) {
					if (used_[i] != 0) {
						rtn_iter.node.uptr_ = used_ + i;
						rtn_iter.node.dptr_ = data_ + i;
						rtn_iter.node.eptr_ = used_ + capacity_;
						break;
					}
				}
				return rtn_iter;
			}

			const_iterator begin() const noexcept { return cbegin(); }
			const_iterator cbegin() const noexcept {
				const_iterator rtn_iter = this->end();
				for (size_type i = 0; i < capacity_; i++) {
					if (used_[i] != 0) {
						rtn_iter.node.uptr_ = used_ + i;
						rtn_iter.node.dptr_ = data_ + i;
						rtn_iter.node.eptr_ = used_ + capacity_;
						break;
					}
				}
				return rtn_iter;
			}

			iterator end() noexcept {
				iterator rtn_iter;

				rtn_iter.node.uptr_ = used_ + capacity_;
				rtn_iter.node.dptr_ = data_ + capacity_;
				rtn_iter.node.eptr_ = used_ + capacity_;

				return rtn_iter;
			}

			const_iterator end() const noexcept { return cend(); }
			const_iterator cend() const noexcept {
				const_iterator rtn_iter;

				rtn_iter.node.uptr_ = used_ + capacity_;
				rtn_iter.node.dptr_ = data_ + capacity_;
				rtn_iter.node.eptr_ = used_ + capacity_;

				return rtn_iter;
			}

			// modifiers.
			template <typename... _Args>
			std::pair<iterator, bool> emplace(_Args&&... args) {
				return this->insert(value_type(std::forward<_Args>(args)...));
			}

			template <typename... _Args>
			std::pair<iterator, bool> try_emplace(const key_type& k, _Args&&... args) {
				size_type index = custom_bucket(k, data_, used_, capacity_);
				if (index == capacity_ || load_factor() > max_load_factor()) {
					this->rehash(2 * this->bucket_count());
					index = custom_bucket(k, data_, used_, capacity_);
				}

				if (used_[index] != 1) {
					new (data_ + index) value_type(k, mapped_type(std::forward<_Args>(args)...)); // todo: maybe forward
					used_[index] = 1;
					length_++;
				} else {
					return { this->end(), false };
				}

				iterator some_iter;
				some_iter.node.dptr_ = data_ + index;
				some_iter.node.uptr_ = used_ + index;
				some_iter.node.eptr_ = used_ + capacity_;

				return { some_iter, true };
			}

			template <typename... _Args>
			std::pair<iterator, bool> try_emplace(key_type&& k, _Args&&... args) {
				size_type index = custom_bucket(k, data_, used_, capacity_);
				if (index == capacity_ || load_factor() > max_load_factor()) {
					this->rehash(2 * this->bucket_count());
					index = custom_bucket(k, data_, used_, capacity_);
				}

				if (used_[index] != 1) {
					new (data_ + index) value_type(std::move(k), mapped_type(std::forward<_Args>(args)...)); // todo: maybe forward
					used_[index] = 1;
					length_++;
				} else {
					return { this->end(), false };
				}

				iterator some_iter;
				some_iter.node.dptr_ = data_ + index;
				some_iter.node.uptr_ = used_ + index;
				some_iter.node.eptr_ = used_ + capacity_;

				return { some_iter, true };
			}

			std::pair<iterator, bool> insert(const value_type& x) {
				size_type index = custom_bucket(x.first, data_, used_, capacity_);
				if (index == capacity_ || load_factor() > max_load_factor_) {
					this->rehash(2 * this->bucket_count());
					index = custom_bucket(x.first, data_, used_, capacity_);
				}

				if (used_[index] != 1) {
					new (data_ + index) value_type(x);
					used_[index] = 1;
					length_++;
				} else {
					return { this->end(), false };
				}

				iterator some_iter;
				some_iter.node.dptr_ = data_ + index;
				some_iter.node.uptr_ = used_ + index;
				some_iter.node.eptr_ = used_ + capacity_;

				return { some_iter, true };
			}

			std::pair<iterator, bool> insert(value_type&& x) {
				size_type index = custom_bucket(x.first, data_, used_, capacity_);
				if (index == capacity_ || load_factor() > max_load_factor()) {
					this->rehash(2 * this->bucket_count());
					index = custom_bucket(x.first, data_, used_, capacity_);
				}

				if (used_[index] != 1) {
					new (data_ + index) value_type(std::move(x)); // todo: maybe forward
					used_[index] = 1;
					length_++;
				} else {
					return { this->end(), false };
				}

				iterator some_iter;
				some_iter.node.dptr_ = data_ + index;
				some_iter.node.uptr_ = used_ + index;
				some_iter.node.eptr_ = used_ + capacity_;

				return { some_iter, true };
			}

			template <typename _InputIterator>
			void insert(_InputIterator first, _InputIterator last) {
				for (auto iter = first; iter != last; iter++) {
					this->insert(*iter);
				}
			}

			void insert(std::initializer_list<value_type> l) {
				this->insert(l.begin(), l.end());
			}

			template <typename _Obj>
			std::pair<iterator, bool> insert_or_assign(const key_type& k, _Obj&& obj) {
				auto iter = this->find(k);
				bool insert_rtn = false;
				if (iter == this->end()) {
					this->insert(value_type(k, obj));
					insert_rtn = true;
					iter = this->find(k);
				} else {
					iter->second = obj;
				}
				return { iter, insert_rtn };
			}

			template <typename _Obj>
			std::pair<iterator, bool> insert_or_assign(key_type&& k, _Obj&& obj) {
				auto iter = this->find(k);
				bool insert_rtn = false;
				if (iter == this->end()) {
					this->insert(value_type(std::move(k), obj));
					insert_rtn = true;
					iter = this->find(k);
				} else {
					iter->second = obj;
				}
				return { iter, insert_rtn };
			}

			iterator erase(const_iterator position) {
				if (position == this->end() || *position.node.uptr_ != 1) {
					throw std::runtime_error("Invalid iterator for erase data");
				}

				*position.node.uptr_ = 2;
				position.node.dptr_->second.~mapped_type();
				length_--;

				iterator other_position;

				other_position.node.dptr_ = const_cast<value_type *>(position.node.dptr_);
				other_position.node.eptr_ = position.node.eptr_;
				other_position.node.uptr_ = position.node.uptr_;

				other_position++;

				return other_position;
			}

			iterator erase(iterator position) {
				if (position == this->end() || *position.node.uptr_ != 1) {
					throw std::runtime_error("Invalid iterator for erase data");
				}

				*position.node.uptr_ = 2;
				position.node.dptr_->second.~mapped_type();
				position++;
				length_--;
				return position;
			}

			size_type erase(const key_type& x) {
				auto iter = this->find(x);
				if (iter != this->end()) {
					this->erase(iter);
					return 1;
				}
				return 0;
			}

			iterator erase(const_iterator first, const_iterator last) {
				iterator last_iter;
				last_iter.node.dptr_ = const_cast<value_type*>(last.node.dptr_);
				last_iter.node.eptr_ = last.node.eptr_;
				last_iter.node.uptr_ = last.node.uptr_;

				if (first != last) {
					auto iter = this->erase(first);
					while (iter != last_iter) {
						iter = this->erase(iter);
					}
					return iter;
				}

				return last_iter;
			}

			void clear() noexcept {
				for (size_type i = 0; i < capacity_; i++) {
					if (used_[i] == 1) {
						data_[i].second.~mapped_type();
					}
					used_[i] = 0;
				}
				length_ = 0;
			}

			void swap(hash_map& x) {
				std::swap(x.data_, data_);
				std::swap(x.used_, used_);
				std::swap(x.length_, length_);
				std::swap(x.capacity_, capacity_);
				std::swap(x.allocator_, allocator_);
				std::swap(x.hasher_, hasher_);
				std::swap(x.max_load_factor_, max_load_factor_);
				std::swap(x.pred_, pred_);
			}

			template <typename _H2, typename _P2>
			void merge(hash_map<K, T, _H2, _P2, Alloc>& source) {
				for (auto iter = source.begin(); iter != source.end(); ) {
					if (!this->contains(iter->first)) {
						this->insert(*iter);
						iter = source.erase(iter);
					} else {
						iter++;
					}
				}
			}

			template <typename _H2, typename _P2>
			void merge(hash_map<K, T, _H2, _P2, Alloc>&& source) {
				for (auto iter = source.begin(); iter != source.end(); ) {
					if (!this->contains(iter->first)) {
						this->insert(std::move(*iter));
						iter = source.erase(iter);
					} else {
						iter++;
					}
				}
			}

			// observers.

			///  Returns the hash functor object with which the %hash_map was
			///  constructed.
			Hash hash_function() const {
				return hasher_;
			}

			///  Returns the key comparison object with which the %hash_map was
			///  constructed.
			Pred key_eq() const {
				return pred_;
			}

			// lookup.
			iterator find(const key_type& x) {
				size_type index = custom_bucket(x, data_, used_, capacity_);
				if (index != capacity_ && used_[index] != 1) {
					index = capacity_;
				}

				iterator some_iter;
				some_iter.node.dptr_ = data_ + index;
				some_iter.node.uptr_ = used_ + index;
				some_iter.node.eptr_ = used_ + capacity_;
				return some_iter;
			}
			const_iterator find(const key_type& x) const {
				size_type index = custom_bucket(x, data_, used_, capacity_);
				if (index != capacity_ && used_[index] != 1) {
					index = capacity_;
				}

				const_iterator some_iter;
				some_iter.node.dptr_ = data_ + index;
				some_iter.node.uptr_ = used_ + index;
				some_iter.node.eptr_ = used_ + capacity_;
				return some_iter;
			}

			size_type count(const key_type& x) const {
				return (this->find(x) != this->end() ? 1 : 0);
			}

			bool contains(const key_type& x) const {
				return (this->count(x) == 1);
			}

			mapped_type& operator[](const key_type& k) {
				size_type index = custom_bucket(k, data_, used_, capacity_);
				if (index == capacity_ || load_factor() > max_load_factor()) {
					this->rehash(2 * this->bucket_count());
					index = custom_bucket(k, data_, used_, capacity_);
				}

				if (used_[index] == 0) {
					new (data_ + index) value_type{ k, mapped_type() };
					used_[index] = 1;
					length_++;
				}

				return data_[index].second;
			}
			mapped_type& operator[](key_type&& k) {
				size_type index = custom_bucket(k, data_, used_, capacity_);
				if (index == capacity_ || load_factor() > max_load_factor()) {
					this->rehash(2 * this->bucket_count());
					index = custom_bucket(k, data_, used_, capacity_);
				}

				if (used_[index] == 0) {
					new (data_ + index) value_type{ std::move(k), mapped_type() };
					used_[index] = 1;
					length_++;
				}

				return data_[index].second;
			}

			mapped_type& at(const key_type& k) {
				if (length_ == 0) {
					throw std::out_of_range("Out of range");
				}

				size_type index = custom_bucket(k, data_, used_, capacity_);
				if (index == capacity_ || used_[index] == 0) {
					throw std::out_of_range("Out of range");
				}
				return data_[index].second;
			}
			const mapped_type& at(const key_type& k) const {
				if (length_ == 0) {
					throw std::out_of_range("Out of range");
				}

				size_type index = custom_bucket(k, data_, used_, capacity_);
				if (index == capacity_ || used_[index] == 0) {
					throw std::out_of_range("Out of range");
				}
				return data_[index].second;
			}

			// bucket interface.

			size_type bucket_count() const noexcept { return capacity_; }
			size_type bucket(const key_type& _K) const {
				auto idx = custom_bucket(_K, data_, used_, capacity_);
				if (idx == capacity_ || used_[idx] != 1) {
					throw std::runtime_error("Out of range");
				}
				return idx;
			}

			// hash policy.
			float load_factor() const noexcept { return size() * 1.0f / bucket_count(); }
			float max_load_factor() const noexcept { return max_load_factor_; }
			void max_load_factor(float z) { 
				if (z > 1.0 || z < 0.0) {
					throw std::runtime_error("Max Load Factor must be in range [0.0, 1.0]");
				}
				max_load_factor_ = z;
			}
			void rehash(size_type n) {
				char* n_used = new char[n];

				std::fill_n(n_used, n, static_cast<char>(0));

				value_type* n_data = allocator_.allocate(n);

				for (size_type i = 0; i < capacity_; i++) {
					if (used_[i] == 1) {
						size_type index = custom_bucket(data_[i].first, n_data, n_used, n);
						new (n_data + index) value_type(std::move(data_[i]));
						n_used[index] = 1;
					}
				}

				delete[] used_;
				allocator_.deallocate(data_, capacity_);

				used_ = n_used;
				data_ = n_data;

				capacity_ = n;
			}
			void reserve(size_type n) {
				this->rehash(ceil(n / max_load_factor()));
			}

			bool operator==(const hash_map& other) const {
				if (length_ != other.length_) {
					return false;
				}

				for (auto iter = this->begin(); iter != this->end(); iter++) {
					if (!other.contains(iter->first) || other.at(iter->first) != iter->second) {
						return false;
					}
				}

				return true;
			}

		private:
			size_type custom_bucket(const key_type& _K, value_type* data, char* used, size_type capacity) const {
				if (capacity == 0) return 0;

				size_t first_twos = 0;
				bool finded_twos = false;

				size_t start_index = hasher_(_K) % capacity;
				size_t index = start_index;
				while (used[index] == 2 || used[index] == 1 && !pred_(data[index].first, _K)) {
					if (!finded_twos && used[index] == 2) {
						first_twos = index;
						finded_twos = true;
					}

					index = (index + 1) % capacity;
					if (index == start_index) {
						return capacity_;
					}
				}
	
				return (used[index] == 1 || !finded_twos ? index : first_twos);
			}

			hasher hasher_;
			allocator_type allocator_;
			key_equal pred_;

			float max_load_factor_;

			char* used_;
			value_type* data_;
			size_type length_;
			size_type capacity_;
	};

}  // namespace fefu
