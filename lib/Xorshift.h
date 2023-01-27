#pragma once
#ifndef _XORSHIFT_H
#define _XORSHIFT_H
#include <cstdint>

namespace xorshift
{
    class xorshift32
    {
    private:
        uint32_t m_seed;

    public:
        xorshift32(uint32_t _Seed = 1) : m_seed(_Seed) {}
        void seed(uint32_t _Seed) { m_seed = _Seed; }
        void discard(uint64_t z);
        uint32_t operator()(void);
    };

    void xorshift32::discard(uint64_t z)
    {
        while (z--)
            this->operator()();
    }

    uint32_t xorshift32::operator()(void)
    {
        m_seed ^= m_seed << 13;
        m_seed ^= m_seed >> 17;
        m_seed ^= m_seed << 15;
        return m_seed;
    }

    class xorshift64
    {
    private:
        uint64_t m_seed;

    public:
        xorshift64(uint64_t _Seed = 1) : m_seed(_Seed) {}
        void seed(uint64_t seed) { m_seed = seed; }
        void discard(uint64_t z);
        uint64_t operator()(void);
    };

    void xorshift64::discard(uint64_t z)
    {
        while (z--)
            this->operator()();
    }

    uint64_t xorshift64::operator()(void)
    {
        m_seed ^= m_seed << 13;
        m_seed ^= m_seed >> 7;
        m_seed ^= m_seed << 17;
        return m_seed;
    }

} // namespace xorshift

#endif //_XORSHIFT_H