#pragma once
#include <glm\glm.hpp>
namespace sa {
	template<unsigned Col, unsigned Row, typename Type>
	class Matrix : public glm::mat<Col, Row, Type> {
	private:

	public:
		// Defaults to identity matrix
		Matrix(Type scalar = 1);
		// copy from same Matrix type
		Matrix(const glm::mat<Col, Row, Type>& other);

		// cast from Matrix type with other container type
		template<typename T>
		Matrix(const glm::mat<Col, Row, T>& other);
		
		// cast from Matrix type with other Column and Row count
		template<unsigned C, unsigned R>
		Matrix(const glm::mat<C, R, Type>& other);

	};

	typedef Matrix<4, 4, float> Matrix4x4;
	typedef Matrix<4, 3, float> Matrix4x3;
	typedef Matrix<4, 2, float> Matrix4x2;

	typedef Matrix<3, 4, float> Matrix3x4;
	typedef Matrix<3, 3, float> Matrix3x3;
	typedef Matrix<3, 2, float> Matrix3x2;

	typedef Matrix<2, 4, float> Matrix2x4;
	typedef Matrix<2, 3, float> Matrix2x3;
	typedef Matrix<2, 2, float> Matrix2x2;

	template<unsigned Col, unsigned Row, typename Type>
	inline Matrix<Col, Row, Type>::Matrix(Type scalar) {
		glm::mat<Col, Row, Type> m(scalar);
		*this = m;
	}

	template<unsigned Col, unsigned Row, typename Type>
	inline Matrix<Col, Row, Type>::Matrix(const glm::mat<Col, Row, Type>& other) {
		memcpy(this, &other, Col * Row * sizeof(Type));
	}
	

	template<unsigned Col, unsigned Row, typename Type>
	template<typename T>
	inline Matrix<Col, Row, Type>::Matrix(const glm::mat<Col, Row, T>& other) {
		for (unsigned c = 0; c < Col; c++) {
			for (unsigned r = 0; r < Row; r++) {
				(*this)[c][r] = static_cast<Type>(other[c][r]);
			}
		}
	}

	template<unsigned Col, unsigned Row, typename Type>
	template<unsigned C, unsigned R>
	inline Matrix<Col, Row, Type>::Matrix(const glm::mat<C, R, Type>& other) {
		*this = Matrix<Col, Row, Type>(0);
		for (unsigned c = 0; c < std::min(Col, C); c++) {
			for (unsigned r = 0; r < std::min(Row, R); r++) {
				(*this)[c][r] = static_cast<Type>(other[c][r]);
			}
		}
	}


}