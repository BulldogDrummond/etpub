package ldd_check;

# Package constructor
sub new
{
  my $this = {};
  bless $this;

  return $this;
}

sub do_check {
	($path, $abort) = @_;
	print("ldd check on $path\n");
	$line = `ldd -r $path 2>&1 | grep 'undefined symbol'`;
	chop($line);
	if (length($line) > 0)
	{		
		# verbose it
		print("ldd -r $path\n");
		system("ldd -r $path 2>&1");
		# error out if asked for
		if ($abort == 1)
		{
			# don't use die, as it doesn't actually abort cons - we require a correct exit status code
			printf("Undefined symbols in $path, aborting\n");
			exit(1);
		}
	}
}

# Close package
1;
