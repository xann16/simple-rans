// #include "bit_buffer.h"

#include <cstdio>

#include "num_freq_table.h"
#include "num_freq_table_alias.h"

void alias_test()
{
  auto buf = coding::data_block< 1024, 4 >( "LICENSE" );

  std::printf( "NUMERAL SYSTEM FREQUENCY TABLE WITH ALIAS TEST:\n\n" );

  auto ft = coding::num_freq_table_alias< 4, 12 >( buf );
  buf.rewind();
  auto ft_std = coding::num_freq_table< 4, 12 >( buf );
  ft.display();

  if ( ft != ft )
  {
    std::printf( "Identity error!" );
  }

  std::printf( "\nSymbol fetching test:\n         C: " );
  for ( std::size_t i = 0; i < 16; ++i )
  {
    std::printf( " %8u ", static_cast< coding::word >( i ) << 8u );
  }
  std::printf( "\n (alias) S: " );
  for ( std::size_t i = 0; i < 16; ++i )
  {
    std::printf( "       %02x ",
                 ft.symbol( static_cast< coding::word >( i << 8u ) ) );
  }
  std::printf( "\n   (std) S: " );
  for ( std::size_t i = 0; i < 16; ++i )
  {
    std::printf( "       %02x ",
                 ft_std.symbol( static_cast< coding::word >( i << 8u ) ) );
  }
  std::printf( "\n\n" );

  auto bb = coding::bit_buffer< 128 >();
  ft.write_header( bb );
  std::printf( "\nHeader size in buffer: %lu.\n", bb.size() );
  bb.rewind();
  auto rft = coding::num_freq_table_alias< 4, 12 >( bb );

  if ( ft != rft )
  {
    std::printf( "Read frequency table mismatch!" );
  }

  std::printf( "\n" );
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char const *argv[] )
{
  std::printf( "ALIAS METHOD TESTS:\n\n" );

  alias_test();

  std::printf( "\n" );

  return 0;
}
