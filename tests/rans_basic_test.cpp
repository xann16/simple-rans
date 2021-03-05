// #include "bit_buffer.h"

#include <cstdio>

#include "num_freq_table.h"
#include "num_freq_table_alias.h"
#include "rans.h"

template < std::size_t SL, std::size_t N, std::size_t NUM,
           template < std::size_t, std::size_t > class _FreqTable >
void rans_test( bool show_freq_table, bool verbose )
{
  std::printf( "==========================\n" );
  std::printf( "=== rANS TEST (SL = %lu) ===\n", SL );
  std::printf( "==========================\n\n" );

  // prep data blocks (in/out) and intermediate bit buffer
  auto data = coding::data_block< N, SL >( ".clang-tidy" );
  auto bits = coding::bit_buffer< 4 * 1024 >();
  auto dout = coding::data_block< N, SL >();
  dout.prepare_full();

  if ( show_freq_table )
  {
    auto ft = _FreqTable< SL, NUM >( data );
    std::printf( "FREQUENCY TABLE:\n" );
    ft.display();
    std::printf( "\n" );
    data.rewind();
  }

  // encoding & decoding
  auto stats = coding::rans::encode< NUM, _FreqTable >( data, bits, verbose );
  auto decoding_time =
    coding::rans::decode< NUM, _FreqTable >( bits, dout, verbose );
  stats.set_decoding_time( decoding_time );

  // data consistency check
  std::printf( "Data consistency check after decoding: %s.\n\n",
               ( data == dout ) ? "OK" : "FAILED" );
  if ( data != dout )
  {
    std::printf( "INPUT:\n\n" );
    data.print();
    std::printf( "\n\nOUTPUT:\n\n" );
    dout.print();
    std::printf( "\n\n" );
  }

  // encoding/decoding stats
  stats.display( "rANS" );

  std::printf( "\n\n" );
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char const *argv[] )
{
  std::printf( "rANS TESTS:\n\n" );

  auto show_freq_table = false;
  auto verbose = false;
  const std::size_t N = 2 * 1024;
  const std::size_t NUM = 12;

  rans_test< 1, 2, NUM, coding::num_freq_table >( true, true );

  rans_test< 1, N, NUM, coding::num_freq_table >( show_freq_table, verbose );
  rans_test< 2, N, NUM, coding::num_freq_table >( show_freq_table, verbose );
  rans_test< 4, N, NUM, coding::num_freq_table >( show_freq_table, verbose );
  rans_test< 8, N, NUM, coding::num_freq_table >( show_freq_table, verbose );

  std::printf( "\n\n" );



  // std::printf( "rANS (ALIAS) TESTS:\n\n" );

  // rans_test< 1, N, NUM, coding::num_freq_table_alias >(
  //  show_freq_table );  // , verbose );
  // rans_test< 2, N, NUM, coding::num_freq_table_alias >(
  //  show_freq_table );  //, verbose );
  // rans_test< 4, N, NUM, coding::num_freq_table_alias >(
  //  show_freq_table );  //, verbose );
  // rans_test< 8, N, NUM, coding::num_freq_table_alias >(
  //  show_freq_table );  //, verbose );

  std::printf( "\n" );


  return 0;
}
