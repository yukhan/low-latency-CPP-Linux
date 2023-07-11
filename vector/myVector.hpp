
#include <iostream>
#include <type_traits>
#include <vector>
#include <initializer_list>
using namespace std;
template<typename T>
class myVector
{
public:
	myVector() = default;

	/* support construct like myVector v(begin(), end()); */
	template<typename Iterator>
	myVector(Iterator const& start, Iterator const& end)
	{
		auto const sz  = end - start;
		data_ =reinterpret_cast<T*> ( new char [sizeof(T) * sz]);
		end_ = data_ + sz ;
		capacity_ = data_ + sz;

		for(auto current =start, i=0; current != end; ++current, ++i)
			new ((void*)(data_ + i)) T(*current);

	}

	/* support construct like myVector v{1,2,3,4}; */
	myVector(initializer_list<T> const& inList): myVector(inList.begin(), inList.end())
	{
	}
	~myVector()
	{
		delete [] data_;
		data_ = nullptr;
	}

	
	myVector(myVector const& rhs)
	{
		data_ = reinterpret_cast<T*> (new char [sizeof(T) * rhs.size()]);
		end_ = data_ + rhs.size();
		capacity_ = data_ + rhs.capacity();

		for(int i=0; i < rhs.size(); ++i)
			new ((void*)(data_ + i)) T(rhs.data_[i]);

	}
	myVector& operator=(myVector const& rhs)
	{
		/* copy and swap idiom */
		myVector tmp(rhs);
		swap(*this, tmp);
		return *this;
	}

	/* move constructor*/
	myVector(myVector&& rhs) noexcept
	{
		swap(*this, rhs);
	}

	/* move operator */
	myVector& operator=(myVector&& rhs) noexcept
	{
		myVector tmp(std::move(rhs));
		swap(*this, tmp);

		return *this;
	}
	

	T& operator[](int idx)
	{
		return data_[idx];
	}

	void push_back(T const&  element)
	{
		checkCapacityAndExpand();

		*end_ = element;
		++end_;

	}

	void push_back(T&& element)
	{
		checkCapacityAndExpand();

		new ((void*)end_) T(std::move(element));
		++end_;

	}

	template<typename... Args>
	void emplace_back(Args... arg)
	{
		checkCapacityAndExpand();
		new ((void*)end_) T(arg...);
		++end_;

	}

	T pop_back()
	{
		return *--end_;
	}

	/* begin() and end() needed to support for(type : myVector) */
	T* begin()
	{
		return data_;
	}

	T* end()
	{
		return end_;
	}
	int size() const
	{
		return (end_ - data_);
	}

	int capacity() const
	{
		return (capacity_ - data_);
	}


private:
	void swap(myVector& lhs, myVector& rhs)
	{
		std::swap(lhs.data_, rhs.data_);
		std::swap(lhs.end_, rhs.end_);
		std::swap(lhs.capacity_, rhs.capacity_);
	}
	template<typename U>
	std::enable_if_t<std::is_nothrow_move_constructible_v<U> == true>
	copyMove(U* dst)
	{
		const int sz = size();
		for(int i =0; i < sz; ++i)
			dst[i] = std::move(data_[i]);


	}

	template<typename U>
	std::enable_if_t<std::is_nothrow_move_constructible_v<U> == false>
	copyMove(U* dst)
	{
		const int sz = size();
		for(int i =0; i < sz; ++i)
			dst[i] = data_[i];

	}

	void checkCapacityAndExpand()
	{
		if(end_ == capacity_)
		{
			/* calculate the bytes for allocation to expand the
			 * capacity_. we always allocate twice the current size
			 */
			uint64_t bytes = (sizeof(T)*(size()+1)) << 1 ;
			auto ptr = new char [bytes];

			/* now copy or move the existing elements
			 * into the new expanded buffer
			 */
			copyMove((reinterpret_cast<T*> (ptr)));

			/* get the number of current elements */
			int const sz = size();

			/*as data_ has been moved to new buffer,
			 * its safe to delete the existing one
			 */
			delete [] data_;

			/*data_ points to new buffer*/
			data_ = reinterpret_cast<T*> (ptr);
			/* end_ is updated to point new buffer */
			end_ = data_ + sz;

			/* capacity_ should point to end_ of buffer
			 * we could easily find the number of elements
			 * pre-allocated by using num_elements = bytes/sizeof(T)
			 * but division is expensive hence we us below mechanism
			 */
			capacity_ = data_ + ((size() + 1) << 1);
		}

	}


private:
	T* data_ = nullptr;
	T* end_ = nullptr;
	T* capacity_= nullptr;
};

struct BigClass
{
	BigClass(int xx, int yy):x(xx), y(yy){};
	int x;
	int y;
};
int main()
{
	myVector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	cout<<"myVector size = "<< v.size() << "\n";
	cout<<"popped value = " << v.pop_back() << "\n";
	cout<<"popped value = " << v.pop_back() << "\n";
	cout<<"popped value = " << v.pop_back() << "\n";
	cout<<"popped value = " << v.pop_back() << "\n";
	cout<<"myVector size after pop = "<< v.size() << "\n";
	v.push_back(4);
	v.push_back(3);
	v.push_back(2);
	v.push_back(1);
	cout<<"myVector size = "<< v.size() << "\n";

	for(int i=0; i < v.size(); ++i)
		cout<<"v[i] : "<< v[i] << "\n";


	myVector<BigClass> mbg;
	mbg.push_back(BigClass(1,2));
	mbg.push_back(BigClass(3,4));
	mbg.push_back(BigClass(5,6));
	mbg.push_back(BigClass(7,8));

	BigClass bg(9,10);
	mbg.push_back(bg);
	cout<<"myVector size = "<< mbg.size() << "\n";

	for(int i=0; i < mbg.size(); ++i)
		cout<<"mbg[i] : "<< mbg[i].x <<" : "<< mbg[i].y << "\n";

	std::vector<int> vbg{4,5,6,7,8,9};
	myVector<int> inl(vbg.begin(), vbg.end());
	for(auto const& el : inl)
		cout<<"element : "<< el << "\n";

}
