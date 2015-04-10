#!/usr/bin/perl
# script to configure slabs
# author: amit vasudevan (amitvasudevan@acm.org)

use Tie::File;

chomp(my $rootdir = $ARGV[0]);
chomp(my $filename = $ARGV[1]);
tie my @array, 'Tie::File', $filename or die $!;

my $i = 0;

while( $i <= $#array) {
    my $line = $array[$i];
    chomp($line);

    my $trimline = $line;
    $trimline =~ s/^\s+|\s+$//g ;     # remove both leading and trailing whitespace

    # split the line using the comma delimiter
    my @slabinfo = split(/,/, $trimline);
    #print "Processing line: $line \n";

    my $slabdir = $slabinfo[0];
    $slabdir =~ s/^\s+|\s+$//g ;     # remove both leading and trailing whitespace
    my $slabtype = $slabinfo[1];
    $slabtype =~ s/^\s+|\s+$//g ;     # remove both leading and trailing whitespace


    print "Configuring slab: $rootdir/$slabdir with type:$slabtype ...\n";
    system "cd $rootdir/$slabdir && autoreconf --install && automake --add-missing --copy >/dev/null 2>&1";
    system "cd $rootdir/$slabdir && ./configure --with-slabtype=$slabtype";


    # move on to the next line
    $i = $i + 1;
}
