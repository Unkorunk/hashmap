#pragma once

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

		allocator() noexcept = default;

		allocator(const allocator&) noexcept = default;

		template <class U>
		allocator(const allocator<U>&) noexcept {}

		~allocator() = default;

		pointer allocate(size_type n) {
			return (pointer)::operator new(n * sizeof(value_type));
		}

		void deallocate(pointer p, size_type n) noexcept { ::operator delete(p, n); }
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
			: dptr_(nullptr), uptr_(nullptr), eptr_(nullptr) {
		}
		hash_map_iterator(const hash_map_iterator& other) noexcept
			: dptr_(other.dptr_), uptr_(other.uptr_), eptr_(other.eptr_) {
		}

		reference operator*() const { return *dptr_; }
		pointer operator->() const { return dptr_; }

		// prefix ++
		hash_map_iterator& operator++() {
			if (uptr_ != eptr_) {
				uptr_++;
				dptr_++;

				while (*uptr_ == 0 && uptr_ != eptr_) {
					dptr_++;
					uptr_++;
				}
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
		pointer dptr_;
		char* uptr_;
		char* eptr_;
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
			: dptr_(nullptr), uptr_(nullptr), eptr_(nullptr) {}
		hash_map_const_iterator(const hash_map_const_iterator& other) noexcept 
			: dptr_(other.dptr_), uptr_(other.uptr_), eptr_(other.eptr_) {}
		hash_map_const_iterator(const hash_map_iterator<ValueType>& other) noexcept
			: dptr_(other.dptr_), uptr_(other.uptr_), eptr_(other.eptr_) {}

		reference operator*() const { return *dptr_; }
		pointer operator->() const { return dptr_; }

		// prefix ++
		hash_map_const_iterator& operator++() {
			uptr_++;
			dptr_++;

			while (*uptr_ != 1 && uptr_ != eptr_) {
				dptr_++;
				uptr_++;
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
		pointer dptr_;
		char* uptr_;
		char* eptr_;
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
				: max_load_factor_(0.45f), length_(0), allocator_() {
				
				capacity_ = std::min(static_cast<size_type>(1), n);
				used_ = new char[capacity_];
				data_ = allocator_.allocate(capacity_);
				std::fill_n(used_, capacity_, static_cast<char>(0));
			}

			template <typename InputIterator>
			hash_map(InputIterator first, InputIterator last, size_type n = 1)
				: hash_map(std::distance(first, last) > n ? std::distance(first, last) : n) {
				this->insert(first, last);
			}

			hash_map(const hash_map& other)
				: max_load_factor_(0.45f),
				length_(other.length_),
				capacity_(other.capacity_),
				allocator_(),
				used_(new char[other.capacity_]) {
				data_ = allocator_.allocate(other.capacity_);
				std::copy(other.data_, other.data_ + other.capacity_, data_);
				std::copy(other.used_, other.used_ + other.capacity_, used_);
			}

			hash_map(hash_map&& other)
				: max_load_factor_(0.45f),
				capacity_(0),
				length_(0),
				allocator_(),
				data_(nullptr),
				used_(nullptr) {
				swap(other);
			}

			explicit hash_map(const allocator_type& a)
				: max_load_factor_(0.45f),
				capacity_(0),
				length_(0),
				data_(nullptr),
				used_(nullptr),
				allocator_(a) {
			}

			hash_map(const hash_map& other, const allocator_type& a)
				: max_load_factor_(0.45f),
				length_(other.length_),
				capacity_(other.capacity_),
				allocator_(a),
				used_(new char[other.capacity_]) {
				data_ = allocator_.allocate(other.capacity_);
				std::copy(other.data_, other.data_ + other.capacity_, data_);
				std::copy(other.used_, other.used_ + other.capacity_, used_);
			}

			hash_map(hash_map&& other, const allocator_type& a)
				: max_load_factor_(0.45f),
				capacity_(0),
				length_(0),
				allocator_(),
				data_(nullptr),
				used_(nullptr) {
				swap(other);
				allocator_ = a;
			}

			hash_map(std::initializer_list<value_type> l, size_type n = 1) 
				: max_load_factor_(0.45f), length_(0), allocator_() {

				capacity_ = std::max(l.size(), std::max(static_cast<size_type>(1), n));
				used_ = new char[capacity_];
				data_ = allocator_.allocate(capacity_);
				std::fill_n(used_, capacity_, static_cast<char>(0));

				this->insert(l.begin(), l.end());
			}


			/// Copy assignment operator.
			hash_map& operator=(const hash_map& other) {
				max_load_factor_ = 0.45f;
				length_ = other.length_;
				capacity_ = other.capacity_;
				data_ = allocator_.allocate(other.capacity_);
				used_ = new char[other.capacity_];

				std::copy(other.data_, other.data_ + other.capacity_, data_);
				std::copy(other.used_, other.used_ + other.capacity_, used_);
			}

			/// Move assignment operator.
			hash_map& operator=(hash_map&& other) {
				max_load_factor_ = 0.45f;
				capacity_ = 0;
				length_ = 0;
				data_ = nullptr;
				used_ = nullptr;

				swap(other);
			}

			hash_map& operator=(std::initializer_list<value_type> l) {
				max_load_factor_ = 0.45f;
				capacity_ = l.size();
				length_ = 0;
				data_ = allocator_.allocate(l.size());
				used_ = new char[l.size()]();
				std::fill_n(used_, capacity_, static_cast<char>(0));
				for (auto& vls : l) {
					this->operator[](vls.first) = vls.second;
				}
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
				iterator rtn_iter;
				for (size_type i = 0; i < capacity_; i++) {
					if (used_[i] != 0) {
						rtn_iter.uptr_ = used_ + i;
						rtn_iter.dptr_ = data_ + i;
						rtn_iter.eptr_ = used_ + capacity_;
						break;
					}
				}
				return rtn_iter;
			}

			const_iterator begin() const noexcept { return cbegin(); }
			const_iterator cbegin() const noexcept {
				const_iterator rtn_iter;
				for (size_type i = 0; i < capacity_; i++) {
					if (used_[i] != 0) {
						rtn_iter.uptr_ = used_ + i;
						rtn_iter.dptr_ = data_ + i;
						rtn_iter.eptr_ = used_ + capacity_;
						break;
					}
				}
				return rtn_iter;
			}

			iterator end() noexcept {
				iterator rtn_iter;

				rtn_iter.uptr_ = used_ + capacity_;
				rtn_iter.dptr_ = data_ + capacity_;
				rtn_iter.eptr_ = used_ + capacity_;

				return rtn_iter;
			}

			const_iterator end() const noexcept { return cend(); }
			const_iterator cend() const noexcept {
				const_iterator rtn_iter;

				rtn_iter.uptr_ = used_ + capacity_;
				rtn_iter.dptr_ = data_ + capacity_;
				rtn_iter.eptr_ = used_ + capacity_;

				return rtn_iter;
			}

			// modifiers.
			template <typename... _Args>
			std::pair<iterator, bool> emplace(_Args&&... args) {
				return this->insert(value_type(std::forward<_Args>(args)...));
			}

			template <typename... _Args>
			std::pair<iterator, bool> try_emplace(const key_type& k, _Args&&... args) {
				return this->insert(value_type(k, mapped_type(std::forward<_Args>(args)...))); // TODO: on place
			}

			template <typename... _Args>
			std::pair<iterator, bool> try_emplace(key_type&& k, _Args&&... args) {
				return this->insert(value_type(std::move(k), mapped_type(std::forward<_Args>(args)...))); // TODO: on place
			}

			std::pair<iterator, bool> insert(const value_type& x) {
				try {
					size_type index = bucket(x.first);
					if (index == capacity_) {
						this->rehash(2 * this->bucket_count());
						index = bucket(x.first);
					}

					if (used_[index] == 0) {
						new (data_ + index) value_type(x);
						used_[index] = 1;
						length_++;
					}

					iterator some_iter;
					some_iter.dptr_ = data_ + index;
					some_iter.uptr_ = used_ + index;
					some_iter.eptr_ = used_ + capacity_;

					return { some_iter, true };
				} catch (...) {
					return { this->end(), false };
				}
			}

			std::pair<iterator, bool> insert(value_type&& x) {
				try {
					size_type index = bucket(x.first);
					if (index == capacity_) {
						this->rehash(2 * this->bucket_count());
						index = bucket(x.first);
					}

					if (used_[index] == 0) {
						new (data_ + index) value_type(std::move(x)); // todo: maybe forward
						used_[index] = 1;
						length_++;
					}

					iterator some_iter;
					some_iter.dptr_ = data_ + index;
					some_iter.uptr_ = used_ + index;
					some_iter.eptr_ = used_ + capacity_;

					return { some_iter, true };
				} catch (...) {
					return { this->end(), false };
				}
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
				if (iter == this->end()) {
					this->insert(value_type(k, obj));
				} else {
					iter->second = obj;
				}
			}

			template <typename _Obj>
			std::pair<iterator, bool> insert_or_assign(key_type&& k, _Obj&& obj) {
				auto iter = this->find(k);
				if (iter == this->end()) {
					this->insert(value_type(std::move(k), obj));
				} else {
					iter->second = obj;
				}
			}

			iterator erase(const_iterator position) {
				*position.uptr_ = 2;
				position.dptr_->second.~mapped_type();
				position++;
				length_--;
				return position;
			}

			iterator erase(iterator position) {
				*position.uptr_ = 2;
				position.dptr_->second.~mapped_type();
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
				for (auto iter = first; iter != last; ) {
					this->erase(iter);
				}
			}

			void clear() noexcept {
				for (size_type i = 0; i < capacity_; i++) {
					if (used_[i] == 1) {
						data_[i].second.~mapped_type();
					}
					used_[i] = 0;
				}
			}

			void swap(hash_map& x) {
				swap(x.data_, data_);
				swap(x.used_, used_);
				swap(x.length_, length_);
				swap(x.capacity_, capacity_);
				swap(x.allocator_, allocator_);
				swap(x.hasher_, hasher_);
				swap(x.max_load_factor_, max_load_factor_);
				swap(x.pred_, pred_);
			}

			template <typename _H2, typename _P2>
			void merge(hash_map<K, T, _H2, _P2, Alloc>& source) {
				for (auto iter = source.begin(); iter != source.end(); iter++) {
					if (!this->contains(iter->first)) {
						this->insert(*iter);
						iter = source.erase(iter);
					}
				}
			}

			template <typename _H2, typename _P2>
			void merge(hash_map<K, T, _H2, _P2, Alloc>&& source) {
				for (auto iter = source.begin(); iter != source.end(); iter++) {
					if (!this->contains(iter->first)) {
						this->insert(std::move(*iter));
						iter = source.erase(iter);
					}
				}
			}

			// observers.

			///  Returns the hash functor object with which the %hash_map was
			///  constructed.
			Hash hash_function() const { return hasher_; }

			///  Returns the key comparison object with which the %hash_map was
			///  constructed.
			Pred key_eq() const { return pred_; }

			// lookup.
			iterator find(const key_type& x) {
				size_type index = custom_bucket(x, data_, used_, capacity_, false);
				if (!used_[index]) {
					index = capacity_;
				}

				iterator some_iter;
				some_iter.dptr_ = data_ + index;
				some_iter.uptr_ = used_ + index;
				some_iter.eptr_ = used_ + capacity_;
				return some_iter;
			}
			const_iterator find(const key_type& x) const {
				size_type index = bucket(x);
				if (used_[index] != 1) {
					index = capacity_;
				}

				const_iterator some_iter;
				some_iter.dptr_ = data_ + index;
				some_iter.uptr_ = used_ + index;
				some_iter.eptr_ = used_ + capacity_;
				return some_iter;
			}

			size_type count(const key_type& x) const {
				return (this->find(x) != this->end() ? 1 : 0);
			}

			bool contains(const key_type& x) const {
				return (this->count(x) == 1);
			}

			mapped_type& operator[](const key_type& k) {
				size_type index = bucket(k);
				if (index == capacity_) {
					this->rehash(2 * this->bucket_count());
					index = bucket(k);
				}

				if (used_[index] == 0) {
					new (data_ + index) value_type{ k, mapped_type() };
					used_[index] = 1;
					length_++;
				}

				return data_[index].second;
			}
			mapped_type& operator[](key_type&& k) {
				size_type index = bucket(k);
				if (index == capacity_) {
					this->rehash(2 * this->bucket_count());
					index = bucket(k);
				}

				if (used_[index] == 0) {
					new (data_ + index) value_type{ std::move(k), mapped_type() };
					used_[index] = 1;
					length_++;
				}

				return data_[index].second;
			}

			mapped_type& at(const key_type& k) {
				size_type index = bucket(k);
				if (used_[index] == 0) {
					throw std::out_of_range("Out of range");
				}
				return data_[index].second;
			}
			const mapped_type& at(const key_type& k) const {
				size_type index = bucket(k);
				if (used_[index] == 0) {
					throw std::out_of_range("Out of range");
				}
				return data_[index].second;
			}

			// bucket interface.

			size_type bucket_count() const noexcept { return capacity_; }
			size_type bucket(const key_type& _K) const {
				return custom_bucket(_K, data_, used_, capacity_);
			}

			// hash policy.
			float load_factor() const noexcept { return size() * 1.0f / bucket_count(); }
			float max_load_factor() const noexcept { return max_load_factor_; }
			void max_load_factor(float z) { max_load_factor_ = z; }
			void rehash(size_type n) {
				n = capacity_ + n;

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
				if (other.capacity_ != this->capacity_ || other.length_ != this->length_ ||
					std::abs(max_load_factor_ - other.max_load_factor) > 1e-7) {
					return false;
				}

				for (size_type i = 0; i < capacity_; i++) {
					if (this->used_[i] != other.used_[i]) {
						return false;
					} else if (used_[i] == 1 && data_[i] != other.data_[i]) {
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
	
				if (used[index] == 1) {
					return index;
				} else {
					if (finded_twos) {
						return first_twos;
					} else {
						return index;
					}
				}
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
