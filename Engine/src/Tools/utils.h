#pragma once

namespace sa {
	namespace utils {
		char toLower(char c);
		char toUpper(char c);

		std::string toUpper(const std::string& str);
		std::string toLower(const std::string& str);

		std::string trim(const std::string& str);

		std::vector<std::string> split(const std::string& str, char seperator, bool trimWhiteSpace = true);

		void stripTypeName(std::string& str);

		namespace details {

			template<typename, typename T>
			struct has_signature {
				static_assert(
					std::integral_constant<T, false>::value,
					"Second template parameter needs to be function type");
			};

			template<typename C, typename Ret, typename ...Args>
			struct has_signature<C, Ret(Args...)> {
			private:
				template<typename T>
				static constexpr auto check(Ret(T::* func)(Args...)) ->
					typename
						std::is_same<
							decltype( ((&std::declval<T>())->*func)(std::declval<Args>() ...)),
							Ret
						>::type;
				
				template<typename>
				static constexpr std::false_type check(...);


				typedef decltype(check<C>(0)) type;

			public:
				static constexpr bool value = type::value;
			};

		}
	}

}