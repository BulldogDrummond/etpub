#!/usr/bin/perl
# etpub_stats.pl
# Takes a GUID and player name
# and returns a color-formatted
# print out of a player's ranks in
# kills and wins, adjusted by kill
# rating and player rating.
#
# by: Josh Menke
# updated: 6/10/2006

use strict;

my $xpsave_cfg = "/path/to/.etwolf/etpub/xpsave.cfg";

my $entry_type = "";
my %tmp_xpsave = ();
my $entry_type = "";
my %xpsave = ();
my $killrating = 1600;
my $playerrating = 0;
my $playerrating1 = 0;
my $playerrating2 = 0;
my $deviation = sqrt(2.0);
my $low_rate = 0.5;
my $pi = 3.14159;

my $guid = shift or die("No GUID.");
my $name = shift or die("No name.");

open( IN, $xpsave_cfg ) || die("Error opening file $xpsave_cfg: $!");
while (<IN>) {
    chomp;
    s/(^\s*|\s*\r*$)//g;

    next unless ( $_ || /\s*\#/ );
	if (/^\[(.*)\]$/) {
		$entry_type = $1;
		%tmp_xpsave = ();
	}

    if ( $entry_type eq "xpsave" ) {
        $tmp_xpsave{$1} = $2 if (/^(.*?)\s+=\s+(.*)/);

        if ( $tmp_xpsave{'name'} && $tmp_xpsave{'guid'} && $tmp_xpsave{'rating'} && $tmp_xpsave{'killrating'} && $tmp_xpsave{'rating_variance'} ) {
		$tmp_xpsave{'name'} =~ s/\^.//g;
		$tmp_xpsave{'lower_rating'} = 1.0 /
		(1.0+exp(-$tmp_xpsave{'rating'}/
			sqrt(1.0+3*20*$tmp_xpsave{'rating_variance'} / (3.14159*3.14159) ) ));
		%{$xpsave{ $tmp_xpsave{'guid'} }} = %tmp_xpsave;
		if ($tmp_xpsave{'guid'} eq $guid) {
			$killrating = $tmp_xpsave{'killrating'};
			$playerrating = $tmp_xpsave{'rating'};
			$deviation = sqrt($tmp_xpsave{'rating_variance'});
			$low_rate = $tmp_xpsave{'lower_rating'};
		}
		%tmp_xpsave = ();
        }
    }
}
close(IN);

my $prank = 1;
for my $player (sort( {$b->{lower_rating} <=> $a->{lower_rating};} values %xpsave)) {
	my $pguid = $$player{'guid'};
	last if ($pguid eq $guid);
	$prank++;
}

my $krank = 1;
for my $player (sort( {$b->{killrating} <=> $a->{killrating};} values %xpsave)) {
	my $pguid = $$player{'guid'};
	last if ($pguid eq $guid);
	$krank++;
}

if ($krank == 1) {
	$krank = "^1Top KILLA";
}

if ($prank == 1) {
	$prank = "^1Top PLAYA";
}

$playerrating1 = $playerrating - 2*$deviation;
$playerrating2 = $playerrating + 2*$deviation;
$playerrating1 = sprintf("%.3f",$playerrating1);
$playerrating2 = sprintf("%.3f",$playerrating2);
$killrating = 1.0 / (1.0+exp(-($killrating-1600)/400));
$killrating /= 1.0-$killrating;
$killrating = sprintf("%.3f",$killrating);
$low_rate = sprintf("%.3f",$low_rate);
$deviation = sprintf("%.3f",$deviation);
print "^f[ ^7$name^f ] [Rank:^3$prank^f] [Kill Rank:^3$krank^f]\n^f[PR Win %:^3$low_rate^f] [^fKR K/D:^3$killrating^f]";
