#!/usr/local/bin/perl
#
# Test image filter.
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1, print "1..44\n"; }
END {print "not ok 1\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't' || die 'Cd failed';

testFilter('input.miff', 'Blur',
  q/0.0x1.5/,
  '7b5e0779098aa3ab22d4272bbfab8e1e',
  '6046acf8622ed1bdf5628416cf26ea88');

++$test;
testFilter('input.miff', 'Border',
  q/10/,
  '9a29118485648b8d1af4190348954103',
  'd786ed89a0fad7a401e200e57629faa8');

++$test;
testFilter('input.miff', 'Chop',
  q/+10+10/,
  'd29da2e9202ebfbaceb18ebd29e47132');

++$test;
testFilter('input.miff', 'ColorFloodfill',
  q/+25+45/,
  '09c82f6759682ee2adbd09571edfb34f',
  '12643c99ecf96cf1d7e8f5cc62b13c78');

++$test;
testFilter('input.miff', 'Colorize',
  q/red/,
  '62907fa59892a6241e81fa10cd513ef2',
  '2ffd120c34e437df526227b1dcd83f7d');

++$test;
testFilter('input.miff', 'Contrast',
  q/1/,
  'c2156d2134de57d98a03fe8ffdf0a3cd',
  'e4d90d8d2af844650864c4e193acb270');

++$test;
testFilter('input.miff', 'Convolve',
  [1, 2, 1, 2, 4, 2, 1, 2, 1],
  'eade44acbbda7dc3155778286e77f954',
  '78da2d7c584ffd74b41137ea23460abb');

++$test;
testFilter('input.miff', 'Crop',
  q/10x10/,
  'd29da2e9202ebfbaceb18ebd29e47132');

++$test;
testFilter('input.miff', 'Despeckle',
  q/ /,
  '79b2523ae5fb8d6c3219efedfc5c97f2',
  'f4e6ed6a3dd1696e78fc69a761802f01');

++$test;
testFilter('input.miff', 'Edge',
  q/3/,
  '29c6f3e24fb5dbae9468efd7b4b391ae',
  '89457006d11b4d9ea348d90f2fa75ec7');

++$test;
testFilter('input.miff', 'Emboss',
  q/3/,
  '4a84d8bcd2c7c494cc913a845caf7b9a',
  'e8e5eb03ca6563f34db3692747a8d9f8');

++$test;
testFilter('input.miff', 'Equalize',
  q/ /,
  'e7c25176f93a087008197e82104ab3cb',
  '4731d138fa4bf61a12911686b3bebf85');

++$test;
testFilter('input.miff', 'Flip',
  q/ /,
  '5343a506b26bc6af15c78f1dea9cfdbd');

++$test;
testFilter('input.miff', 'Flop',
  q/ /,
  '4770e370fb842b770122a16a789f9bbb');

++$test;
testFilter('input.miff', 'Frame',
  q/10x10/,
  '026f10a0cd41b8b61e31b27f3adb6d71', 
  '1c36dcdff27bd8618598def99f5227d0');

++$test;
testFilter('input.miff', 'Gamma',
  q/2.2/,
  '23a6358f8dfb340e7daf90c2ac991321',
  'e2570162c1ee89f182107c21d5aa0bbd');

++$test;
testFilter('input.miff', 'Implode',
  q/30/,
  '7bc7519752848f31eb93d6aa78e193d3',
  'c64ccc30e9de0804219b13c65af7697c');

++$test;
testFilter('input.miff', 'Layer',
  q/Red/,
  '855e49d855e2dffc8abdbb78014575e0');

++$test;
testFilter('input.miff', 'Magnify',
  q/ /,
  '066a93e6613bcb143da9eed016ac8121',
  'ad774b9ad972dbc4dca6193ff49f51a3');

++$test;
testFilter('input.miff', 'MatteFloodfill',
  q/+25+45/,
  '875628edd6e4c05889ddfb279d05dcd0',
  '6f55ec5c0a6e1472ea54c4320d3a9030');

++$test;
testFilter('input.miff', 'Minify',
  q/ /,
  '39610223a220856eee90340cfc08ef80',
  '3a7a5363b30f4c493238992a4b6ecfde');

++$test;
testFilter('input.miff', 'Modulate',
  q/60/,
  '1c4736b6d1ed28e1846ab1f47c792ef4',
  'cfc9efc540fe048eefd92ce743c0630e');

++$test;
testFilter('input.miff', 'Negate',
  q/0/,
  '7c1ca362ad2afc2eb11d61bc08e48026');

++$test;
testFilter('input.miff', 'Normalize',
  q/ /,
  '3874fb4ca728b2e4bdf020a6b00d47b0',
  '72cee3dd1622629f43093cb02fcdac43');

++$test;
testFilter('input.miff', 'OilPaint',
  q/3/,
  'a70161481478c896def77f29b0f8f997',
  'e16d40c46206f6c3dc682139de76f76d');

++$test;
testFilter('input.miff', 'Opaque',
  q/#fefefd/,
  'eb4d6e084afe2835a1ad28ad7fc12ced');

++$test;
testFilter('input.miff', 'Quantize',
  q/128/,
  '8ff5790c89425da8ddca2647526b7ab9',
  'd51eec15134fee4bfcdb732b6d706a17');

++$test;
testFilter('input.miff', 'Raise',
  q/10x10/,
  '0cbe112e62a8e8a6e178a55a56b69a4b',
  '1bc9ae340eef6cc6257b0e2bfb5a0ab0');

++$test;
testFilter('input.miff', 'Roll',
  q/+10+10/,
  '5b5165d9bfda88340e0095ca4ac190dc');

++$test;
testFilter('input.miff', 'Rotate',
  q/10/,
  'ae4c2b922aec1130999e6f9668b1f080',
  'd5141f9e3a246be7bbd3b15e38602e15');

++$test;
testFilter('input.miff', 'Sample',
  q/50%/,
  '602eb9131754c2467f87a8a4ddd21cb7');

++$test;
testFilter('input.miff', 'Scale',
  q/50%/,
  'fd9f86e0fb31bc2fd67aa43084e64e24',
  'f37f5d4ca4ac9e243cedb3abf35758ac');

++$test;
testFilter('input.miff', 'Segment',
  q/1/,
  '441f25c2c60c224061352f546b6ddff3');

++$test;
testFilter('input.miff', 'Shade',
  q/30/,
  '2096afda6790e7e8a8abf31665a78d0f',
  '06417bec58f55fe1339dbec94da181d3');

++$test;
testFilter('input.miff', 'Sharpen',
  q/0.0x1.5/,
  'bef1ddd9a7f66b5e3c0c66f4ca63ca22',
  'd2a0063811738be757a6d427499a6187');

++$test;
testFilter('input.miff', 'Shear',
  q/10/,
  '640dffd1d8ad695bde747a76f79c8312',
  '97dc6a82f0fd80f7d82c5f6fe0fecc73');

++$test;
testFilter('input.miff', 'Solarize',
  q/60/,
  '2c67db3c251365114d229d5c0a36362b');

++$test;
testFilter('input.miff', 'Swirl',
  q/60/,
  'b98790b16435c145c1cc68a1f1bbfc13',
  '41baeae1599d63ded7c7f60adcb43e7d');

++$test;
testFilter('input.miff', 'Texture',
  q/granite:/,
  'eb4d6e084afe2835a1ad28ad7fc12ced');

++$test;
testFilter('input.miff', 'Transform',
  q/50x60/,
  '3978d936c56107752e690c6107cafdbe');

++$test;
testFilter('input.miff', 'Threshold',
  q/128/,
  '32730cfe0d9939d04049cd90b2acdb2d',
  'caa7ebca666f176a8f4d4a2f012e2236');

++$test;
testFilter('input.miff', 'Trim',
  q/ /,
  'eb4d6e084afe2835a1ad28ad7fc12ced');

++$test;
testFilter('input.miff', 'Wave',
  q/25x157/,
  '8972b0a8441091394e0b91acc85a2192',
  '59444de9684faa8ef8a8287c2c17f7f3');

++$test;
testFilter('input.miff', 'Zoom',
  q/50%/,
  'b3a770742494edc995abc2b6e6a7faec',
  '9e78d6fcaf019530500dff7078a23efc');

++$test;

1;
