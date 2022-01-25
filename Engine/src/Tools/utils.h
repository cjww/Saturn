#pragma once
#include <type_traits>
namespace sa {
	namespace utils {
		template<typename Type>
		struct promote_integral_type {

			static double test(float) { return 0.0; }
			
			template<typename Test>
			static auto test(Test) -> decltype(+std::declval<Test>()) { return +std::declval<Test>(); };

			//typedef decltype(test(std::declval<Type>())) type;
		};

		// nullptr_t promotes to void *
		template <typename T, typename = typename std::enable_if<std::is_same<T, std::nullptr_t>::value>::type>
		void* default_promote_impl(T);

		// float promotes to double
		template <typename T, typename = typename std::enable_if<std::is_same<T, float>::value>::type>
		double default_promote_impl(T);

		// scalar types other than nullptr_t/float that have a conversion from/to 0 promote to their common type
		// this also matches function and array types, after their implicit conversion to a pointer type
		template <typename T, typename = typename std::enable_if<std::is_scalar<T>::value && !std::is_same<T, std::nullptr_t>::value && !std::is_same<T, float>::value>::type>
		decltype(true ? 0 : std::declval<T>()) default_promote_impl(T);

		// scoped enumeration types don't get promoted
		template <typename T, typename = typename std::enable_if<std::is_enum<T>::value>::type>
		typename std::enable_if<!std::is_convertible<T, typename std::underlying_type<T>::type>::value, T>::type default_promote_impl(T);

		// class types don't get promoted
		template <typename T, typename = typename std::enable_if<std::is_class<T>::value || std::is_union<T>::value>::type>
		T default_promote_impl(T);

		template <typename T>
		constexpr bool check_vararg_passable(...) {
			return true ? true : check_vararg_passable<T>(*(typename std::remove_reference<T>::type*)0);
		}

		template <typename T, bool = check_vararg_passable<T>()>
		struct default_promote {
			typedef decltype(default_promote_impl(std::declval<T>())) type;
		};
	}

}