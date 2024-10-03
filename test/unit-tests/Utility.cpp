// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Utility.hpp"
#include "mimic++/Printer.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdint>

using namespace mimicpp;

TEMPLATE_TEST_CASE(
	"to_underlying converts the enum value to its underlying representation.",
	"[utility]",
	std::int8_t,
	std::uint8_t,
	std::int16_t,
	std::uint16_t,
	std::int32_t,
	std::uint32_t,
	std::int64_t,
	std::uint64_t
)
{
	using UnderlyingT = TestType;

	SECTION("When an enum value is given.")
	{
		enum Test
			: UnderlyingT
		{
		};

		const UnderlyingT value = GENERATE(
			std::numeric_limits<UnderlyingT>::min(),
			0,
			1,
			std::numeric_limits<UnderlyingT>::max());

		STATIC_REQUIRE(std::same_as<UnderlyingT, decltype(to_underlying(Test{value}))>);
		REQUIRE(value == to_underlying(Test{value}));
	}

	SECTION("When an class enum value is given.")
	{
		enum class Test
			: UnderlyingT
		{
		};

		const UnderlyingT value = GENERATE(
			std::numeric_limits<UnderlyingT>::min(),
			0,
			1,
			std::numeric_limits<UnderlyingT>::max());

		STATIC_REQUIRE(std::same_as<UnderlyingT, decltype(to_underlying(Test{value}))>);
		REQUIRE(value == to_underlying(Test{value}));
	}
}

TEMPLATE_TEST_CASE_SIG(
	"same_as_any determines, whether T is the same as any other given type.",
	"[utility]",
	((bool expected, typename T, typename... Others), expected, T, Others...),
	(false, int),
	(false, int, int&),
	(false, int, int&, int&),
	(false, int, int&, double, float),
	(true, int, int),
	(true, int, int&, int),
	(true, int, double, int, int&)
)
{
	STATIC_REQUIRE(expected == same_as_any<T, Others...>);
}

TEMPLATE_TEST_CASE_SIG(
	"unique_list_t is an alias to a tuple with just unique types.",
	"[utility]",
	((bool dummy, typename Expected, typename... Types), dummy, Expected, Types...),
	(true, std::tuple<>),
	(true, std::tuple<int, int&>, int, int&),
	(true, std::tuple<int, int&>, int, int&, int&),
	(true, std::tuple<int, int&, double, float>, int, int&, double, float),
	(true, std::tuple<int>, int, int),
	(true, std::tuple<int, int&>, int, int&, int),
	(true, std::tuple<int, double, int&>, int, double, int, int&)
)
{
	STATIC_REQUIRE(std::same_as<Expected, detail::unique_list_t<Types...>>);
}

TEMPLATE_TEST_CASE_SIG(
	"string concept determines, whether the given type can be used as a string-type.",
	"[utility]",
	((bool expected, typename T), expected, T),
	(false, char),

	(true, char*),
	(true, const char*),
	(true, wchar_t*),
	(true, const wchar_t*),
	(true, char8_t*),
	(true, const char8_t*),
	(true, char16_t*),
	(true, const char16_t*),
	(true, char32_t*),
	(true, const char32_t*),

	(true, char[]),
	(true, const char[]),
	(true, char(&)[]),
	(true, const char(&)[]),
	(true, char[42]),
	(true, const char[42]),
	(true, char(&)[42]),
	(true, const char(&)[42]),

	(true, std::string),
	(true, const std::string),
	(true, std::string&),
	(true, const std::string&),
	(true, std::string&&),
	(true, const std::string&&),

	(true, std::wstring),
	(true, std::u8string),
	(true, std::u16string),
	(true, std::u32string),

	(true, std::string_view),
	(true, std::wstring_view),
	(true, std::u8string_view),
	(true, std::u16string_view),
	(true, std::u32string_view)
)
{
	STATIC_REQUIRE(expected == mimicpp::string<T>);
}

// let's just specialize for std::string. Other char-strings must choose the internal converter.
template <>
struct custom::to_lower_converter<std::string>
{
	[[nodiscard]]
	std::string operator()([[maybe_unused]] const std::string& str) const
	{
		return "custom::to_lower_converter<std::string>";
	}
};

TEST_CASE(
	"detail::to_lower_hook::to_lower selects the correct converter.",
	"[utility][detail]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("custom::to_lower_converter specializations are preferred.")
	{
		const std::string result = detail::to_lower_hook::to_lower(
			std::string{"Hello, World!"});

		REQUIRE_THAT(
			result,
			Matches::Equals("custom::to_lower_converter<std::string>"));
	}

	SECTION("Otherwise, to_lower_hook::to_lower_converter is chosen.")
	{
		const std::string result = detail::to_lower_hook::to_lower(
			std::string_view{"Hello, World!"});

		REQUIRE_THAT(
			result,
			Matches::Equals("hello, world!"));
	}
}

TEMPLATE_TEST_CASE(
	"detail::to_lower_hook::to_lower converts the given string to its lower-case representation.",
	"[utility][detail]",
	const char*,
	//std::string, We can not test it here, as we specialized the custom hook for testing purposes
	std::string_view
)
{
	namespace Matches = Catch::Matchers;

	const auto [expected, source] = GENERATE(
		(table<std::string, const char*>)({
			{"", ""},
			{" !1337\t", " !1337\t"},
			{"hello, world!", "HeLlO, WoRlD!"},
			}));

	const std::string result = detail::to_lower_hook::to_lower(
			static_cast<TestType>(source));

	REQUIRE_THAT(
		result,
		Matches::Equals(expected));
}
