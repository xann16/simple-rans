#ifndef CODING_NUM_FREQ_TABLE_ALIAS_H_INCLUDED
#define CODING_NUM_FREQ_TABLE_ALIAS_H_INCLUDED

#include <cmath>
#include <cstdio>
#include <cstring>
#include <tuple>
#include <utility>

#include "bit_buffer.h"
#include "data_block.h"

namespace coding
{
  // table storing symbol frequencies for certain data block
  template < std::size_t SL, std::size_t N >
  class num_freq_table_alias
  {
  public:
    num_freq_table_alias() noexcept {}

    template < std::size_t _BufSize >
    explicit num_freq_table_alias( bit_buffer< _BufSize > &buf )
    {
      buf.read( size() * sizeof( word ), m_cdf_p );
      m_cdf_p[ size() ] = num_base();
      construct_alias_table();
    }

    template < std::size_t _DataSize >
    explicit num_freq_table_alias( data_block< _DataSize, SL > &data )
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
      construct_alias_table();
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
      auto bucket =
        static_cast< byte >( value >> static_cast< word >( N - SL ) );
      auto shift = value & ( ( 1u << static_cast< word >( N - SL ) ) - 1u );
      return shift < m_dividers[ bucket ] ? bucket : m_aliases[ bucket ];
    }

    void display()
    {
      std::printf( "Total symbol count:    %5u\n", symbol_count() );
      std::printf( "Numeral system size:   %5u\n", num_base() );
      std::printf( "Alias bucket size:     %5u\n", bucket_size() );
      std::printf( "Bits/symbol (Shannon): %10.4f\n",
                   bits_per_symbol_theory() );
      std::printf(
        " S    f(S)     C(S)     p(S)    cdf(S)   div(S)  alias(S)\n" );
      for ( std::size_t i = 0; i < size(); ++i )
      {
        std::printf( " %02x %8u %8u %.6f %.6f %8u   %02x\n",
                     static_cast< uint >( i ), f( i ), cdf( i ), p( i ),
                     p_cdf( i ), m_dividers[ i ], m_aliases[ i ] );
      }
    }

    template < std::size_t _BufSize >
    void write_header( bit_buffer< _BufSize > &buf ) noexcept
    {
      buf.write( ( size() ) * sizeof( word ), m_cdf_p );
    }

    bool operator==( num_freq_table_alias const &other ) const noexcept
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

    bool operator!=( num_freq_table_alias const &other ) const noexcept
    {
      return !( *this == other );
    }

    template < std::size_t _BufSize >
    static num_freq_table_alias
    read_header_reverse( bit_buffer< _BufSize > &buf )
    {
      auto result = num_freq_table_alias{};
      buf.read_reverse( size() * sizeof( word ), result.m_cdf_p );
      result.m_cdf_p[ size() ] = num_base();
      result.construct_alias_table();
      return result;
    }

    ulong rans_encode_adjust( byte s, ulong x ) const noexcept
    {
      return m_alias_remap[ ( x % static_cast< ulong >( f( s ) ) ) +
                            static_cast< ulong >( cdf( s ) ) ];
    }

    std::pair< ulong, ulong > adjusted_f_and_cdf( [[maybe_unused]] word s,
                                                  [[maybe_unused]] ulong value )
    {
      auto bucket =
        static_cast< ulong >( value >> static_cast< ulong >( N - SL ) );
      auto shift = value & ( ( 1u << static_cast< ulong >( N - SL ) ) - 1u );
      auto i = bucket << 1u;
      if ( shift >= m_dividers[ bucket ] )
      {
        i++;
      }

      return std::make_pair( static_cast< ulong >( m_slot_freq[ i ] ),
                             static_cast< ulong >( m_slot_csum[ i ] ) );
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


    template < typename T, std::size_t M >
    class bounded_stack
    {
    public:
      void push( T value ) noexcept { m_data_p[ m_top++ ] = value; }
      T pop() noexcept { return m_data_p[ --m_top ]; }
      T top() const noexcept { return m_data_p[ m_top - 1 ]; }
      std::size_t size() const noexcept { return m_top; }
      operator bool() const noexcept { return m_top != 0u; }

    private:
      std::size_t m_top = 0u;
      T m_data_p[ M ] = {};
    };

    using bstack = bounded_stack< byte, 1u << SL >;

    void construct_alias_table()
    {
      auto large = bstack{};
      auto small = bstack{};

      for ( std::size_t i = 0u; i < size(); ++i )
      {
        m_dividers[ i ] = f( i );
        if ( m_dividers[ i ] > bucket_size() )
        {
          large.push( static_cast< byte >( i ) );
        }
        else
        {
          small.push( static_cast< byte >( i ) );
        }
      }

      while ( large && small )
      {
        auto sm = small.pop();
        auto lg = large.pop();

        m_aliases[ sm ] = lg;
        m_dividers[ lg ] -= bucket_size() - m_dividers[ sm ];

        if ( m_dividers[ lg ] < bucket_size() )
        {
          small.push( lg );
        }
        else
        {
          large.push( lg );
        }
      }
      assert_alias();
      construct_alias_remap();
    }

    void construct_alias_remap() noexcept
    {
      word used[ 1u << SL ] = {};
      // for (std::size_t i = 0u; i < size(); ++i)
      //{
      //  remaining[i] = f(i);
      //}

      for ( std::size_t i = 0u; i < size(); ++i )
      {
        // initial bucket symbols
        auto orig_curr = cdf( i ) + used[ i ];
        auto alias_beg =
          static_cast< word >( bucket_size() * static_cast< word >( i ) );
        auto count = m_dividers[ i ];
        m_slot_freq[ ( i << 1u ) + 0u ] = f( i );
        m_slot_csum[ ( i << 1u ) + 0u ] = alias_beg + used[ i ];
        used[ i ] += count;
        auto alias_end = static_cast< word >( alias_beg + count );

        while ( alias_beg < alias_end )
        {
          m_alias_remap[ orig_curr++ ] = alias_beg++;
        }

        // aliased bucket symbols
        auto j = m_aliases[ i ];
        orig_curr = cdf( j ) + used[ j ];
        alias_beg = alias_end;
        count = bucket_size() - count;
        m_slot_freq[ ( i << 1u ) + 0u ] = f( j );
        m_slot_csum[ ( i << 1u ) + 0u ] = alias_beg + used[ j ];
        used[ j ] += count;
        alias_end = alias_beg + count;

        while ( alias_beg < alias_end )
        {
          m_alias_remap[ orig_curr++ ] = alias_beg++;
        }
      }

      // check correctness
      for ( std::size_t i = 0u; i < size(); ++i )
      {
        if ( used[ i ] != f( i ) )
        {
          std::printf( "Invalid alias remap table!" );
        }
      }
    }

    word bucket_size() const noexcept
    {
      return static_cast< word >( 1u << ( N - SL ) );
    }

    void assert_alias()
    {
      word freqs[ 1u << SL ] = {};
      for ( std::size_t i = 0u; i < size(); ++i )
      {
        freqs[ i ] += m_dividers[ i ];
        freqs[ m_aliases[ i ] ] += bucket_size() - m_dividers[ i ];
      }
      for ( std::size_t i = 0u; i < size(); ++i )
      {
        if ( freqs[ i ] != f( i ) )
        {
          std::printf( "Invalid alias table!" );
        }
      }
    }

  private:
    word m_cdf_p[ ( 1u << SL ) + 1u ] = {};
    uint m_symbol_count = 0u;
    double m_bits_per_symbol_theory = 0.0;

    word m_dividers[ 1u << SL ] = {};
    byte m_aliases[ 1u << SL ] = {};
    word m_alias_remap[ 1u << N ] = {};
    word m_slot_freq[ 1u << ( SL + 1u ) ] = {};
    word m_slot_csum[ 1u << ( SL + 1u ) ] = {};
  };

}  // namespace coding

#endif  // !CODING_NUM_FREQ_TABLE_ALIAS_H_INCLUDED
