#! /usr/bin/perl -w

# Copyright (C) 1998 Alexander Schroeder

# This program may be freely used, modified and distributed.  It may not be sold or used commercially without
# prior written permission from the author.

use strict;

my $UPDATE = 'Thu Jun 18 22:55:51 1998';

# Startup: help, command line parsing

my $usage = <<EOT;
MkLang -- kompiliert language.def und produziert language.h und language.c
Letzte Aenderung: $UPDATE

Eintraege in language.def haben die Form

[LANGUAGE]
language 1
language 2
...

[CODE_1]
text 1 in language 1
text 1 in language 2
...

[CODE_2]
text 1 in language 1
text 1 in language 2
...

The texts may contain wildcards: %sX and %dX, where X is a number from 0 to 9, 
denoting the order of the parameters within the text.  The parameters must be
given to the translate function in this order.

Moegliche Optionen:
    --verbose
    --debug

EOT

my $verbose = 0;
my $debug = 0;
my $lfile = "language.def";

# Command line parsing
while ($#ARGV >= 0 && $ARGV[0] =~ /^-/) 
{
    $_ = shift(@ARGV);
    if (/^-(-)?v(erbose)?$/)      { $verbose = 1; next; }
    if (/^-(-)?d(ebug)?$/)        { $debug = 1; $verbose = 1; next; }
    die $usage;
}

print "reading languages...\n" if $verbose;

open DEFINITION, $lfile;
my $languages = 0;
my %strings = ();
my $lines = 0;
$_ = 0;

do 
{
    $lines++;
    $_ = <DEFINITION>;
    die "$lfile:$lines:definition ended without [LANGUAGE] tag.\n" if eof();
}
until /^\[LANGUAGE\]$/;

while (<DEFINITION>)
{
    $lines++;
    last if /^$/;
    die "$lfile:$lines:language definition not ended on line $lines.\n" if /\[/;
    chop;
    die "$lfile:$lines:illegal character '$1' in \"$_\"\n" if /(\W)/;
    $languages++;
    $strings{"LANGUAGE"}{$languages} = $_;
    print "  language: ", $strings{"LANGUAGE"}{$languages}, "\n" if $verbose;
}
print "  $languages languages.\n" if $verbose;

print "reading texts...\n" if $verbose;

my $item;
my $language = $languages;

while (<DEFINITION>)
{
    $lines++;
    next if /^$|#/;
    if (/^\[(.*)\]$/)
    {
	die "$lfile:$lines:not enough definitions before line $lines.\n" if $language != $languages;
	$item = $1;
	$language = 0;
	print "item: $item\n" if $debug;
	die "$lfile:$lines:illegal character '$item' in $_\n" if $item =~ /(\W)/;
	die "$lfile:$lines:constant '$item' starts with 'ST_'\n" if $item =~ /^ST_/;
    }
    else
    {
	$language++;
	die "$lfile:$lines:too many definitions at line $lines.\n" if $language > $languages;
	chop;
	die "$lfile:$lines:illegal character '$1' in \"$_\"\n" if /([^][0-9A-Za-z_%:,;.\'\/\$\\\"!?() +-])/;
	my @subst_var;
	my $index;
	my $maxindex = -1;
	for ($index = 0; $index < 10; $index++)
	{
	    $subst_var[$index] = '';
	}
	while (/%(.)(.)/g)
	{
	    my $type = $1;
	    my $index = $2;
	    next if $type !~ /[ds]/;
	    die "$lfile:$lines:illegal index '$index' in \"$_\"\n" if !defined $index 
		or $index and $index !~ /\d/
		    or $index and $index < 0
			or $index and $index > 9
			    or $index and $subst_var[$index] ne '';
	    $maxindex = $index if $index > $maxindex;
	    $subst_var[$index] = $type;
	}
	if ($maxindex >= 0)
	{
	    for ($index = 0; $index <= $maxindex; $index++)
	    {
		die "$lfile:$lines:missing index '$index' in \"$_\"\n" if $subst_var[$index] eq '';
	    }
	}
	$strings{$item}{$language} = $_;
	print "  text: ", $strings{$item}{$language}, "\n" if $debug;
    }
}
die "$lfile:$lines:not enough definitions at the end of definition file.\n" if $language != $languages;

print "writing header...\n" if $verbose;

open HEADER, ">language.h";

print HEADER <<"EOT";
/* Automatically generated header file by mklang.pl */

#ifndef LANGUAGE_H
#define LANGUAGE_H

/* The first language defined ist the default.  */
#define DEFAULT_LANGUAGE	0

enum
  {
EOT

for $item (values %{$strings{"LANGUAGE"}})
{
    print HEADER "    L_", uc $item, ",\n";
}

print HEADER << "EOT";
    MAXLANGUAGES,
  };

enum
  {
EOT

for $item (keys %strings)
{
    print HEADER "    ST_", $item, ",\n";
}

print HEADER << "EOT";
    MAXSTRINGS,
  };

extern char *strings[MAXSTRINGS][MAXLANGUAGES];

#endif /* LANGUAGE_H */

EOT

open SOURCE, ">language.c";

print SOURCE <<"EOT";
/* Automatically generated source file by mklang.pl */

#include "language.h"

char *strings[MAXSTRINGS][MAXLANGUAGES] =
{
EOT

for $item (keys %strings)
{
    print "item: $item\n" if $debug;
    print SOURCE "    {";
    for $language (1..$languages)
    {
	print $item, $language if !$strings{$item}{$language};
	print SOURCE "\"$strings{$item}{$language}\", ";
    }
    print SOURCE "},\n";
}

print SOURCE << "EOT";
};

EOT










