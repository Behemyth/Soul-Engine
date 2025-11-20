export module synodic.soul.core:structures;

import std;

// Note: std::span is already provided by the standard library (C++20)
// We just re-export it for convenience
export using std::span;

// RingBuffer: Fixed-size circular buffer
export template <typename T, std::size_t N>
class RingBuffer {
public:
    RingBuffer() : head_(0), tail_(0), size_(0) {}

    // Add element to the buffer (overwrites oldest if full)
    void Push(const T& value) {
        buffer_[head_] = value;
        head_ = (head_ + 1) % N;
        if (size_ < N) {
            ++size_;
        } else {
            tail_ = (tail_ + 1) % N;
        }
    }

    void Push(T&& value) {
        buffer_[head_] = std::move(value);
        head_ = (head_ + 1) % N;
        if (size_ < N) {
            ++size_;
        } else {
            tail_ = (tail_ + 1) % N;
        }
    }

    // Access elements (0 is newest, N-1 is oldest)
    T& operator[](std::size_t index) noexcept {
        std::size_t actual_index = (head_ - 1 - index + N) % N;
        return buffer_[actual_index];
    }

    const T& operator[](std::size_t index) const noexcept {
        std::size_t actual_index = (head_ - 1 - index + N) % N;
        return buffer_[actual_index];
    }

    // Size queries
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return N; }
    bool empty() const noexcept { return size_ == 0; }
    bool full() const noexcept { return size_ == N; }

    // Clear the buffer
    void clear() noexcept {
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

private:
    std::array<T, N> buffer_;
    std::size_t head_;
    std::size_t tail_;
    std::size_t size_;
};

// TypeTraits: Additional type trait utilities
export namespace TypeTraits {

// Check if a type is any of the given types
template <typename T, typename... Types>
struct is_any_of : std::bool_constant<(std::is_same_v<T, Types> || ...)> {};

template <typename T, typename... Types>
inline constexpr bool is_any_of_v = is_any_of<T, Types...>::value;

// Function traits
template <typename T>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<R(Args...)> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
};

template <typename R, typename... Args>
struct function_traits<R(*)(Args...)> : function_traits<R(Args...)> {};

template <typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...)> : function_traits<R(Args...)> {
    using class_type = C;
};

template <typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...) const> : function_traits<R(Args...)> {
    using class_type = C;
};

} // namespace TypeTraits
