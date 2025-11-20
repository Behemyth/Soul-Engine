export module synodic.soul.core:utility;

import std;

// ClassID: Static class identification system using CRTP pattern
// Provides unique incrementing IDs for each class type
export template <typename T>
class ClassID {
public:
    static std::size_t ID() noexcept {
        static const std::size_t id = counter_++;
        return id;
    }

private:
    inline static std::size_t counter_ = 0;
};

// Exception base class and common exceptions
export class Exception : public std::exception {
public:
    explicit Exception(const char* message) : message_(message) {}
    explicit Exception(const std::string& message) : message_(message) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

export class NotImplemented : public Exception {
public:
    NotImplemented() : Exception("Functionality not yet implemented") {}
    explicit NotImplemented(const char* message) : Exception(message) {}
    explicit NotImplemented(const std::string& message) : Exception(message) {}
};

export class InvalidArgument : public Exception {
public:
    explicit InvalidArgument(const char* message) : Exception(message) {}
    explicit InvalidArgument(const std::string& message) : Exception(message) {}
};

export class RuntimeError : public Exception {
public:
    explicit RuntimeError(const char* message) : Exception(message) {}
    explicit RuntimeError(const std::string& message) : Exception(message) {}
};

// HashString: Compile-time and runtime string hashing
export class HashString {
public:
    using HashType = std::size_t;

    // Runtime hash
    explicit HashString(const char* str) : hash_(Hash(str)) {}
    explicit HashString(const std::string& str) : hash_(Hash(str.c_str())) {}

    // Get hash value
    HashType Value() const noexcept { return hash_; }
    operator HashType() const noexcept { return hash_; }

    // Static hash function (can be used at compile time in C++20)
    static constexpr HashType Hash(const char* str) noexcept {
        // FNV-1a hash algorithm
        HashType hash = 14695981039346656037ULL;
        while (*str) {
            hash ^= static_cast<HashType>(*str++);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

private:
    HashType hash_;
};

// ThreadLocal: Thread-local storage wrapper (placeholder for boost::fiber::fiber_specific_ptr)
// TODO: Replace with actual fiber-local storage when Boost.Fiber is available
export template <typename T>
class ThreadLocal {
public:
    ThreadLocal() = default;
    ~ThreadLocal() = default;

    T* get() const noexcept {
        return ptr_.get();
    }

    void reset(T* ptr = nullptr) {
        ptr_.reset(ptr);
    }

    T* operator->() const noexcept {
        return ptr_.get();
    }

    T& operator*() const noexcept {
        return *ptr_;
    }

private:
    std::unique_ptr<T> ptr_;
};
