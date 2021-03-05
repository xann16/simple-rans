#ifndef CODING_DATA_BLOCK_H_INCLUDED
#define CODING_DATA_BLOCK_H_INCLUDED

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <utility>

#include "common.h"

namespace coding
{
  // binary symbol buffer of fixed maximal size with non-parallelizable
  //  sequential access
  // supported symbol lengths: 1, 2, 4, 8
  template < std::size_t N, std::size_t SL >
  class data_block
  {
  public:
    data_block() noexcept { reset(); }
    explicit data_block( char const *filepath ) { load( filepath ); }
    data_block( data_block const & ) = delete;
    data_block( data_block && ) = delete;
    data_block &operator=( data_block const & ) = delete;
    data_block &operator=( data_block && ) = delete;
    ~data_block() noexcept = default;

    std::size_t size() const noexcept
    {
      return raw_byte_count() + ( ( m_bit_offset == 0 ) ? 0 : 1 );
    }

    std::size_t max_size() const noexcept { return N; }

    std::size_t bit_count() const noexcept
    {
      return ( raw_byte_count() << 3u ) + m_bit_offset;
    }

    std::size_t max_bit_count() const noexcept { return N << 3u; }

    std::size_t symbol_count() const noexcept { return bit_count() / SL; }

    std::size_t max_symbol_count() const noexcept
    {
      return max_bit_count() / SL;
    }

    std::size_t symbol_length() const noexcept { return SL; }

    operator bool() const noexcept { return m_curr_p != m_end_p; }

    bool is_beg() const noexcept
    {
      return m_curr_p == m_data_p && m_bit_offset == 0u;
    }

    void prepare_full()
    {
      m_end_p = m_data_p;
      std::advance( m_end_p, max_size() );
      m_curr_p = m_end_p;
    }

    void write_symbol( byte symbol )
    {
      symbol = static_cast< byte >(
        ( ( symbol & mask() ) << static_cast< byte >( m_bit_offset ) ) );
      *m_curr_p |= symbol;
      advance_symbol_length();
      m_end_p = m_curr_p;
    }

    void write_symbol_reverse( byte symbol )
    {
      reverse_symbol_length();
      symbol = static_cast< byte >(
        ( ( symbol & mask() ) << static_cast< byte >( m_bit_offset ) ) );
      *m_curr_p |= symbol;
    }

    byte read_symbol() noexcept
    {
      auto result = static_cast< byte >(
        static_cast< byte >( ( *m_curr_p ) >> m_bit_offset ) & mask() );
      advance_symbol_length();
      return result;
    }

    void reset() noexcept
    {
      std::memset( static_cast< void * >( m_data_p ), 0, N );
      m_curr_p = m_data_p;
      m_end_p = m_data_p;
      m_bit_offset = 0;
    }

    void load( char const *filepath )
    {
      reset();

      auto fin = std::ifstream( filepath, std::ios::binary );
      if ( !fin.is_open() )
      {
        std::printf( "Failed to load data from file: '%s'.", filepath );
      }

      auto bytes_read =
        fin.readsome( reinterpret_cast< char * >( m_data_p ), N );
      std::advance( m_end_p, bytes_read );

      fin.close();
    }

    void rewind() noexcept
    {
      m_curr_p = m_data_p;
      m_bit_offset = 0;
    }

    std::pair< std::size_t, std::size_t > get_position() const noexcept
    {
      return std::make_pair( static_cast< std::size_t >(
                               reinterpret_cast< std::uintptr_t >( m_end_p ) -
                               reinterpret_cast< std::uintptr_t >( m_data_p ) ),
                             m_bit_offset );
    }

    void set_position( std::pair< std::size_t, std::size_t > &pos ) noexcept
    {
      rewind();
      std::advance( m_curr_p, pos.first );
      m_bit_offset = pos.second;
    }

    bool operator==( data_block const &other ) const noexcept
    {
      if ( size() != other.size() )
      {
        return false;
      }

      for ( std::size_t i = 0; i < size(); ++i )
      {
        if ( m_data_p[ i ] != other.m_data_p[ i ] )
        {
          return false;
        }
      }

      return true;
    }

    bool operator!=( data_block const &other ) const noexcept
    {
      return !( *this == other );
    }

    void print()
    {
      for ( size_t i = 0u; i < size(); ++i )
      {
        std::printf( "%c", m_data_p[ i ] );
      }
    }

  private:
    void advance_symbol_length()
    {
      m_bit_offset = ( m_bit_offset + SL ) % 8u;
      if ( m_bit_offset == 0u )
      {
        std::advance( m_curr_p, 1 );
      }
    }

    void reverse_symbol_length()
    {
      if ( m_bit_offset == 0u )
      {
        std::advance( m_curr_p, -1 );
      }
      m_bit_offset = ( m_bit_offset - SL ) % 8u;
    }

    byte mask() const noexcept
    {
      return ( byte{ 1 } << static_cast< byte >( SL ) ) - byte{ 1 };
    }

    std::size_t raw_byte_count() const noexcept
    {
      return static_cast< std::size_t >(
        reinterpret_cast< std::uintptr_t >( m_end_p ) -
        reinterpret_cast< std::uintptr_t >( m_data_p ) );
    }

  private:
    byte m_data_p[ N ];
    byte *m_curr_p;
    byte *m_end_p;
    std::size_t m_bit_offset = 0;
  };

}  // namespace coding

#endif  // !CODING_DATA_BLOCK_H_INCLUDED
