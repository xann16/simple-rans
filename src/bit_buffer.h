#ifndef CODING_BIT_BUFFER_H_INCLUDED
#define CODING_BIT_BUFFER_H_INCLUDED

#include <cstring>
#include <iterator>

#include "common.h"

namespace coding
{
  // bit buffer of fixed maximal size with non-parallelizable sequential access
  template < std::size_t N >
  class bit_buffer
  {
  public:
    bit_buffer() noexcept : m_curr_p( m_data_p ), m_end_p( m_data_p ) {}
    bit_buffer( bit_buffer const & ) = delete;
    bit_buffer( bit_buffer && ) = delete;
    bit_buffer &operator=( bit_buffer const & ) = delete;
    bit_buffer &operator=( bit_buffer && ) = delete;
    ~bit_buffer() noexcept = default;

    std::size_t size() const noexcept
    {
      return static_cast< std::size_t >(
        reinterpret_cast< std::uintptr_t >( m_end_p ) -
        reinterpret_cast< std::uintptr_t >( m_data_p ) );
    }

    std::size_t max_size() const noexcept { return N; }

    void write( std::size_t n, const void *src_p )
    {
      std::memcpy( static_cast< void * >( m_curr_p ), src_p, n );
      advance( n );
      m_end_p = m_curr_p;
    }

    void write_word( word value )
    {
      write( 2, static_cast< void * >( &value ) );
    }

    void read( std::size_t n, void *dst_p ) noexcept
    {
      std::memcpy( dst_p, static_cast< void * >( m_curr_p ), n );
      advance( n );
    }

    word read_word()
    {
      word res;
      read( 2, static_cast< void * >( &res ) );
      return res;
    }

    void read_reverse( std::size_t n, void *dst_p ) noexcept
    {
      reverse( n );
      std::memcpy( dst_p, static_cast< void * >( m_curr_p ), n );
    }

    word read_word_reverse()
    {
      word res;
      read_reverse( 2, static_cast< void * >( &res ) );
      return res;
    }

    void reset() noexcept
    {
      m_curr_p = m_data_p;
      m_end_p = m_data_p;
    }

    void advance( std::size_t offset ) noexcept
    {
      // std::printf( "===BB=== Advancing by %lu bytes.\n", offset );
      std::advance( m_curr_p, offset );
    }

    void reverse( std::size_t offset ) noexcept
    {
      // std::printf( "===BB=== Reversing by %lu bytes.\n", offset );
      std::advance( m_curr_p, -static_cast< int >( offset ) );
    }

    void rewind() noexcept { m_curr_p = m_data_p; }

    uint length() const noexcept { return static_cast< uint >( size() ) << 3u; }

    operator bool() const noexcept { return m_curr_p != m_end_p; }

    bool is_beg() const noexcept { return m_curr_p == m_data_p; }

  private:
    byte m_data_p[ N ];
    byte *m_curr_p;
    byte *m_end_p;
  };

}  // namespace coding

#endif  // !CODING_BIT_BUFFER_H_INCLUDED
