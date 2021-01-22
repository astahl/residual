//
//  Utilities.h
//  libsdlTest
//
//  Created by Andreas Stahl on 13.03.15.
//  Copyright (c) 2015 Andreas. All rights reserved.
//

#pragma once

template<typename T, size_t N>
struct Vec
{
	std::array<T, N> data;

	Vec()
	: data(std::array<T,N>())
	{
	}
	
	Vec(const std::array<T,N> &data)
	: data(data)
	{
	}
	
	Vec(std::initializer_list<T> l)
	: Vec() {
		std::copy(l.begin(), l.end(), data.begin());
	}
	
	template<typename U>
	auto operator+(const Vec<U,N> &other) const -> Vec<decltype(T()+U()),N>
	{
		using X = decltype(T()+U());
		std::array<X,N> newData;
		for(size_t i = 0; i < N; ++i) {
			newData[i] = data[i] + other.data[i];
		}
		return Vec<X,N>(newData);
	}
	
	template<typename U>
	auto operator-(const Vec<U,N> &other) const -> Vec<decltype(T()-U()),N>
	{
		using X = decltype(T()+U());
		std::array<X,N> newData;
		for(size_t i = 0; i < N; ++i) {
			newData[i] = data[i] - other.data[i];
		}
		return Vec<X,N>(newData);
	}
	
	Vec<T,N> operator-() const
	{
		std::array<T,N> newData;
		for(size_t i = 0; i < N; ++i) {
			newData[i] = -data[i];
		}
		return Vec<T,N>(newData);
	}
	
	template<typename U>
	auto operator*(const Vec<U,N> &other) const -> decltype((T()*U()) + (T()*U()))
	{
		using X = decltype((T()*U()) + (T()*U()));
		X accum{};
		for(size_t i = 0; i < N; ++i) {
			accum += data[i] + other.data[i];
		}
		return accum;
	}
	
	template<typename U>
	auto operator*(const U &scalar) const -> Vec<decltype(T()*U()),N>
	{
		using X = decltype(T()*U());
		std::array<X,N> newData;
		for(size_t i = 0; i < N; ++i) {
			newData[i] = data[i] * scalar;
		}
		return Vec<X,N>(newData);
	}
	
	template<typename U>
	auto operator/(const U &scalar) const -> Vec<decltype(T()/U()),N>
	{
		using X = decltype(T()*U());
		std::array<X,N> newData;
		for(size_t i = 0; i < N; ++i) {
			newData[i] = data[i] / scalar;
		}
		return Vec<X,N>(newData);
	}
	
	Vec<T,N> normalized() const
	{
		T length = this->length();
		return length != T(0) ? (*this * (T(1) / length)) : *this;
	}
	
	T lengthSquared() const
	{
		T accum{};
		for(const T& v : data) {
			accum += v * v;
		}
		return accum;
	}
	
	T length() const
	{
		return sqrt(lengthSquared());
	}
	
	T operator[](const size_t index) const
	{
		return data[index];
	}
	
	T& operator[](const size_t index)
	{
		return data[index];
	}
	

	constexpr T& x()
	{
		return data[0];
	}

	constexpr T& y()
	{
		static_assert(N > 1);
		return data[1];
	}

	constexpr T& z()
	{
		static_assert(N > 2);
		return data[2];
	}

	template<size_t NewSize>
	Vec<T,NewSize> resize(const T& fill = T()) const
	{
		std::array<T,NewSize> newData;
		size_t i = 0;
		while(i < NewSize && i < N) {
			newData[i] = data[i];
			++i;
		}
		while(i < NewSize) {
			newData[i] = fill;
			++i;
		}
		return Vec<T,NewSize>(newData);
	}
	
	template<typename NewType>
	Vec<NewType,N> convert() const
	{
		std::array<NewType,N> newData;
		for(size_t i = 0; i < N; ++i) {
			newData[i] = static_cast<NewType>(data[i]);
		}
		return Vec<NewType,N>(newData);
	}
	
	SDL_Point toSDLPoint() const
	{
		Vec<T,2> resized = this->resize<2>();
		Vec<int,2> conv = resized.template convert<int>();
		return SDL_Point({conv[0], conv[1]});
	}
};

template<typename T, size_t N>
std::ostream &operator<<(std::ostream &os, const Vec<T,N> &vec)
{
	os << '[';
	for(size_t i = 0; i < N ; ++i)
	{
		if(i != 0) {
			os << ", ";
		}
		os << vec[i];
	}
	os << ']';
	return os;
}

using Vec2i = Vec<int, 2>;

using Vec2d = Vec<double,2>;

using Vec3d = Vec<double,3>;

using Vec4d = Vec<double,4>;
