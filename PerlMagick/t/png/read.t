#!/usr/local/bin/perl
#
# Test reading PNG images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#

BEGIN { $| = 1; $test=1; print "1..6\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/png' || die 'Cd failed';

#
# 1) Test Black-and-white, bit_depth=1 PNG
# 
print( "1-bit grayscale PNG ...\n" );
testRead( 'input_bw.png',
  '26df8d4a408d695d4f7c720a39e88bb08ef1728aa2bd1d62134b09f8f0a53fe2' );

#
# 2) Test Monochrome PNG
# 
++$test;
print( "8-bit grayscale PNG ...\n" );
testRead( 'input_mono.png',
  '98688672bd9fa81731c8d442d24e655004d11ef17e6b36ceb70de5c01b683e81' );

#
# 3) Test 16-bit Portable Network Graphics
# 
++$test;
print( "16-bit grayscale PNG ...\n" );
testRead( 'input_16.png',
  '72b2c72d4e90436dd45adb77f992e1ed3c26e327a5ce5b068bd515b49039b6df',
  '07696b623faf33d67921b80b29ff905ff9d546c445a54026a32d263adbf5f6b9' );
#
# 4) Test 256 color pseudocolor PNG
# 
++$test;
print( "8-bit indexed-color PNG ...\n" );
testRead( 'input_256.png',
  '123c57a0f57f7be7ffcfb1bd428f568c23f939f5812112a986ffbbc19a9f6c0f' );

#
# 5) Test TrueColor PNG
# 
++$test;
print( "24-bit Truecolor PNG ...\n" );
testRead( 'input_truecolor.png',
  '5b48d510d17bd4c68b875a9689e8ea17caacbe7f3d566c162016e24f2ef875fd' );

#
# 6) Test Multiple-image Network Graphics
# 
++$test;
print( "MNG with 24-bit Truecolor PNGs...\n" );
testRead( 'input.mng',
  'f1467cb3a1ed311e5e8ddf7b8bfde62353709991e2f1c7fc9a06db8d413ba336' );

