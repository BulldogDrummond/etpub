#!/usr/bin/perl
open(T, ">".$ARGV[0].".tmp");
open(F, $ARGV[0]);
while($line = <F>) {
	$line =~ s/\r//g;
	print T $line;
}
close(F);
rename($ARGV[0].".tmp", $ARGV[0]);
