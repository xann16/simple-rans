#include "bit_buffer.h"
#include "compr_stats.h"
#include "data_block.h"
#include "freq_table.h"
#include "num_freq_table.h"

#include <cstdio>

void bit_buffer_test()
{
  auto buf = coding::bit_buffer< 1024 >{};

  coding::word words[] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u };
  std::size_t wc = 10u;

  std::printf( "BIT BUFFER TEST:\n\n" );

  for ( std::size_t i = 0; i < wc; ++i )
  {
    buf.write_word( words[ i ] );
    std::printf( "W%2lu: Writing '%04x' to buffer. Current length: %2lu.\n",
                 i + 1, words[ i ], buf.size() );
  }

  buf.rewind();
  std::printf( "\n" );

  for ( std::size_t i = 0; i < wc; ++i )
  {
    auto res = buf.read_word();
    std::printf( "R%2lu: [%s] Reading '%04x' to buffer. Expected: %04x.\n",
                 i + 1, ( res == words[ i ] ) ? " OK " : "FAIL", res,
                 words[ i ] );
  }

  std::printf( "\n" );
}

void data_block_manual_test()
{
  auto buf1 = coding::data_block< 256, 1 >();
  auto buf2 = coding::data_block< 256, 2 >();
  auto buf4 = coding::data_block< 256, 4 >();
  auto buf8 = coding::data_block< 256, 8 >();

  coding::byte vals[] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 15u, 16u, 255u };
  std::size_t val_count = 12;

  for ( std::size_t i = 0u; i < val_count; ++i )
  {
    buf1.write_symbol( vals[ i ] );
    buf2.write_symbol( vals[ i ] );
    buf4.write_symbol( vals[ i ] );
    buf8.write_symbol( vals[ i ] );
  }

  std::printf( "DATA BLOCK BASIC TEST:\n\n" );
  std::printf( "Stats after adding 12 symbols:\n" );
  std::printf( "*----------------------------------------------*\n" );
  std::printf( "| SL |    maximal size    |    current size    |\n" );
  std::printf( "|    | byte | bit  | sym  | byte | bit  | sym  |\n" );
  std::printf( "|----+------+------+------+------+------+------|\n" );
  std::printf( "| %2lu | %4lu | %4lu | %4lu | %4lu | %4lu | %4lu |\n",
               buf1.symbol_length(), buf1.max_size(), buf1.max_bit_count(),
               buf1.max_symbol_count(), buf1.size(), buf1.bit_count(),
               buf1.symbol_count() );
  std::printf( "| %2lu | %4lu | %4lu | %4lu | %4lu | %4lu | %4lu |\n",
               buf2.symbol_length(), buf2.max_size(), buf2.max_bit_count(),
               buf2.max_symbol_count(), buf2.size(), buf2.bit_count(),
               buf2.symbol_count() );
  std::printf( "| %2lu | %4lu | %4lu | %4lu | %4lu | %4lu | %4lu |\n",
               buf4.symbol_length(), buf4.max_size(), buf4.max_bit_count(),
               buf4.max_symbol_count(), buf4.size(), buf4.bit_count(),
               buf4.symbol_count() );
  std::printf( "| %2lu | %4lu | %4lu | %4lu | %4lu | %4lu | %4lu |\n",
               buf8.symbol_length(), buf8.max_size(), buf8.max_bit_count(),
               buf8.max_symbol_count(), buf8.size(), buf8.bit_count(),
               buf8.symbol_count() );
  std::printf( "*----------------------------------------------*\n" );

  buf1.rewind();
  buf2.rewind();
  buf4.rewind();
  buf8.rewind();

  std::printf( "\nSymbols read:\n" );
  std::printf( "FULL SL=1 SL=2 SL=4 SL=8\n" );
  for ( std::size_t i = 0u; i < val_count; ++i )
  {
    std::printf( " %02x    %x    %x    %x   %02x \n", vals[ i ],
                 buf1.read_symbol(), buf2.read_symbol(), buf4.read_symbol(),
                 buf8.read_symbol() );
  }

  std::printf( "\n" );
}

void data_block_file_test()
{
  auto smbuf = coding::data_block< 128, 8 >( "LICENSE" );
  auto bigbuf = coding::data_block< 2048, 8 >( "LICENSE" );

  std::printf( "DATA BLOCK FILE TEST:\n\n" );

  printf( "Block sizes - small: %3lu/128; big: %4lu/2K.\n", smbuf.size(),
          bigbuf.size() );

  std::printf( "\nLicense in small buffer:\n" );
  std::printf( "-------------------------------\n" );
  for ( std::size_t i = 0u; i < smbuf.size(); ++i )
  {
    std::printf( "%c", smbuf.read_symbol() );
  }

  std::printf( "\n" );

  std::printf( "\nLicense in big buffer:\n" );
  std::printf( "-------------------------------\n" );
  for ( std::size_t i = 0u; i < bigbuf.size(); ++i )
  {
    std::printf( "%c", bigbuf.read_symbol() );
  }

  std::printf( "\n" );
}

void freq_table_test()
{
  auto buf = coding::data_block< 1024, 4 >( "LICENSE" );

  std::printf( "FREQUENCY TABLE TEST:\n\n" );

  auto ft = coding::freq_table< 4 >( buf );
  ft.display();

  if ( ft != ft )
  {
    std::printf( "Identity error!" );
  }

  std::printf( "\nSymbol fetching test:\n C: " );
  for ( std::size_t i = 0; i < 11; ++i )
  {
    std::printf( " %3.1f ", static_cast< double >( i ) * 0.1 );
  }
  std::printf( "\n S: " );
  for ( std::size_t i = 0; i < 11; ++i )
  {
    std::printf( "  %02x ", ft.symbol( static_cast< double >( i ) * 0.1 ) );
  }
  std::printf( "\n\n" );

  auto bb = coding::bit_buffer< 128 >();
  ft.write_header( bb );
  std::printf( "\nHeader size in buffer: %lu.\n", bb.size() );
  bb.rewind();
  auto rft = coding::freq_table< 4 >( bb );

  if ( ft != rft )
  {
    std::printf( "Read frequency table mismatch!" );
  }

  std::printf( "\n" );
}

void num_freq_table_test()
{
  auto buf = coding::data_block< 1024, 4 >( "LICENSE" );

  std::printf( "NUMERAL SYSTEM FREQUENCY TABLE TEST:\n\n" );

  auto ft = coding::num_freq_table< 4, 12 >( buf );
  ft.display();

  if ( ft != ft )
  {
    std::printf( "Identity error!" );
  }

  std::printf( "\nSymbol fetching test:\n C: " );
  for ( std::size_t i = 0; i < 16; ++i )
  {
    std::printf( " %8u ", static_cast< coding::word >( i ) << 8u );
  }
  std::printf( "\n S: " );
  for ( std::size_t i = 0; i < 16; ++i )
  {
    std::printf( "       %02x ",
                 ft.symbol( static_cast< coding::word >( i << 8u ) ) );
  }
  std::printf( "\n\n" );

  auto bb = coding::bit_buffer< 128 >();
  ft.write_header( bb );
  std::printf( "\nHeader size in buffer: %lu.\n", bb.size() );
  bb.rewind();
  auto rft = coding::num_freq_table< 4, 12 >( bb );

  if ( ft != rft )
  {
    std::printf( "Read frequency table mismatch!" );
  }

  std::printf( "\n" );
}

void compr_stats_basic_test()
{
  auto cs = coding::compr_stats< 8 >();

  std::printf( "COMPRESSION STATS BASIC TEST:\n\n" );

  cs.set_header_length( 32u );
  cs.set_symbol_count( 40000u );
  cs.set_raw_encoded_length( 234568u );
  cs.set_bits_per_symbol_theory( 3.212 );

  cs.set_encoding_time( std::chrono::nanoseconds{ 10u } );
  cs.set_decoding_time( std::chrono::nanoseconds{ 20u } );

  cs.display( "test" );

  std::printf( "\n" );
}


int main( [[maybe_unused]] int argc, [[maybe_unused]] char const *argv[] )
{
  bit_buffer_test();
  data_block_manual_test();
  data_block_file_test();
  freq_table_test();
  num_freq_table_test();
  compr_stats_basic_test();

  return 0;
}
