#!/usr/bin/perl

use strict; use warnings;

my $NCHI_START = 1000;
my $NCHI_END = 100000;
my $nchi_step = 5000;

sub calc {
   my $algorithm = shift;

   open(POU,">$algorithm.data") or die "Can't open $algorithm.data for writing: $!\n";
   for (my $nchi = $NCHI_START; $nchi < $NCHI_END; $nchi += $nchi_step) {
      my $res = `./chi_square $algorithm string $nchi 100`;
      if ($res =~ /(\d+\.\d+).*?(\d+\.\d+)/) {
         print POU $nchi . " " . $1 . " " . $2 . "\n";

      }
   }
   close POU;

}

#my @alg_array = ("torek", "goulburn", "phong", "hsieh", "jenkins2", "jenkins3", "sha1", "korzendorfer1");
my @alg_array = ("jenkins2", "jenkins3");

foreach my $alg (@alg_array) {
   calc($alg);
}

