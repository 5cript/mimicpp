# mimic++, a modern and (mostly) macro free mocking framework

[![codecov](https://codecov.io/gh/DNKpp/mimicpp/graph/badge.svg?token=T9EpgyuyUi)](https://codecov.io/gh/DNKpp/mimicpp)
[![Coverage Status](https://coveralls.io/repos/github/DNKpp/mimicpp/badge.svg)](https://coveralls.io/github/DNKpp/mimicpp)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b852271c6e8742fe8a1667e679dc422b)](https://app.codacy.com/gh/DNKpp/mimicpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/b852271c6e8742fe8a1667e679dc422b)](https://app.codacy.com/gh/DNKpp/mimicpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue)](https://dnkpp.github.io/mimicpp/)

---

## Author

Dominic Koepke  
Mail: [DNKpp2011@gmail.com](mailto:dnkpp2011@gmail.com)

# Table of Contents

* [Introduction](#introduction)
    * [Core Design](#core-design)
    * [Examples](#examples)
    * [Other Choices](#other-choices)
    * [Special Acknowledgement](#special-acknowledgement)
* [Customizability](#customizability)
    * [Stringification](#stringification)
* [Integration](#integration)
    * [Installation](#installation)
        * [CMake](#cmake)
        * [Single-Header](#single-header)
    * [Test Framework](#test-framework)
* [Documentation](#documentation)
* [Testing](#testing)
    * [Windows](#windows)
    * [Linux](#linux)
    * [macOS](#macos)
* [License](#license)
* [Known Issues](#known-issues)

---

# Introduction

``mimic++`` is a c++20 mocking framework, which aims to offer a natural end expressive syntax, without constantly resorting to macros.
To be honest, macros cannot be completely avoided, but they can at least be reduced to a very minimum.

The one thing, that this framework does different than all other (or at least all I am aware of) mocking framework is, that mock objects explicitly are function objects,
thus directly callable. It's simple and straight forward.

As I'm mainly working on template or functional-style code, I wanted something simpler than always creating explicit mock types for the simplest of use cases.
So, ``mimicpp::Mock`` objects can directly be used as functional objects, but they can also be used as member objects and thus serve as actual member functions.

If you are curious, have a look at the [documentation](https://dnkpp.github.io/mimicpp/), investigate the examples folder or play around online at
[godbolt.org](https://godbolt.org/z/dTWEhK15W).

## Core Design

The framework is designed with two core concepts in mind: Mocks and Expectations.
Mocks can be used to define behaviour on a per test-case basis, without the necessity of creating dozens of types. The go-to example is,
if you have a custom type, which somehow makes a connection to a concrete database, you do not want to setup an actual database connection during
your test runs. You then simply install a database mock, which then yields the exact replies as it were defined for that particular case:
the so called "Expectations".

So, Mocks and Expectations are going together hand in hand.

## Examples

```cpp
#include <mimic++/mimic++.hpp>

namespace matches = mimicpp::matches;
using matches::_;
namespace expect = mimicpp::expect;
namespace finally = mimicpp::finally;

TEST_CASE("Mocks are function objects.")
{
    mimicpp::Mock<int(std::string, std::optional<int>)> mock{};     // actually enables just `int operator ()(std::string, float)`
    SCOPED_EXP mock.expect_call("Hello, World", _)                  // requires the first argument to match the string "Hello, World"; the second has no restrictions
				and expect::at_least(1)                             // controls, how often the whole expectation must be matched
				and expect::arg<0>(!matches::range::is_empty())     // addtionally requires the first argument to be not empty (note the preceeding !)
				and expect::arg<1>(matches::ne(std::nullopt))       // requires the second argument to compare unequal to "std::nullopt"
				and expect::arg<1>(matches::lt(1337))               // and to be less than 1337
				and finally::returns(42);                           // And, when matches, returns 42

    int result = mock("Hello, World", 1336);                        // matches
    REQUIRES(42 == result);
}

TEST_CASE("Mocks can be overloaded.")
{
    mimicpp::Mock<
        int(std::string, std::optional<int>),                   // same as previous test
        void() const                                            // enables `void operator ()() const` (note the const specification)
    > mock{};

    SCOPED_EXP mock.expect_call()                               // setup an expectation for the void() overload
                and expect::twice();                            // should be matched twice

    mock();                                                     // first match

    // you can always create new expectations as you need them, even if the object is already in use
    SCOPED_EXP mock.expect_call(!matches::range::is_empty(), 42)        // you can always apply matches directly; if just a value is provided, defaults to matches::eq
                and expect::once()                                      // once() is the default, but you can state that explicitly
                and finally::throws(std::runtime_error{"some error"});  // when matches, throws an exception

    REQUIRE_THROWS(mock("Test", 42));						// ok, matched

	// still a pending expectation for void() overload
	std::as_const(mock)();                                          // explicitly call from a const object
}
```

## Other Choices

### Always Stay Within The Language Definition

There are a lot of mocking frameworks, which utilize clever tricks and apply some compiler specific instructions to make the work more enjoyable.
``mimic++`` does not!
This framework will never touch the ground of undefined behaviour or tricks, which will only work under some circumstances, as this is nothing I
want to support and maintain over a set of compilers or configurations.
Unfortunatle this often leads to a less elegant syntax for users. If you need that, than this framework is probably not the right for you.
Pick your poison :)

## Special Acknowledgement

This framework is highly inspired by the well known [trompeloeil](https://github.com/rollbear/trompeloeil), which I have used myself for several years now.
It's definitly not bad, but sometimes feels a little bit dated and some macros do not play very well with formatting tools and the like.
If you need a pre-c++20 mocking-framework, you should definitly give it a try.

Fun fact: ``mimic++`` uses ``trompeloeil`` for it's own test suite :D

# Customizability

A framework should be a useful tool that can be used in a variety of ways. However, it should not be a foundation that limits the house to be built on it.
For this reason ``mimic++`` offers various ways for customization: E.g. users may create their own expectation policies and integrate them seamlessly, without
changing any line of the ``mimic++`` code-base.

## Stringification

``mimic++`` can not provide stringification for any type out there, but it's often very useful to see a proper textual reprensentation of an object, when a test fails.
``mimic++`` will use ``std::format`` for ``formattable`` types, but sometimes that is not, what we want, as users for example want to have an alternative
stringification just for testing purposes.
Users can therefore add their specializations of the ``mimicpp::custom::Printer`` type and thus tell ``mimic++`` how a given type shall be printed.

Custom specializations will always be prefered over any pre-existing printing methods, thus users may even override the stringification of the internal report types.

## Documentation

The documenation is generated via ``doxygen``. Users can do this locally by enabling both, the ``MIMICPP_CONFIGURE_DOXYGEN`` and ``MIMICPP_ENABLE_GENERATE_DOCS``,
cmake options and building the target ``mimicpp-generate-docs`` manually.

The documentation for the ``main`` branch is always available on the [github-pages](https://dnkpp.github.io/mimicpp/); for the ``development`` branch it is also available on the ``dev-gh-pages`` branch,
but unfortunatly not directly viewable on the browser.
Every release has the generated documentation attached.

# Integration

``mimic++`` is a head-only library. Users can easily enjoy all features by simply including the ``mimic++/mimic++.hpp`` header. Of course one can be more granular
and include just what's necessary. The choice is yours.

## Installation

### CMake

This framework is header-only and completely powered by cmake, thus the integration into a cmake project is straight-forward.
```cmake
target_link_libraries(
    <your_target_name>
    PUBLIC
    mimicpp::mimicpp
)
```

Users can either pick a commit in the ``main`` branch or a version tag and utilize the cmake ``FetchContent`` module:
```cmake
include(FetchContent)

FetchContent_Declare(
    mimicpp
    GIT_REPOSITORY https://github.com/DNKpp/mimicpp
    GIT_TAG        <any_commit_hash_or_tag>
)

FetchContent_MakeAvailable(mimicpp)
# do not forget linking via target_link_libraries as shown above
```

As an alternative, I recommend the usage of [CPM](https://github.com/cpm-cmake/CPM.cmake), which is a featureful wrapper around the ``FetchContent``
functionality:
```cmake
include(CPM.cmake) # or include(get_cpm.cmake)

CPMAddPackage("gh:DNKpp/mimicpp#<any_commit_hash_or_tag>")
# do not forget linking via target_link_libraries as shown above
```

### Single-Header

As an alternative each release has a header file named ``mimic++-amalgamated.hpp`` attached, which contains all
definitions (except for the specific test-framework adapters) and can be simply dropped into any c++20 project (or used on [godbolt.org](https://godbolt.org).
After that, users may also just pick the appropriate adapter header for their desired test-framework and put that into their project aswell.

## Test Framework

Mocking frameworks usually do not exist for their own, as they are in fact just an advanced technice for creating tests. Instead, they should work
together with any existing test framework out there. ``mimic++`` provides the ``IReporter`` interface, which in fact serves as a bridge from ``mimic++``
into the utilized test framework. ``mimic++`` provides some concrete reporter implementations for well known test frameworks, but users may create custom adapters for
any test framework or simply use the default reporter.
For more details have a look into the ``reporting`` section in the documentation.

Official adapters exist for the following frameworks:

* [Boost.Test](https://github.com/boostorg/test)
* [Catch2](https://github.com/catchorg)
* [GTest](https://github.com/google/googletest)

# Testing

``mimic++`` utilizes a strict testing policy, thus each official feature is well tested. The effect of those test-cases are always tracked by the extensive ci,
which checks the compilation success, test cases outcomes and coverage on dozens of different os, compiler and build configurations.

The coverage is generated via ``gcov`` and evaluated by
[codacy](https://app.codacy.com/gh/DNKpp/mimicpp),
[codecov](https://codecov.io/gh/DNKpp/mimicpp) and
[coveralls](https://coveralls.io/github/DNKpp/mimicpp).

## Windows

| OS           | Compiler | c++-20 | c++-23 |
|--------------|----------|:------:|:------:|
| Windows 2022 | msvc     |    x   |    x   |
| Windows 2022 | clangCl  |    x   |    x   |

## Linux

| Compiler | libstdc++ | libc++ | c++-20 | c++-23 |
|----------|:---------:|:------:|:------:|:------:|
| clang-17 |     x     |    x   |    x   |    x   |
| clang-18 |     x     |    x   |    x   |    x   |
| gcc-13   |     x     |    -   |    x   |    x   |
| gcc-14   |     x     |    -   |    x   |    x   |

## macOS

| Compiler          | libstdc++ | libc++ | c++-20 | c++-23 |
|-------------------|:---------:|:------:|:------:|:------:|
| AppleClang-17.0.6 |     -     |    x   |    x   |    x   |
| AppleClang-18.1.6 |     -     |    x   |    x   |    x   |

As new compilers become available, they will be added to the workflow, but older compilers will probably never be supported.

# License

[BSL-1.0](LICENSE_1_0.txt) (free, open source)

```text
          Copyright Dominic "DNKpp" Koepke 2024 - 2024
 Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at
          https://www.boost.org/LICENSE_1_0.txt)
```

---

# Known Issues

### Clang-18.1 + libc++
Date: 25.09.2024

This combination introduced a regression regarding the ``std::invocable`` concept and a default parameter of type ``std::source_location``.
On this version, all invocable checks will fail, but the ``std::is_invocable`` trait still works as expected. Unfortunatly this can not solved easily by this lib, sorry for that.

Clang-17 and Clang-19 do not suffer from this issue.
For more information have a look at: https://github.com/llvm/llvm-project/issues/106428
