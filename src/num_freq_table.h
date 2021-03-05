#ifndef CODING_NUM_FREQ_TABLE_H_INCLUDED
#define CODING_NUM_FREQ_TABLE_H_INCLUDED

#include <cmath>
#include <cstdio>
#include <cstring>

#include "bit_buffer.h"
#include "data_block.h"

namespace coding
{
  // table storing symbol frequencies for certain data block
  template < std::size_t SL, std::size_t N >
  class num_freq_table
  {
  public:
    num_freq_table() noexcept {}

    template < std::size_t _BufSize >
    explicit num_freq_table( bit_buffer< _BufSize > &buf )
    {
      buf.read( size() * sizeof( word ), m_cdf_p );
      m_cdf_p[ size() ] = num_base();
    }

    template < std::size_t _DataSize >
    explicit num_freq_table( data_block< _DataSize, SL > &data )
    {
      uint freqs[ 1u << SL ] = {};
      auto cursor_pos = data.get_position();
      while ( data )
      {
        ++freqs[ data.read_symbol() ];
        ++m_symbol_count;
      }
      data.set_position( cursor_pos );
      init( freqs );
      m_cdf_p[ size() ] = num_base();
    }

    static std::size_t size() noexcept { return 1u << SL; }

    uint header_length() const noexcept
    {
      return static_cast< uint >( size() * sizeof( word ) ) << 3u;
    }

    static word num_base() noexcept { return 1u << N; }

    word num_mask() const noexcept { return ( 1u << N ) - 1u; }

    word cdf( std::size_t index ) const noexcept { return m_cdf_p[ index ]; }

    word f( std::size_t index ) const noexcept
    {
      return m_cdf_p[ index + 1 ] - m_cdf_p[ index ];
    }

    double p_cdf( std::size_t index ) const noexcept
    {
      return static_cast< double >( cdf( index ) ) /
             static_cast< double >( num_base() );
    }

    double p( std::size_t index ) const noexcept
    {
      return static_cast< double >( f( index ) ) /
             static_cast< double >( num_base() );
    }

    uint symbol_count() const noexcept { return m_symbol_count; }

    double bits_per_symbol_theory() const noexcept
    {
      return m_bits_per_symbol_theory;
    }

    byte symbol( word value ) const noexcept
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
      std::printf( "Total symbol count:    %5u\n", symbol_count() );
      std::printf( "Numeral system size:   %5u\n", num_base() );
      std::printf( "Bits/symbol (Shannon): %10.4f\n",
                   bits_per_symbol_theory() );
      std::printf( " S    f(S)     C(S)     p(S)    cdf(S)\n" );
      for ( std::size_t i = 0; i < size(); ++i )
      {
        std::printf( " %02x %8u %8u %.6f %.6f\n", static_cast< uint >( i ),
                     f( i ), cdf( i ), p( i ), p_cdf( i ) );
      }
    }

    template < std::size_t _BufSize >
    void write_header( bit_buffer< _BufSize > &buf ) noexcept
    {
      buf.write( ( size() ) * sizeof( word ), m_cdf_p );
    }

    bool operator==( num_freq_table const &other ) const noexcept
    {
      for ( std::size_t i = 0u; i < size(); ++i )
      {
        if ( m_cdf_p[ i ] != other.m_cdf_p[ i ] )
        {
          return false;
        }
      }
      return true;
    }

    bool operator!=( num_freq_table const &other ) const noexcept
    {
      return !( *this == other );
    }

    template < std::size_t _BufSize >
    static num_freq_table read_header_reverse( bit_buffer< _BufSize > &buf )
    {
      auto result = num_freq_table{};
      buf.read_reverse( size() * sizeof( word ), result.m_cdf_p );
      result.m_cdf_p[ size() ] = num_base();
      return result;
    }

    ulong rans_encode_adjust( byte s, ulong x ) const noexcept
    {
      return ( x % static_cast< ulong >( f( s ) ) ) +
             static_cast< ulong >( cdf( s ) );
    }

    std::pair< ulong, ulong > adjusted_f_and_cdf( [[maybe_unused]] word s,
                                                  [[maybe_unused]] ulong value )
    {
      return std::make_pair( static_cast< ulong >( f( s ) ),
                             static_cast< ulong >( cdf( s ) ) );
    }

  private:
    void init( uint *freqs_p ) noexcept
    {
      init_bits_per_symbol_theory( freqs_p );

      for ( std::size_t i = 1u; i < size(); ++i )
      {
        freqs_p[ i ] += freqs_p[ i - 1 ];
      }

      m_cdf_p[ 0 ] = 0u;

      auto fac = static_cast< double >( num_base() ) /
                 static_cast< double >( symbol_count() );

      for ( std::size_t i = 1u; i < size(); ++i )
      {
        m_cdf_p[ i ] = static_cast< word >(
          static_cast< double >( freqs_p[ i - 1 ] ) * fac );
      }
    }

    void init_bits_per_symbol_theory( uint *freqs_p )
    {
      // cf. Shannon
      m_bits_per_symbol_theory = 0.0;
      for ( std::size_t i = 0; i < size(); ++i )
      {
        auto pr = static_cast< double >( freqs_p[ i ] ) /
                  static_cast< double >( m_symbol_count );
        if ( pr != 0.0 )
        {
          m_bits_per_symbol_theory -= pr * std::log2( pr );
        }
      }
    }

  private:
    word m_cdf_p[ ( 1u << SL ) + 1u ] = {};
    uint m_symbol_count = 0u;
    double m_bits_per_symbol_theory = 0.0;
  };

}  // namespace coding

#endif  // !CODING_NUM_FREQ_TABLE_H_INCLUDED
