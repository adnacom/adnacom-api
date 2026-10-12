/*!
 * \file `expected.hxx`
 * `std::expected` compatibility wrapper for Adnacom API for use with pre-C++23 compilers.
 * Defines `Expected` and `Unexpected` types which map to `std::expected` and `std::unexpected`
 * respectively, if available;
 * otherwise, provides a simple implementation based on `std::optional` as a replacement.
 * \date 2026.10.10
 */
#pragma once


#if __cplusplus >= 202302L // Is C++23 supported?
#include <expected>

namespace Adnacom::Api {

template <typename Ty, typename Err>
using Expected = std::expected<Ty, Err>;
template <typename Err>
using Unexpected = std::unexpected<Err>;
} // Adnacom::Api namespace


#else // C++23 not available; hack together a simple replacement for `std::expected`.
#include <optional>

namespace Adnacom::Api {

template <typename Err>
class Unexpected
{
public:
	using value_type = Err;

	explicit Unexpected(Err hr) : myerr_{ hr } {}

	Err& value() { return myerr_; }
	const Err& value() const { return myerr_; }

protected:
	Err myerr_;
};

template <typename ErrTy>
Unexpected(ErrTy) -> Unexpected<ErrTy>;

template <typename Ty, typename Err>
class Expected
{
public:
	using value_type = Ty;
	using unexpected_type = Unexpected<Err>;

	Expected(const Ty& val) : myval_{ val } {}
	Expected(Ty&& val) : myval_{ std::move(val) } {}

	Expected(const unexpected_type& unex) : myerr_{ unex.value() } {}

	constexpr bool has_value() const noexcept { return myval_.has_value(); }
	constexpr explicit operator bool() const noexcept { return has_value(); }

	constexpr const Ty& value() const& { return myval_.value(); }
	constexpr const Ty&& value() const&& { return myval_.value(); }
	constexpr Ty& value()& { return myval_.value(); }
	constexpr Ty&& value()&& { return myval_.value(); }

	constexpr const Ty* operator->() const { return &myval_.value(); }
	constexpr Ty* operator->() { return &myval_.value(); }

	constexpr const auto& operator*() const& { return *myval_; }
	constexpr auto&& operator*()&& { return *myval_; }

	constexpr const auto& error() const { return myerr_; }
	constexpr auto& error() { return myerr_; }

	template <typename Tz = std::remove_cv_t<Ty>>
	constexpr auto value_or(Tz&& fallbackValue) const& { return myval_.value_or(std::forward<Tz>(fallbackValue)); }
	template <typename Tz = std::remove_cv_t<Ty>>
	constexpr auto value_or(Tz&& fallbackValue)&& { return myval_.value_or(std::forward<Tz>(fallbackValue)); }


protected:
	std::optional<Ty> myval_;
	Err myerr_ {};
};

} // Adnacom::Api namespace


#endif // no C++23
