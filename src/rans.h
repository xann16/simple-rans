#ifndef CODING_RANS_H_INCLUDED
#define CODING_RANS_H_INCLUDED

#include <chrono>

#include "bit_buffer.h"
#include "compr_stats.h"
#include "data_block.h"

namespace coding::rans
{

  template < std::size_t _NumBase,
             template < std::size_t, std::size_t > class _FreqTable,
             std::size_t _BufSize, std::size_t _DataSize, std::size_t _SymLen >
  compr_stats< _SymLen > encode( data_block< _DataSize, _SymLen > &src,
                                 bit_buffer< _BufSize > &dst )
  {
    auto stats = compr_stats< _SymLen >{};

    // start encoding
    auto start_time = std::chrono::high_resolution_clock::now();

    // compute frequency table from data block and write it to buffer
    auto ft = _FreqTable< _SymLen, _NumBase >( src );
    src.rewind();

    // ---------------

    const ulong MASK = ( 1ul << 16ul ) - 1ul;
    ulong d = 32 - _NumBase;
    ulong x = 0ul;

    while ( src )
    {
      auto s = static_cast< ulong >( src.read_symbol() );
      if ( x >= ( static_cast< ulong >( ft.f( s ) ) << d ) )
      {
        dst.write_word( static_cast< word >( x & MASK ) );
        x >>= 16ul;
      }
      // x = ( ( x / f ) << _NumBase ) + ( x % f ) +
      //    static_cast< ulong >( ft.cdf( s ) );
      x = ( ( x / static_cast< ulong >( ft.f( s ) ) ) << _NumBase ) +
          ft.rans_encode_adjust( static_cast< byte >( s ), x );
    }

    while ( x > 0 )
    {
      dst.write_word( static_cast< word >( x & MASK ) );
      x >>= 16ul;
    }

    ft.write_header( dst );

    // ---------------

    // end encoding
    auto encoding_time = std::chrono::duration_cast< std::chrono::nanoseconds >(
      std::chrono::high_resolution_clock::now() - start_time );

    // write current run stats
    stats.set_header_length( ft.header_length() );
    stats.set_symbol_count( ft.symbol_count() );
    stats.set_bits_per_symbol_theory( ft.bits_per_symbol_theory() );
    stats.set_encoded_length( dst.length() );
    stats.set_encoding_time( encoding_time );

    return stats;
  }

  template < std::size_t _NumBase,
             template < std::size_t, std::size_t > class _FreqTable,
             std::size_t _BufSize, std::size_t _DataSize, std::size_t _SymLen >
  std::chrono::nanoseconds decode( bit_buffer< _BufSize > &src,
                                   data_block< _DataSize, _SymLen > &dst )
  {
    // start decoding
    auto start_time = std::chrono::high_resolution_clock::now();

    // decode frequency table from input bits
    auto ft = _FreqTable< _SymLen, _NumBase >::read_header_reverse( src );

    // ---------------

    auto mask = static_cast< ulong >( ft.num_mask() );
    ulong x = 0ul;

    while ( x < ft.num_base() )
    {
      x = ( x << 16ul ) + static_cast< ulong >( src.read_word_reverse() );
    }

    while ( !src.is_beg() )
    {
      auto s = ft.symbol( static_cast< word >( x & mask ) );
      dst.write_symbol_reverse( s );
      auto [ f, cdf ] = ft.adjusted_f_and_cdf( s, x & mask );
      // x = ( ft.f( s ) * ( x >> _NumBase ) ) + ( x & mask ) - ft.cdf( s );
      x = ( f * ( x >> _NumBase ) ) + ( x & mask ) - cdf;
      if ( x < ( 1ul << 16ul ) )
      {
        x = ( x << 16ul ) + static_cast< ulong >( src.read_word_reverse() );
      }
    }

    while ( x > 0 )
    {
      auto s = ft.symbol( static_cast< word >( x & mask ) );
      dst.write_symbol_reverse( s );
      x = ( ft.f( s ) * ( x >> _NumBase ) ) + ( x & mask ) - ft.cdf( s );
    }

    // ---------------

    // end decoding
    auto decoding_time = std::chrono::duration_cast< std::chrono::nanoseconds >(
      std::chrono::high_resolution_clock::now() - start_time );

    return decoding_time;
  }


  // VARIANTS WITH VERBOSE MODE OPTION

  template < std::size_t _NumBase,
             template < std::size_t, std::size_t > class _FreqTable,
             std::size_t _BufSize, std::size_t _DataSize, std::size_t _SymLen >
  compr_stats< _SymLen > encode( data_block< _DataSize, _SymLen > &src,
                                 bit_buffer< _BufSize > &dst, bool verbose )
  {
    auto stats = compr_stats< _SymLen >{};

    // start encoding
    auto start_time = std::chrono::high_resolution_clock::now();

    // compute frequency table from data block and write it to buffer
    auto ft = _FreqTable< _SymLen, _NumBase >( src );
    src.rewind();

    // ---------------

    const ulong MASK = ( 1ul << 16ul ) - 1ul;
    ulong d = 32 - _NumBase;
    ulong x = 0ul;
    ulong i = 0;
    ulong w = 0;

    while ( src )
    {
      auto s = static_cast< ulong >( src.read_symbol() );
      if ( verbose )
      {
        std::printf( "[ENC] -> #s=%5lu: symbol read: %02lx\n", ++i, s );
      }
      if ( x >= ( static_cast< ulong >( ft.f( s ) ) << d ) )
      {
        dst.write_word( static_cast< word >( x & MASK ) );
        if ( verbose )
        {
          std::printf( "[ENC] <- #w=%5lu - word written: %04x\n", ++w,
                       static_cast< word >( x & MASK ) );
        }
        if ( verbose )
        {
          std::printf( "[ENC] \t\t\t\t\t\t x: %lu -> ", x );
        }
        x >>= 16ul;
        if ( verbose )
        {
          std::printf( "%lu\n", x );
        }
      }
      if ( verbose )
      {
        std::printf( "[ENC] \t\t\t\t\t\t x: %lu -> ", x );
      }
      // x = ( ( x / f ) << _NumBase ) + ( x % f ) +
      //    static_cast< ulong >( ft.cdf( s ) );
      x = ( ( x / static_cast< ulong >( ft.f( s ) ) ) << _NumBase ) +
          ft.rans_encode_adjust( static_cast< byte >( s ), x );
      if ( verbose )
      {
        std::printf( "%lu\n", x );
      }
    }


    while ( x > 0 )
    {
      dst.write_word( static_cast< word >( x & MASK ) );
      if ( verbose )
      {
        std::printf( "[ENC] %5lu: -> %lu. word written: %04x\n", i, ++w,
                     static_cast< word >( x & MASK ) );
      }
      if ( verbose )
      {
        std::printf( "[ENC] \t\t\t\t\t\t x: %lu -> ", x );
      }
      x >>= 16ul;
      if ( verbose )
      {
        std::printf( "%lu\n", x );
      }
    }


    ft.write_header( dst );

    if ( verbose )
    {
      std::printf(
        "[ENC] Encoding completed. Encoded %lu symbols in %lu bytes.\n\n", i,
        w * 2 );
    }

    // ---------------

    // end encoding
    auto encoding_time = std::chrono::duration_cast< std::chrono::nanoseconds >(
      std::chrono::high_resolution_clock::now() - start_time );

    // write current run stats
    stats.set_header_length( ft.header_length() );
    stats.set_symbol_count( ft.symbol_count() );
    stats.set_bits_per_symbol_theory( ft.bits_per_symbol_theory() );
    stats.set_encoded_length( dst.length() );
    stats.set_encoding_time( encoding_time );

    return stats;
  }

  template < std::size_t _NumBase,
             template < std::size_t, std::size_t > class _FreqTable,
             std::size_t _BufSize, std::size_t _DataSize, std::size_t _SymLen >
  std::chrono::nanoseconds decode( bit_buffer< _BufSize > &src,
                                   data_block< _DataSize, _SymLen > &dst,
                                   bool verbose )
  {
    // start decoding
    auto start_time = std::chrono::high_resolution_clock::now();

    // decode frequency table from input bits
    auto ft = _FreqTable< _SymLen, _NumBase >::read_header_reverse( src );

    // ---------------

    auto mask = static_cast< ulong >( ft.num_mask() );
    ulong i = 0ul;
    ulong w = 0ul;

    ulong x = 0ul;


    while ( x < ft.num_base() )
    {
      auto new_word = static_cast< ulong >( src.read_word_reverse() );
      if ( verbose )
      {
        std::printf( "[DEC] -> #w=%5lu - word read: %04lx\n", ++w, new_word );
      }
      if ( verbose )
      {
        std::printf( "[DEC] \t\t\t\t\t\t x: %lu -> ", x );
      }
      x = ( x << 16ul ) + new_word;
      if ( verbose )
      {
        std::printf( "%lu\n", x );
      }
    }


    while ( !src.is_beg() )
    {
      auto s = ft.symbol( static_cast< word >( x & mask ) );
      dst.write_symbol_reverse( s );
      if ( verbose )
      {
        std::printf( "[DEC] <- #s=%5lu - symbol written: %02x\n", ++i, s );
      }
      if ( verbose )
      {
        std::printf( "[DEC] \t\t\t\t\t\t x: %lu -> ", x );
      }
      auto [ f, cdf ] = ft.adjusted_f_and_cdf( s, x & mask );
      // x = ( ft.f( s ) * ( x >> _NumBase ) ) + ( x & mask ) - ft.cdf( s );
      x = ( f * ( x >> _NumBase ) ) + ( x & mask ) - cdf;
      if ( verbose )
      {
        std::printf( "%lu\n", x );
      }
      if ( x < ( 1ul << 16ul ) )
      {
        auto new_word = static_cast< ulong >( src.read_word_reverse() );
        if ( verbose )
        {
          std::printf( "[DEC] -> #w=%5lu - word read: %04lx\n", ++w, new_word );
        }
        if ( verbose )
        {
          std::printf( "[DEC] \t\t\t\t\t\t x: %lu -> ", x );
        }
        x = ( x << 16ul ) + new_word;
        if ( verbose )
        {
          std::printf( "%lu\n", x );
        }
      }
    }


    while ( x > 0 )
    {
      auto s = ft.symbol( static_cast< word >( x & mask ) );
      dst.write_symbol_reverse( s );
      if ( verbose )
      {
        std::printf( "[DEC] <- #s=%5lu - symbol written: %02x\n", ++i, s );
      }
      if ( verbose )
      {
        std::printf( "[DEC] \t\t\t\t\t\t x: %lu -> ", x );
      }
      x = ( ft.f( s ) * ( x >> _NumBase ) ) + ( x & mask ) - ft.cdf( s );
      if ( verbose )
      {
        std::printf( "%lu\n", x );
      }
    }


    if ( verbose )
    {
      std::printf(
        "[DEC] Decoding completed. Decoded %lu symbols from %lu bytes.\n\n", i,
        w * 2 );
    }

    // ---------------

    // end decoding
    auto decoding_time = std::chrono::duration_cast< std::chrono::nanoseconds >(
      std::chrono::high_resolution_clock::now() - start_time );

    return decoding_time;
  }


}  // namespace coding::rans

#endif  // !CODING_COMPR_STATS_H_INCLUDED
