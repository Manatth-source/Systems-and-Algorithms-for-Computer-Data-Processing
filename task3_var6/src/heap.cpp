#pragma once
#include <vector>
#include <cstddef>
#include <functional>
#include <utility>
#include <stdexcept>
#include <algorithm>


namespace MyHeap
{
	template <class T, class Compare = std::greater<T>>
	class Heap
	{
	public:
		const T& peek() const
		{
			if (data_.empty()) throw std::out_of_range("peek from empty heap");

			return data_[0];
		}


		T pop()
		{
			if (data_.empty()) throw std::out_of_range("pop from empty heap");

			T top = std::move(data_[0]);
			T last = std::move(data_.back());
			data_.pop_back();

			if (!data_.empty())
			{
				data_[0] = std::move(last);
				descend(0);
			}

			return top;
		}


		std::size_t size()
		{
			return data_.size();
		}


		void push(T item)
		{
			data_.push_back(std::move(item));
			surface(data_.size() - 1);
		}


		void heapify(std::vector<T>&& items)
		{
			data_ = std::move(items);

			for (std::size_t i = data_.size() / 2; i > 0; --i)
				descend(i - 1);
		}


		T replaceTop(T item) {
			if (data_.empty()) throw std::out_of_range("replaceTop on empty heap");

			T old = std::move(data_[0]);

			data_[0] = std::move(item);
			descend(0);

			return old;
		}

	private:
		std::vector<T> data_;
		Compare before_;


		void surface(std::size_t i)
		{
			while (i > 0) {
				std::size_t p = parent(i);

				if (before_(data_[i], data_[p])) {
					std::swap(data_[i], data_[p]);
					i = p;
				}
				else
					break;
			}
		}


		void descend(std::size_t i)
		{
			while (true) {
				std::size_t best = i;
				std::size_t l = left(i), r = right(i);
				if (before_(data_[left], data_[i])) best = l;
				else if (before_(data_[right], data_[i])) best = r;

				if (best == i) break;
				std::swap(data_[i], data_[best]);
				i = best;
			}
		}


		std::size_t left(std::size_t i)
		{
			return std::min(2 * i + 1, data_.size() - 1);
		}


		std::size_t right(std::size_t i)
		{
			return std::min(2 * i + 2, data_.size() - 1);
		}


		std::size_t parent(std::size_t i)
		{
			return std::max(static_cast<std::size_t>(0), (i - 1) / 2);
		}
	};
}