#ifndef CORE_LOGGER_BUFFER_H
#define CORE_LOGGER_BUFFER_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <streambuf>
#include <string_view>
#include <vector>

namespace pans::detail {
    template<std::size_t INLINE_CAPACITY>
    class InlineBuffer {
    public:
        void append(const char *data, std::size_t size) {
            if (size == 0) {
                return;
            }
            // m_inline足够存放则存入
            if (m_overflow.empty() && m_size + size <= INLINE_CAPACITY) {
                std::memcpy(m_inline.data() + m_size, data, size);
                m_size += size;
                return;
            }
            if (m_overflow.empty()) {
                const std::size_t required_capacity = m_size + size;
                m_overflow.reserve(std::max(required_capacity, INLINE_CAPACITY * 2)); // ?
                m_overflow.insert(m_overflow.end(), m_inline.data(), m_inline.data() + m_size);
            }
            m_overflow.insert(m_overflow.end(), data, data + size);
            m_size = m_overflow.size();
        }

        void append(std::string_view value) {
            append(value.data(), value.size());
        }

        void append(char value) {
            append(&value, 1);
        }

        [[nodiscard]] const char *data() const noexcept {
            return m_overflow.empty() ? m_inline.data() : m_overflow.data();
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return m_size;
        }

        [[nodiscard]] std::string_view view() const noexcept {
            return std::string_view{data(), size()};
        }

    private:
        // 较短的日志, 存在栈上的array, 长度不超过INLINE_CAPACITY
        std::array<char, INLINE_CAPACITY> m_inline{};
        // 较长的日志, 存在堆上
        std::vector<char> m_overflow{};
        std::size_t m_size{0};
    };

    template<std::size_t INLINE_CAPACITY>
    class SmallStringBuffer final : public std::streambuf {
    public:
        explicit SmallStringBuffer(InlineBuffer<INLINE_CAPACITY> &buffer) noexcept : m_buffer(buffer) {}
    protected:
        std::streamsize xsputn(const char* data, std::streamsize size) noexcept override {
            if (size <= 0) return 0;
            m_buffer.append(data, static_cast<std::size_t>(size));
            return size;
        }
        int_type overflow(int_type c) override {
            if (traits_type::eq_int_type(c, traits_type::eof())) {
                return traits_type::not_eof(c);
            }
            m_buffer.append(traits_type::to_char_type(c));
            return c;
        }
    private:
        InlineBuffer<INLINE_CAPACITY> &m_buffer;
    };
}

#endif
