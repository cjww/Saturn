#pragma once

#include <algorithm>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "structs.hpp"

namespace sa {

	template<unsigned Comp, typename Type>
	class Vector : public glm::vec<Comp, Type> {
	private:

	public:
		Vector(Type scalar = 0);
		template<typename ...T>
		Vector(Type x, Type y, T... rest);

		// copy from same Vector type
		Vector(const glm::vec<Comp, Type>& other);
		
		// cast from Vector type with other container type
		template<typename T>
		Vector(const glm::vec<Comp, T>& other);

		// cast from Vector type with other component count
		template<unsigned C>
		Vector(const glm::vec<C, Type>& other);

		// cast from Vector type with other container type and other component count
		template<unsigned C, typename T>
		Vector(const glm::vec<C, T>& other);

		Vector(const Point& point);
		Vector(const Extent& extent);


		Vector<Comp, Type>& operator+=(const Vector<Comp, Type>& other);
		Vector<Comp, Type>& operator-=(const Vector<Comp, Type>& other);

		std::string toString() const;

		float length() const;
		float distance(const sa::Vector<Comp, Type>& other) const;


	};

	typedef Vector<2, float> Vector2;
	typedef Vector<2, int> Vector2i;
	typedef Vector<2, unsigned int> Vector2u;

	typedef Vector<3, float> Vector3;
	typedef Vector<3, int> Vector3i;
	typedef Vector<3, unsigned int> Vector3u;

	typedef Vector<4, float> Vector4;
	typedef Vector<4, int> Vector4i;
	typedef Vector<4, unsigned int> Vector4u;

	template<>
	inline Vector<2, float>::Vector(const Point& point)
		: Vector(point.x, point.y)
	{
	}

	template<>
	inline Vector<2, unsigned int>::Vector(const Extent& extent)
		: Vector(extent.width, extent.height)
	{
	}
	


	// ------------- Definitions -------------

	
	template<unsigned Comp, typename Type>
	inline Vector<Comp, Type>::Vector(Type scalar) {
		Type* ptr = (Type*)this;
		for (unsigned i = 0; i < Comp; i++) {
			ptr[i] = scalar;
		}
	}

	template<unsigned Comp, typename Type>
	template<typename ...T>
	inline Vector<Comp, Type>::Vector(Type x, Type y, T... rest) {
		Type* ptr = (Type*)this;
		Type values[Comp] = { x, y, rest... };
		for (unsigned i = 0; i < Comp; i++) {
			ptr[i] = values[i];
		}
	}

	template<unsigned Comp, typename Type>
	inline Vector<Comp, Type>::Vector(const glm::vec<Comp, Type>& other) {
		Type* ptr = (Type*)this;
		Type* otherPtr = (Type*)&other;
		for (unsigned i = 0; i < Comp; i++) {
			ptr[i] = other[i];
		}
	}

	template<unsigned Comp, typename Type>
	template<typename T>
	inline Vector<Comp, Type>::Vector(const glm::vec<Comp, T>& other) {
		Type* ptr = (Type*)this;
		T* otherPtr = (T*)&other;
		for (unsigned i = 0; i < Comp; i++) {
			ptr[i] = static_cast<Type>(other[i]);
		}
	}

	template<unsigned Comp, typename Type>
	template<unsigned C>
	inline Vector<Comp, Type>::Vector(const glm::vec<C, Type>& other) {
		Type* ptr = (Type*)this;
		Type* otherPtr = (Type*)&other;
		memset(ptr, 0, Comp * sizeof(Type));
		for (unsigned i = 0; i < std::min(Comp, C); i++) {
			ptr[i] = other[i];
		}
	}

	template<unsigned Comp, typename Type>
	template<unsigned C, typename T>
	inline Vector<Comp, Type>::Vector(const glm::vec<C, T>& other) {
		Type* ptr = (Type*)this;
		T* otherPtr = (T*)&other;
		memset(ptr, 0, Comp * sizeof(Type));
		for (unsigned i = 0; i < std::min(Comp, C); i++) {
			ptr[i] = static_cast<Type>(other[i]);
		}
	}

	template<unsigned Comp, typename Type>
	inline Vector<Comp, Type>& Vector<Comp, Type>::operator+=(const Vector<Comp, Type>& other) {
		*this = *this + other;
		return *this;
	}

	template<unsigned Comp, typename Type>
	inline Vector<Comp, Type>& Vector<Comp, Type>::operator-=(const Vector<Comp, Type>& other) {
		*this = *this - other;
		return *this;
	}

	template<unsigned Comp, typename Type>
	inline std::string Vector<Comp, Type>::toString() const {
		std::string str = "( ";
		Type* ptr = (Type*)this;
		for (unsigned i = 0; i < Comp; i++) {
			str += std::to_string(ptr[i]);
			if (i < Comp - 1)
				str += ", ";
		}
		str += " )";
		return str;
	}

	template<unsigned Comp, typename Type>
	inline float Vector<Comp, Type>::length() const {
		glm::vec<Comp, float> v = *this;
		return glm::length(v);
	}

	template<unsigned Comp, typename Type>
	inline float Vector<Comp, Type>::distance(const sa::Vector<Comp, Type>& other) const {
		return (other - *this).length();
	}

}

namespace std {
	template<unsigned Comp, typename Type>
	struct hash<sa::Vector<Comp, Type>> {
		size_t operator()(const sa::Vector<Comp, Type>& v) const {
			return std::hash<glm::vec<Comp, Type>>()(v);
		}
	};
}