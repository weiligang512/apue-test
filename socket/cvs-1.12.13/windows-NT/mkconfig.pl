#! perl -w

use strict;

# For the `mv' function which is smart enough to cross device boundries.
use File::Copy qw{mv};
# For the `basename' function.
use File::Basename;



###
### FUNCTIONS
###
sub save_edit
{
    my ($same, $file_name, $temp_name) = @_;

    if ($same)
    {
	unlink $temp_name
	    or warn "Failed to unlink ", $temp_name, ": $!";
	print "no change: ", $file_name, "\n";
    }
    else
    {
	mv $temp_name, $file_name
	    or die "Failed to rename ", $temp_name, " to ", $file_name, ": $!";

	print "save edit: ", $file_name, "\n";
    }
}

sub get_default
{
    my ($value, $default) = @_;

    if ($value eq "")
    {
	$value = $default;
    }

    return $value;
}



sub show_repeat
{
    my ($file, $new_no, $old_no, $line) = @_;

    print $file, " line ", $new_no, " duplicates line ", $old_no, ": ", $line;
}



sub show_orphan
{
    my ($case, $that, $this, $this_key, %this_macros) = @_;
    my $type = $this_macros{$this_key}[1];

    if ($case eq 0)
    {
	# roots file has extra macro statement
	# tell only of #undef
	return if $type eq "d";
    }
    elsif ($case eq 1)
    {
	# build file has extra macro statement
	# tell only of #define
	return if $type eq "u";
    }
    else
    {
	die "Internal script error";
    }

    if ($type eq "d")
    {
	    $type = "#define";
    }
    elsif ($type eq "u")
    {
	    $type = "#undef";
    }
    else
    {
	die "Internal script error";
    }

    print $this, " line ", $this_macros{$this_key}[0], " has ", $type, " ",
	  $this_key, " not found in ", $that, "\n";
}



sub make_config_h
{
    my $quiet;
    if ($_[0] eq "-q")
    {
	$quiet = 1;
        shift;
    }

    my ($ph_name, $out_name, $inp_name, $end_name) = @_;

    $ph_name = get_default $ph_name, "../config.h.in";
    $out_name = get_default $out_name, "config.h.in";
    $inp_name = get_default $inp_name, $out_name . ".in";
    $end_name = get_default $end_name, $out_name . ".footer";

    print STDERR "($inp_name + $ph_name) . $end_name --> $out_name\n"
	if !$quiet;

    #==========================================================================
    # scan build level configuration to collect define/undef values
    #==========================================================================

    open FINP, "< $inp_name"
	or die "error opening ", $inp_name, " for read: $!";
    my %build_macros;
    while (<FINP>)
    {
	if (/^#\s*define\s*(\w+)(\s+(.+))?$/)
	{
	    if (exists $build_macros{$1})
	    {
		show_repeat $inp_name, $., $build_macros{$1}[0], $_;
	    }
	    else
	    {
		$build_macros{$1} = [$., "d", $3];
	    }
	}
	elsif (/^\s*#\s*undef\s+(\w+)/)
	{
	    if (exists $build_macros{$1})
	    {
		show_repeat $inp_name, $., $build_macros{$1}[0], $_;
	    }
	    else
	    {
		$build_macros{$1} = [$., "u"];
	    }
	}
    }
    close FINP;
    #==========================================================================

    #==========================================================================
    # temporary output file
    #==========================================================================
    my $temp_name = basename($out_name) . ".tmp";

    open FOUT, "> $temp_name"
	or die "error opening ", $temp_name, " for write: $!";

    #==========================================================================
    # copy build level configuration append file to output file
    #==========================================================================
    my $base_out = basename $out_name;
    my $base_prog = basename $0;
    my $base_inp = basename $inp_name;
    my $base_ph = basename $ph_name;
    my $base_end = basename $end_name;

    print FOUT <<EOF;
/***
 *** $base_out, generated by $base_prog:
 ***
 ***   ($base_inp
 ***    + ../$base_ph)
 ***   . $base_end
 ***   --> $base_out
 ***
 *** ***** DO NOT ALTER THIS FILE!!! *****
 ***
 *** Changes to this file will be overwritten by automatic script runs.
 *** Changes should be made to the $base_inp & $base_end
 *** files instead.
 ***/

EOF

    #==========================================================================
    # copy root level configuration to output file
    # while keeping track of conditional compile nesting level
    #==========================================================================
    open FINP, "< $ph_name"
	or die "error opening ", $ph_name, " for read: $!";
    my %ph_macros;
    while (<FINP>)
    {

	my $out_line = $_;

	if (/^\s*#\s*undef\s+(\w+)/)
	{
	    if (exists $ph_macros{$1})
	    {
		    show_repeat $ph_name, $., $ph_macros{$1}[0], $_;
	    }
	    else
	    {
		    $ph_macros{$1} = [$., "u"];
	    }

	    if (exists $build_macros{$1}
	        and $build_macros{$1}[1] eq "d")
	    {
		$out_line = "#define $1";

		$out_line .= " " . $build_macros{$1}[2]
		    if defined $build_macros{$1}[2];

		$out_line .= "\n";
	    }
	}
	print FOUT $out_line;
    }
    close FINP;
    #==========================================================================

    #==========================================================================
    # copy build level configuration append file to output file
    #==========================================================================
    if (open FINP, "< $end_name")
    {
	while (<FINP>)
	{
		print FOUT $_;
	}
	close FINP;
    }
    #==========================================================================
    close FOUT;
    #==========================================================================

    #==========================================================================
    # determine whether output (if any) has changed from last run
    #==========================================================================
    my $same = 0;

    if (open FINP, "< $out_name")
    {
	open FOUT, "< $temp_name"
	    or die "error opening ", $temp_name, " for read: $!";

	$same = 1;
	while ($same)
	{
	    last if eof FINP and eof FOUT;
	    if (eof FINP or eof FOUT or <FINP> ne <FOUT>)
	    {
		$same = 0;
		last;
	    }
	}
	close FOUT;
	close FINP;
    }

    #==========================================================================
    # nag the guilty
    #==========================================================================
    my @keys_build = sort keys %build_macros;
    my @keys_roots = sort keys %ph_macros;
    my ($idx_build, $idx_roots) = (0, 0);
    while ($idx_build < @keys_build or $idx_roots < @keys_roots) {
	if ($idx_build >= @keys_build)
	{
	    show_orphan 0, $inp_name, $ph_name, $keys_roots[$idx_roots],
	                %ph_macros;
	    $idx_roots++;
	}
	elsif ($idx_roots >= @keys_roots)
	{
	    show_orphan 1, $ph_name, $inp_name, $keys_build[$idx_build],
	                   %build_macros;
	    $idx_build++;
	}
	elsif ($keys_build[$idx_build] gt $keys_roots[$idx_roots])
	{
	    show_orphan 0, $inp_name, $ph_name, $keys_roots[$idx_roots],
	                %ph_macros;
	    $idx_roots++;
	}
	elsif ($keys_roots[$idx_roots] gt $keys_build[$idx_build])
	{
	    show_orphan 1, $ph_name, $inp_name, $keys_build[$idx_build],
	                %build_macros;
	    $idx_build++;
	}
	else
	{
	    $idx_build++;
	    $idx_roots++;
	}
    }

    #==========================================================================
    # save output only if changed
    #==========================================================================
    save_edit $same, $out_name, $temp_name;
}



###
### MAIN
###
make_config_h @ARGV;
