#include "data_block.h"
#include "num_freq_table_adapt.h"

#include <cstdio>

void num_freq_table_adapt_test()
{
  auto buf = coding::data_block< 1024, 2 >( "LICENSE" );

  std::printf( "NUMERAL SYSTEM ADAPTIVE FREQUENCY TABLE (rate=3) TEST:\n\n" );

  auto ft = coding::num_freq_table_adapt< 2, 12, 3 >( buf );
  ft.display();

  if ( ft != ft )
  {
    std::printf( "Identity error!" );
  }

  std::printf( "\n" );
}


int main( [[maybe_unused]] int argc, [[maybe_unused]] char const *argv[] )
{
  num_freq_table_adapt_test();

  return 0;
}
