#include <cereal/cereal.hpp>
#include <variant>


namespace inl::game {

template <class T>
struct IsNameValuePair : std::false_type {};

template <class U>
struct IsNameValuePair<cereal::NameValuePair<U>> : std::true_type {};

template <class T>
struct HandleArchive : std::conditional_t<std::is_arithmetic_v<T> || IsNameValuePair<T>::value, std::true_type, std::false_type> {};



template <class... ArchiveOptions>
class VariantInputArchive : public std::variant<ArchiveOptions...>, public cereal::InputArchive<VariantInputArchive<ArchiveOptions...>> {
public:
	constexpr VariantInputArchive() noexcept : cereal::InputArchive<VariantInputArchive<ArchiveOptions...>>(this) {}
	constexpr VariantInputArchive(const VariantInputArchive& other) = delete;
	constexpr VariantInputArchive(VariantInputArchive&& other) noexcept = delete;

	template <class T>
	constexpr VariantInputArchive(T&& t) noexcept
		: std::variant<ArchiveOptions...>(std::forward<T>(t)), cereal::InputArchive<VariantInputArchive<ArchiveOptions...>>(this) {}

	template <class T, class... Args>
	constexpr explicit VariantInputArchive(std::in_place_type_t<T>, Args&&... args)
		: std::variant<ArchiveOptions...>(std::in_place_type<T>, std::forward<Args>(args)...), cereal::InputArchive<VariantInputArchive<ArchiveOptions...>>(this) {}

	template <class T, class U, class... Args>
	constexpr explicit VariantInputArchive(std::in_place_type_t<T>, std::initializer_list<U> il, Args&&... args)
		: std::variant<ArchiveOptions...>(std::in_place_type<T>, il, std::forward<Args>(args)...), cereal::InputArchive<VariantInputArchive<ArchiveOptions...>>(this) {}

	template <std::size_t I, class... Args>
	constexpr explicit VariantInputArchive(std::in_place_index_t<I>, Args&&... args)
		: std::variant<ArchiveOptions...>(std::in_place_index<I>, std::forward<Args>(args)...), cereal::InputArchive<VariantInputArchive<ArchiveOptions...>>(this) {}

	template <std::size_t I, class U, class... Args>
	constexpr explicit VariantInputArchive(std::in_place_index_t<I>, std::initializer_list<U> il, Args&&... args)
		: std::variant<ArchiveOptions...>(std::in_place_index<I>, il, std::forward<Args>(args)...), cereal::InputArchive<VariantInputArchive<ArchiveOptions...>>(this) {}
};


template <class T, class... ArchiveOptions>
void prologue(VariantInputArchive<ArchiveOptions...>& ar, const T& arg) {
	std::visit([&arg](auto&& ar) { prologue(ar, arg); }, ar);
}

template <class T, class... ArchiveOptions>
void epilogue(VariantInputArchive<ArchiveOptions...>& ar, const T& arg) {
	std::visit([&arg](auto&& ar) { epilogue(ar, arg); }, ar);
}

template <class T, class... ArchiveOptions>
std::enable_if_t<HandleArchive<T>::value, void> CEREAL_SAVE_FUNCTION_NAME(VariantInputArchive<ArchiveOptions...>& ar, const T& arg) {
	std::visit([&arg](auto&& ar) { CEREAL_SAVE_FUNCTION_NAME(ar, arg); }, ar);
}

template <class T, class... ArchiveOptions>
std::enable_if_t<HandleArchive<T>::value, void> CEREAL_LOAD_FUNCTION_NAME(VariantInputArchive<ArchiveOptions...>& ar, T& arg) {
	std::visit([&arg](auto&& ar) { CEREAL_LOAD_FUNCTION_NAME(ar, arg); }, ar);
}



template <class... ArchiveOptions>
class VariantOutputArchive : public std::variant<ArchiveOptions...>, public cereal::OutputArchive<VariantOutputArchive<ArchiveOptions...>> {
public:
	constexpr VariantOutputArchive() noexcept : cereal::OutputArchive<VariantOutputArchive<ArchiveOptions...>>(this) {}
	constexpr VariantOutputArchive(const VariantOutputArchive& other) = delete;
	constexpr VariantOutputArchive(VariantOutputArchive&& other) noexcept = delete;

	template <class T>
	constexpr VariantOutputArchive(T&& t) noexcept
		: std::variant<ArchiveOptions...>(std::forward<T>(t)), cereal::OutputArchive<VariantOutputArchive<ArchiveOptions...>>(this) {}

	template <class T, class... Args>
	constexpr explicit VariantOutputArchive(std::in_place_type_t<T>, Args&&... args)
		: std::variant<ArchiveOptions...>(std::in_place_type<T>, std::forward<Args>(args)...), cereal::OutputArchive<VariantOutputArchive<ArchiveOptions...>>(this) {}

	template <class T, class U, class... Args>
	constexpr explicit VariantOutputArchive(std::in_place_type_t<T>, std::initializer_list<U> il, Args&&... args)
		: std::variant<ArchiveOptions...>(std::in_place_type<T>, il, std::forward<Args>(args)...), cereal::OutputArchive<VariantOutputArchive<ArchiveOptions...>>(this) {}

	template <std::size_t I, class... Args>
	constexpr explicit VariantOutputArchive(std::in_place_index_t<I>, Args&&... args)
		: std::variant<ArchiveOptions...>(std::in_place_index<I>, std::forward<Args>(args)...), cereal::OutputArchive<VariantOutputArchive<ArchiveOptions...>>(this) {}

	template <std::size_t I, class U, class... Args>
	constexpr explicit VariantOutputArchive(std::in_place_index_t<I>, std::initializer_list<U> il, Args&&... args)
		: std::variant<ArchiveOptions...>(std::in_place_index<I>, il, std::forward<Args>(args)...), cereal::OutputArchive<VariantOutputArchive<ArchiveOptions...>>(this) {}
};

template <class T, class... ArchiveOptions>
void prologue(VariantOutputArchive<ArchiveOptions...>& ar, const T& arg) {
	std::visit([&arg](auto&& ar) { prologue(ar, arg); }, ar);
}

template <class T, class... ArchiveOptions>
void epilogue(VariantOutputArchive<ArchiveOptions...>& ar, const T& arg) {
	std::visit([&arg](auto&& ar) { epilogue(ar, arg); }, ar);
}

template <class T, class... ArchiveOptions>
std::enable_if_t<HandleArchive<T>::value, void> CEREAL_SAVE_FUNCTION_NAME(VariantOutputArchive<ArchiveOptions...>& ar, const T& arg) {
	std::visit([&arg](auto&& ar) { CEREAL_SAVE_FUNCTION_NAME(ar, arg); }, ar);
}

template <class T, class... ArchiveOptions>
std::enable_if_t<HandleArchive<T>::value, void> CEREAL_LOAD_FUNCTION_NAME(VariantOutputArchive<ArchiveOptions...>& ar, T& arg) {
	std::visit([&arg](auto&& ar) { CEREAL_LOAD_FUNCTION_NAME(ar, arg); }, ar);
}


} // namespace inl::game