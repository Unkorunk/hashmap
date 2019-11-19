#pragma once

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
			uptr_++;
			dptr_++;

			while (*uptr_ == 0 && uptr_ != eptr_) {
				dptr_++;
				uptr_++;
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

			while (*uptr_ == 0 && uptr_ != eptr_) {
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

			hash_map()
				: max_load_factor_(0.9f),
				capacity_(0),
				length_(0),
				data_(nullptr),
				used_(nullptr),
				allocator_() {
			}

			~hash_map() {
				if (data_ != nullptr) {
					allocator_.deallocate(data_, capacity_);
					delete[] used_;
				}
			}

			explicit hash_map(size_type n)
				: max_load_factor_(0.9f),
				capacity_(n),
				length_(0),
				allocator_(),
				used_(new char[n]()) {
				data_ = allocator_.allocate(n);
				std::fill_n(used_, capacity_, 0);
			}

			template <typename InputIterator>
			hash_map(InputIterator first, InputIterator last, size_type n = 0)
				: hash_map(std::distance(first, last) > n ? std::distance(first, last) : n) {
				this->insert(first, last);
			}

			hash_map(const hash_map& other)
				: max_load_factor_(0.9f),
				length_(other.length_),
				capacity_(other.capacity_),
				allocator_(),
				used_(new char[other.capacity_]) {
				data_ = allocator_.allocate(other.capacity_);
				std::copy(other.data_, other.data_ + other.capacity_, data_);
				std::copy(other.used_, other.used_ + other.capacity_, used_);
			}

			hash_map(hash_map&& other)
				: max_load_factor_(0.9f),
				capacity_(0),
				length_(0),
				allocator_(),
				data_(nullptr),
				used_(nullptr) {
				swap(other);
			}

			explicit hash_map(const allocator_type& a)
				: max_load_factor_(0.9f),
				capacity_(0),
				length_(0),
				data_(nullptr),
				used_(nullptr),
				allocator_(a) {
			}

			hash_map(const hash_map& other, const allocator_type& a)
				: max_load_factor_(0.9f),
				length_(other.length_),
				capacity_(other.capacity_),
				allocator_(a),
				used_(new char[other.capacity_]) {
				data_ = allocator_.allocate(other.capacity_);
				std::copy(other.data_, other.data_ + other.capacity_, data_);
				std::copy(other.used_, other.used_ + other.capacity_, used_);
			}

			hash_map(hash_map&& other, const allocator_type& a)
				: max_load_factor_(0.9f),
				capacity_(0),
				length_(0),
				allocator_(),
				data_(nullptr),
				used_(nullptr) {
				swap(other);
				allocator_ = a;
			}

			hash_map(std::initializer_list<value_type> l, size_type n = 0)
				: hash_map((l.size() > n ? l.size() : n)) {
				for (auto& vls : l) {
					this->operator[](vls.first) = vls.second;
				}
			}

			/// Copy assignment operator.
			hash_map& operator=(const hash_map& other) {
				max_load_factor_ = 0.9f;
				length_ = other.length_;
				capacity_ = other.capacity_;
				data_ = allocator_.allocate(other.capacity_);
				used_ = new char[other.capacity_];

				std::copy(other.data_, other.data_ + other.capacity_, data_);
				std::copy(other.used_, other.used_ + other.capacity_, used_);
			}

			/// Move assignment operator.
			hash_map& operator=(hash_map&& other) {
				max_load_factor_ = 0.9f;
				capacity_ = 0;
				length_ = 0;
				data_ = nullptr;
				used_ = nullptr;

				swap(other);
			}

			hash_map& operator=(std::initializer_list<value_type> l) {
				max_load_factor_ = 0.9f;
				capacity_ = l.size();
				length_ = 0;
				data_ = allocator_.allocate(l.size());
				used_ = new char[l.size()]();
				std::fill_n(used_, capacity_, 0);
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
			std::pair<iterator, bool> emplace(_Args&&... args);

			template <typename... _Args>
			std::pair<iterator, bool> try_emplace(const key_type& k, _Args&&... args);

			// move-capable overload
			template <typename... _Args>
			std::pair<iterator, bool> try_emplace(key_type&& k, _Args&&... args);

			std::pair<iterator, bool> insert(const value_type& x) {
				size_type index = bucket(x.first);

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
			}

			std::pair<iterator, bool> insert(value_type&& x) { // TODO: second arg
				size_type index = bucket(x.first);

				if (used_[index] == 0) {
					new (data_ + index) value_type(std::move(x));
					used_[index] = 1;
					length_++;
				}

				iterator some_iter;
				some_iter.dptr_ = data_ + index;
				some_iter.uptr_ = used_ + index;
				some_iter.eptr_ = used_ + capacity_;

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
			std::pair<iterator, bool> insert_or_assign(const key_type& k, _Obj&& obj);

			// move-capable overload
			template <typename _Obj>
			std::pair<iterator, bool> insert_or_assign(key_type&& k, _Obj&& obj);

			iterator erase(const_iterator position);

			iterator erase(iterator position);

			size_type erase(const key_type& x);

			iterator erase(const_iterator first, const_iterator last);

			void clear() noexcept;

			void swap(hash_map& x) {
				swap(x.data_, data_);
				swap(x.used_, used_);
				swap(x.length_, length_);
				swap(x.capacity_, capacity_);
			}

			template <typename _H2, typename _P2>
			void merge(hash_map<K, T, _H2, _P2, Alloc>& source);

			template <typename _H2, typename _P2>
			void merge(hash_map<K, T, _H2, _P2, Alloc>&& source);

			// observers.

			///  Returns the hash functor object with which the %hash_map was
			///  constructed.
			Hash hash_function() const { return hasher_; }

			///  Returns the key comparison object with which the %hash_map was
			///  constructed.
			Pred key_eq() const { return pred_; }

			// lookup.
			iterator find(const key_type& x);
			const_iterator find(const key_type& x) const;

			size_type count(const key_type& x) const;

			bool contains(const key_type& x) const {
				for (int i = 0; i < capacity_; i++) {
					if (used_[i] == 1 && pred_(data_[i].first, x)) {
						return true;
					}
				}
				return false;
			}

			mapped_type& operator[](const key_type& k) {
				size_type index = bucket(k);

				if (used_[index] == 0) {
					new (data_ + index) value_type{ k, mapped_type() };
					used_[index] = 1;
					length_++;
				}

				return data_[index].second;
			}
			mapped_type& operator[](key_type&& k) {
				size_type index = bucket(k);

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
				size_t index = hasher_(_K) % capacity_;
				while (used_[index] == 1 && !pred_(data_[index].first, _K)) {
					index = (index + 1) % capacity_;
				}
				return index;
			}

			// hash policy.
			float load_factor() const noexcept { return size() * 1.0f / bucket_count(); }
			float max_load_factor() const noexcept { return max_load_factor_; }
			void max_load_factor(float z) { max_load_factor_ = z; }
			void rehash(size_type n);
			void reserve(size_type n);

			bool operator==(const hash_map& other) const;

		private:
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
