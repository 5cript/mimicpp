// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_ADAPTERS_BOOST_TEST_HPP
#define MIMICPP_ADAPTERS_BOOST_TEST_HPP

#pragma once

#include "mimic++/Reporter.hpp"

#if __has_include(<boost/test/unit_test.hpp>)
#include <boost/test/unit_test.hpp>
#else
	#error "Unable to find Boost.Test includes."
#endif

namespace mimicpp::detail::boost_test
{
	struct failure
	{
	};

	[[noreturn]]
	inline void send_fail(const StringViewT msg)
	{
		BOOST_TEST_FAIL(msg);
		unreachable();
	}

	inline void send_success(const StringViewT msg)
	{
		BOOST_TEST_MESSAGE(msg);
	}

	inline void send_warning(const StringViewT msg)
	{
		BOOST_TEST_MESSAGE(
			format::format(
				"warning: {}",
				msg));
	}
}

namespace mimicpp
{
	/**
	 * \brief Reporter for the integration into ``Boost.Test``.
	 * \ingroup REPORTING_ADAPTERS
	 * \details This reporter enables the integration of ``mimic++`` into ``Boost.Test`` and prefixes the headers
	 * of ``Boost.Test`` with ``boost/test``.
	 *
	 * This reporter installs itself by simply including this header file into any source file of the test executable.
	 */
	using BoostTestReporterT = BasicReporter<
		&detail::boost_test::send_success,
		&detail::boost_test::send_warning,
		&detail::boost_test::send_fail
	>;
}

namespace mimicpp::detail::boost_test
{
	inline const ReporterInstaller<BoostTestReporterT> installer{};
}

#endif
