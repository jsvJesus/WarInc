#ifndef TL_FIXEDALLOCATOR_H
#define TL_FIXEDALLOCATOR_H

#ifdef _DEBUG
#ifdef ENABLE_MEMORY_DEBUG
#undef new
#endif
#endif

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

namespace r3dTL
{
	template<typename T>
	class TFixedAllocator
	{
	public:
		template<typename U>
		friend class TFixedAllocator;

		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		typedef std::false_type propagate_on_container_copy_assignment;
		typedef std::false_type propagate_on_container_move_assignment;
		typedef std::false_type propagate_on_container_swap;
		typedef std::true_type is_always_equal;

		template<typename U>
		struct rebind
		{
			typedef TFixedAllocator<U> other;
		};

		TFixedAllocator()
			: mArea(NULL)
			, mPtr(0)
			, mMaxItems(0)
		{
		}

		explicit TFixedAllocator(size_type maxItems)
			: mArea(NULL)
			, mPtr(0)
			, mMaxItems(maxItems)
		{
			if (mMaxItems > 0)
			{
				mArea = static_cast<T*>(std::malloc(mMaxItems * sizeof(T)));

				if (!mArea)
				{
					throw std::bad_alloc();
				}
			}
		}

		TFixedAllocator(const TFixedAllocator& cpy)
			: mArea(NULL)
			, mPtr(0)
			, mMaxItems(cpy.mMaxItems)
		{
			if (mMaxItems > 0)
			{
				mArea = static_cast<T*>(std::malloc(mMaxItems * sizeof(T)));

				if (!mArea)
				{
					throw std::bad_alloc();
				}
			}
		}

		template<typename U>
		TFixedAllocator(const TFixedAllocator<U>& cpy)
			: mArea(NULL)
			, mPtr(0)
			, mMaxItems(cpy.mMaxItems)
		{
			if (mMaxItems > 0)
			{
				mArea = static_cast<T*>(std::malloc(mMaxItems * sizeof(T)));

				if (!mArea)
				{
					throw std::bad_alloc();
				}
			}
		}

		~TFixedAllocator()
		{
			std::free(mArea);
			mArea = NULL;
			mPtr = 0;
			mMaxItems = 0;
		}

		pointer address(reference r) const
		{
			return &r;
		}

		const_pointer address(const_reference r) const
		{
			return &r;
		}

		pointer allocate(size_type cnt)
		{
			if (cnt == 0)
			{
				return NULL;
			}

			if (!mArea || cnt > (mMaxItems - mPtr))
			{
				r3d_assert(false && "TFixedAllocator overflow");
				throw std::bad_alloc();
			}

			pointer result = mArea + mPtr;
			mPtr += cnt;

			return result;
		}

		pointer allocate(size_type cnt, const void*)
		{
			return allocate(cnt);
		}

		void deallocate(pointer, size_type)
		{
		}

		size_type max_size() const
		{
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		template<typename U, typename... Args>
		void construct(U* p, Args&&... args)
		{
			::new(static_cast<void*>(p)) U(std::forward<Args>(args)...);
		}

		template<typename U>
		void destroy(U* p)
		{
			p->~U();
		}

		bool operator==(const TFixedAllocator&) const
		{
			return true;
		}

		bool operator!=(const TFixedAllocator& a) const
		{
			return !operator==(a);
		}

	private:
		T* mArea;
		size_type mPtr;
		size_type mMaxItems;
	};
}

#endif