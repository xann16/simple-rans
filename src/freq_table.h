#ifndef CODING_FREQ_TABLE_H_INCLUDED
#define CODING_FREQ_TABLE_H_INCLUDED

#include <cmath>
#include <cstdio>
#include <cstring>

#include "bit_buffer.h"
#include "data_block.h"

namespace coding
{
  // table storing symbol frequencies for certain data block
  template < std::size_t SL >
  class freq_table
  {
  public:
    freq_table() noexcept
    {
      std::memset( m_freqs_p, 0, ( size() + 1 ) * sizeof( uint ) );
      init();
    }

    template < std::size_t N >
    explicit freq_table( bit_buffer< N > &buf )
    {
      buf.read( ( size() + 1 ) * sizeof( uint ), m_freqs_p );
      init();
    }

    template < std::size_t N >
    explicit freq_table( data_block< N, SL > &data )
    {
      std::memset( m_freqs_p, 0, ( size() ) * sizeof( uint ) );
      uint count = 0u;
      auto cursor_pos = data.get_position();
      while ( data )
      {
        ++m_freqs_p[ data.read_symbol() ];
        count++;
      }
      data.set_position( cursor_pos );
      m_freqs_p[ size() ] = count;
      init();
    }

    ~freq_table() noexcept = default;

    std::size_t size() const noexcept { return 1u << SL; }

    uint header_length() const noexcept
    {
      return ( static_cast< uint >( ( size() ) + 1u ) * sizeof( uint ) ) << 3u;
    }

    uint freq( std::size_t index ) const noexcept { return m_freqs_p[ index ]; }

    uint symbol_count() const noexcept { return m_freqs_p[ size() ]; }

    double p( std::size_t index ) const noexcept
    {
      return m_cdf_p[ index + 1 ] - m_cdf_p[ index ];
    }

    double cdf( std::size_t index ) const noexcept { return m_cdf_p[ index ]; }

    double bits_per_symbol_theory()
    {
      // cf. Shannon
      double result = 0.0;
      for ( std::size_t i = 0; i < size(); ++i )
      {
        auto pr = p( i );
        if ( pr != 0.0 )
        {
          result -= pr * std::log2( pr );
        }
      }
      return result;
    }

    byte symbol( double value ) const noexcept
    {
      // simple linear search
      std::size_t i = 0u;
      while ( i < size() )
      {
        if ( value < m_cdf_p[ i ] )
        {
          break;
        }
        ++i;
      }
      return static_cast< byte >( i - 1u );
    }

    void display()
    {
      std::printf( "Total symbol count: %u\n", symbol_count() );
      std::printf( " S  frequency   p(S)     C(S)  \n" );
      for ( std::size_t i = 0; i < size(); ++i )
      {
        std::printf( " %02x %9u %.6f %.6f\n", static_cast< uint >( i ),
                     freq( i ), p( i ), cdf( i ) );
      }
    }

    template < std::size_t N >
    void write_header( bit_buffer< N > &buf ) noexcept
    {
      buf.write( ( size() + 1 ) * sizeof( uint ), m_freqs_p );
    }

    bool operator==( freq_table const &other ) const noexcept
    {
      for ( std::size_t i = 0u; i < size() + 1; ++i )
      {
        if ( m_freqs_p[ i ] != other.m_freqs_p[ i ] )
        {
          return false;
        }
      }
      return true;
    }

    bool operator!=( freq_table const &other ) const noexcept
    {
      return !( *this == other );
    }

  private:
    void init() noexcept
    {
      if ( symbol_count() == 0 )
      {
        memset( m_cdf_p, 0, ( size() + 1 ) * sizeof( float ) );
      }
      else
      {
        m_cdf_p[ 0 ] = 0.0;
        auto count = static_cast< double >( symbol_count() );
        for ( std::size_t i = 0u; i < size(); ++i )
        {
          auto prob = static_cast< double >( m_freqs_p[ i ] ) / count;
          m_cdf_p[ i + 1 ] = m_cdf_p[ i ] + prob;
        }
      }
    }

  private:
    uint m_freqs_p[ ( 2u << SL ) + 1u ];
    double m_cdf_p[ ( 2u << SL ) + 1 ];
  };

}  // namespace coding

#endif  // !CODING_FREQ_TABLE_H_INCLUDED
