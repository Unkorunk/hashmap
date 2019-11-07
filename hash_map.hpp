#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

namespace fefu {

	template<typename T>
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

		void deallocate(pointer p, size_type n) noexcept {
			::operator delete(p, n);
		}
	};

	template<typename ValueType>
	class hash_map_iterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = ValueType;
		using difference_type = std::ptrdiff_t;
		using reference = ValueType&;
		using pointer = ValueType*;

		hash_map_iterator() noexcept;
		hash_map_iterator(const hash_map_iterator& other) noexcept;

		reference operator*() const;
		pointer operator->() const;

		// prefix ++
		hash_map_iterator& operator++();
		// postfix ++
		hash_map_iterator operator++(int);

		friend bool operator==(const hash_map_iterator<ValueType>&, const hash_map_iterator<ValueType>&);
		friend bool operator!=(const hash_map_iterator<ValueType>&, const hash_map_iterator<ValueType>&);
	};

	template<typename ValueType>
	class hash_map_const_iterator {
		// Shouldn't give non const references on value
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = ValueType;
		using difference_type = std::ptrdiff_t;
		using reference = const ValueType&;
		using pointer = const ValueType*;

		hash_map_const_iterator() noexcept;
		hash_map_const_iterator(const hash_map_const_iterator& other) noexcept;
		hash_map_const_iterator(const hash_map_iterator<ValueType>& other) noexcept;

		reference operator*() const;
		pointer operator->() const;

		// prefix ++
		hash_map_const_iterator& operator++();
		// postfix ++
		hash_map_const_iterator operator++(int);

		friend bool operator==(const hash_map_const_iterator<ValueType>&, const hash_map_const_iterator<ValueType>&);
		friend bool operator!=(const hash_map_const_iterator<ValueType>&, const hash_map_const_iterator<ValueType>&);
	};
	template<typename K, typename T,
		typename Hash = std::hash<K>,
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

			hash_map() = default;

			~hash_map() {
				if (data_ != nullptr) {
					allocator_.deallocate(data_, capacity_);
					delete[] used_;
				}
			}

			explicit hash_map(size_type n) : 
				max_load_factor_(0.9f),
				capacity_(n),
				length_(0),
				data_(allocator_.allocate(n)), 
				used_(new bool[n]()) { 
			}

			template<typename InputIterator>
			hash_map(InputIterator first, InputIterator last, size_type n = 0);

			hash_map(const hash_map& other) :
				max_load_factor_(0.9f),
				length_(other.length_),
				capacity_(other.capacity_),
				data_(allocator_.allocate(other.capacity_)),
				used_(new bool[other.capacity_]) {
				std::copy(other.data_, other.data_ + other.capacity_, data_);
				std::copy(other.used_, other.used_ + other.capacity_, used_);
			}

			hash_map(hash_map&& other) : max_load_factor_(0.9f), capacity_(0), length_(0), data_(nullptr), used_(nullptr) {
				swap(other);
			}

			explicit hash_map(const allocator_type& a);

			hash_map(const hash_map& umap, const allocator_type& a);

			hash_map(hash_map&& umap, const allocator_type& a);

			hash_map(std::initializer_list<value_type> l, size_type n = 0) : hash_map( (l.size() > n ? l.size() : n) ) {
				for (auto& vls : l) {
					this->operator[](vls.first) = vls.second;
				}
			}

			/// Copy assignment operator.
			hash_map& operator=(const hash_map&);

			/// Move assignment operator.
			hash_map& operator=(hash_map&&);

			hash_map& operator=(std::initializer_list<value_type> l);

			///  Returns the allocator object used by the %hash_map.
			allocator_type get_allocator() const noexcept;

			// size and capacity:
			bool empty() const noexcept {
				return size() == 0;
			}
			size_type size() const noexcept {
				return length_;
			}
			size_type max_size() const noexcept {
				return std::numeric_limits<size_type>::max();
			}

			// iterators.
			iterator begin() noexcept;

			const_iterator begin() const noexcept {
				return cbegin();
			}
			const_iterator cbegin() const noexcept;

			iterator end() noexcept;

			const_iterator end() const noexcept {
				return cend();
			}
			const_iterator cend() const noexcept;

			// modifiers.
			template<typename... _Args>
			std::pair<iterator, bool> emplace(_Args&&... args);

			template<typename... _Args>
			iterator emplace_hint(const_iterator pos, _Args&&... args);

			template <typename... _Args>
			std::pair<iterator, bool> try_emplace(const key_type& k, _Args&&... args);

			// move-capable overload
			template <typename... _Args>
			std::pair<iterator, bool> try_emplace(key_type&& k, _Args&&... args);

			template <typename... _Args>
			iterator try_emplace(const_iterator hint, const key_type& k,
				_Args&&... args);

			// move-capable overload
			template <typename... _Args>
			iterator try_emplace(const_iterator hint, key_type&& k, _Args&&... args);

			std::pair<iterator, bool> insert(const value_type& x);

			std::pair<iterator, bool> insert(value_type&& x);

			iterator insert(const_iterator hint, const value_type& x);

			iterator insert(const_iterator hint, value_type&& x);

			template<typename _InputIterator>
			void insert(_InputIterator first, _InputIterator last);

			void insert(std::initializer_list<value_type> l);

			template <typename _Obj>
			std::pair<iterator, bool> insert_or_assign(const key_type& k, _Obj&& obj);

			// move-capable overload
			template <typename _Obj>
			std::pair<iterator, bool> insert_or_assign(key_type&& k, _Obj&& obj);

			template <typename _Obj>
			iterator insert_or_assign(const_iterator hint, const key_type& k,
				_Obj&& obj);

			// move-capable overload
			template <typename _Obj>
			iterator insert_or_assign(const_iterator hint, key_type&& k, _Obj&& obj);

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

			template<typename _H2, typename _P2>
			void merge(hash_map<K, T, _H2, _P2, Alloc>& source);

			template<typename _H2, typename _P2>
			void merge(hash_map<K, T, _H2, _P2, Alloc>&& source);

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
			iterator find(const key_type& x);
			const_iterator find(const key_type& x) const;
			
			size_type count(const key_type& x) const;

			bool contains(const key_type& x) const {
				for (int i = 0; i < capacity_; i++) {
					if (used_[i] && pred_(data_[i].first, x)) {
						return true;
					}
				}
				return false;
			}

			mapped_type& operator[](const key_type& k) {
				size_type index = bucket(k);

				if (!used_[index]) {
					new(data_ + index) value_type{ k, mapped_type() };
					used_[index] = true;
					length_++;
				}

				return data_[index].second;
			}
			mapped_type& operator[](key_type&& k) { // TODO: What's different between lvalue reference and this?
				size_type index = bucket(k);

				if (!used_[index]) {
					new(data_ + index) value_type{ k, mapped_type() };
					used_[index] = true;
					length_++;
				}

				return data_[index].second;
			}

			mapped_type& at(const key_type& k) {
				size_type index = bucket(k);
				if (!used_[index]) {
					throw std::out_of_range("Out of range");
				}
				return data_[index].second;
			}
			const mapped_type& at(const key_type& k) const {
				size_type index = bucket(k);
				if (!used_[index]) {
					throw std::out_of_range("Out of range");
				}
				return data_[index].second;
			}

			// bucket interface.

			size_type bucket_count() const noexcept {
				return capacity_;
			}
			size_type bucket(const key_type& _K) const {
				size_t index = hasher_(_K) % capacity_;
				while (used_[index] && !pred_(data_[index].first, _K)) {
					index = (index + 1) % capacity_;
				}
				return index;
			}

			// hash policy.
			float load_factor() const noexcept {
				return size() * 1f / bucket_count();
			}
			float max_load_factor() const noexcept {
				return max_load_factor_;
			}
			void max_load_factor(float z) {
				max_load_factor_ = z;
			}
			void rehash(size_type n);
			void reserve(size_type n);

			bool operator==(const hash_map& other) const;
		private:
			hasher hasher_;
			allocator_type allocator_;
			key_equal pred_;

			float max_load_factor_;

			bool* used_;
			value_type* data_;
			size_type length_;
			size_type capacity_;
	};

} // namespace fefu
