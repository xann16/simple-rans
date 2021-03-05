#ifndef CODING_COMPR_STATS_H_INCLUDED
#define CODING_COMPR_STATS_H_INCLUDED

#include <chrono>
#include <cstdio>

#include "common.h"

namespace coding
{
  // table storing symbol frequencies for certain data block
  template < std::size_t SL >
  class compr_stats
  {
  public:
    uint symbol_length() const noexcept { return static_cast< uint >( SL ); }
    uint symbol_count() const noexcept { return m_symbol_count; }
    uint decoded_length() const noexcept
    {
      return symbol_length() * symbol_count();
    }

    uint raw_encoded_length() const noexcept { return m_raw_bit_count; }
    uint header_length() const noexcept { return m_header_bit_count; }
    uint encoded_length() const noexcept
    {
      return raw_encoded_length() + header_length();
    }

    double raw_compression_rate() const noexcept
    {
      return static_cast< double >( raw_encoded_length() ) /
             static_cast< double >( decoded_length() );
    }
    double compression_rate() const noexcept
    {
      return static_cast< double >( encoded_length() ) /
             static_cast< double >( decoded_length() );
    }

    double bits_per_symbol_theory() const noexcept
    {
      return m_bits_per_symbol_th;
    }
    double bits_per_symbol_raw() const noexcept
    {
      return static_cast< double >( raw_encoded_length() ) /
             static_cast< double >( symbol_count() );
    }
    double bits_per_symbol() const noexcept
    {
      return static_cast< double >( encoded_length() ) /
             static_cast< double >( symbol_count() );
    }
    double redundance_raw() const noexcept
    {
      return bits_per_symbol_raw() - bits_per_symbol_theory();
    }
    double redundance() const noexcept
    {
      return bits_per_symbol() - bits_per_symbol_theory();
    }

    std::chrono::nanoseconds encoding_time() const noexcept
    {
      return m_encoding_time_ns;
    }
    std::chrono::nanoseconds decoding_time() const noexcept
    {
      return m_decoding_time_ns;
    }

    void set_symbol_count( uint value ) { m_symbol_count = value; }
    void set_raw_encoded_length( uint value ) { m_raw_bit_count = value; }
    void set_encoded_length( uint value )
    {
      m_raw_bit_count = value - header_length();
    }
    void set_header_length( uint value ) { m_header_bit_count = value; }
    void set_bits_per_symbol_theory( double value )
    {
      m_bits_per_symbol_th = value;
    }
    void set_encoding_time( std::chrono::nanoseconds value )
    {
      m_encoding_time_ns = value;
    }
    void set_decoding_time( std::chrono::nanoseconds value )
    {
      m_decoding_time_ns = value;
    }

    void display( char const *encoder_name )
    {
      std::printf( "ENCODER STATS FOR '%s':\n", encoder_name );
      std::printf( " - symbol length:  %12u bits\n", symbol_length() );
      std::printf( " - symbol count:   %12u\n", symbol_count() );
      std::printf( " - header length:  %12u bits\n\n", header_length() );

      std::printf( " - decoded length: %12u bits\n", decoded_length() );
      std::printf( " - encoded length: %12u bits    (w/o header: %u bits)\n",
                   encoded_length(), raw_encoded_length() );
      std::printf( " - compression_rate: %13.2f%%     (w/o header: %.2f%%)\n\n",
                   compression_rate() * 100.0, raw_compression_rate() * 100.0 );

      std::printf( " - bits/symbol (theory): %11.4f\n",
                   bits_per_symbol_theory() );
      std::printf( " - bits/symbol (actual): %11.4f    (w/o header: %.4f)\n",
                   bits_per_symbol(), bits_per_symbol_raw() );
      std::printf( " - redundance:           %11.4f    (w/o header: %.4f)\n\n",
                   redundance(), redundance_raw() );

      std::printf( " - encoding time:  %12li ns\n", encoding_time().count() );
      std::printf( " - decoding time:  %12li ns\n", decoding_time().count() );
    }

  private:
    uint m_symbol_count = 0u;
    uint m_raw_bit_count = 0u;
    uint m_header_bit_count = 0u;
    double m_bits_per_symbol_th = 0.0;
    std::chrono::nanoseconds m_encoding_time_ns = {};
    std::chrono::nanoseconds m_decoding_time_ns = {};
  };

}  // namespace coding

#endif  // !CODING_COMPR_STATS_H_INCLUDED
