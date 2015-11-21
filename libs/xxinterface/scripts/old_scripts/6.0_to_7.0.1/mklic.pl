#!/usr/local/bin/perl
##########################################################################
# Copyright (c) 2015, Synopsys, Inc.                                     #
# All rights reserved.                                                   #
#                                                                        #
# Redistribution and use in source and binary forms, with or without     #
# modification, are permitted provided that the following conditions are #
# met:                                                                   #
#                                                                        #
# 1. Redistributions of source code must retain the above copyright      #
# notice, this list of conditions and the following disclaimer.          #
#                                                                        #
# 2. Redistributions in binary form must reproduce the above copyright   #
# notice, this list of conditions and the following disclaimer in the    #
# documentation and/or other materials provided with the distribution.   #
#                                                                        #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      #
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  #
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   #
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, #
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       #
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  #
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  #
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   #
##########################################################################
# mklicense.pl
# Drives the create_license script, validating input and providing
# a menu-driven system for generating DISCOVER licenses.

# 25.Dec.95 Kit Transue

# modified 21.Apr.98 Kit Transue
# use FLEXlm 6.0's meaningful strings for permanent and uncounted licenses
# use lmcrypt instead of makelicense for easier update
# 6.0 licensing is incremental:
# the format of the specification file: (ignore leading #)

#6.000	Developer
#6.000	Project_Leader Developer Engineer

# The extra features after Project_Leader means that a license issued for
# the first item will result in additional licenses issued for the others

# these are stored as lists in an assoc array:

sub load_features {
	local($filename) = @_;
	local($version, $feature, $subfeature_list);
	open(FEATURES, $filename) || die "Can't open $filename--stopped";
	while (<FEATURES>) {
		chop;
		($version, $feature, $subfeature_list) = split(/\s/, $_, 3);
		if ( $version =~ /[0-9.]+/ ) {
		    $features{$version} = $features{$version} . $feature . "\\";
		    $subfeatures{$version . $feature} = $subfeature_list;
		}
	}
}

sub add_licenses {
	local($feature, $quantity) = @_;
	$qty{$feature} += $quantity;
	foreach $x (split(/\s/, $subfeatures{$version . $feature})) {
		$qty{$x} += $quantity;
	}
}


sub get_quantity {
	local($feature) = @_;
	local($tmp);
	if ( $DEMO eq "y" ) {
		printf ("Create a demo license for %s? (y/n) [%s]: ", $feature, $default);
		$quantity = <STDIN>;
		chop $quantity;
		if ($quantity eq "") {
			$quantity = $default;
		}
		while ( $quantity ne "y" && $quantity ne "n" ) {
			printf ("Please enter y or n: ");
			$quantity = <STDIN>;
			chop $quantity;
		}
		$default = $quantity;
		if ( $quantity ne "n" ) {
			add_licenses($feature, 1);
		}
	}
	else {
		printf ("Number of licenses for %s [%d]: ", $feature, $default);
		$quantity = <STDIN>;
		chop $quantity;
		if ($quantity eq "") {
			add_licenses($feature, $default);
		}
		else {
			add_licenses($feature, $quantity);
			$default = $quantity;
		}
	}
}

sub get_company {
	print "Enter company or DEMO for demo: ";
	$company = <STDIN>;
	chop $company;
	if ($company eq "DEMO") {
		$DEMO = "y";
		$company = "demo";
	}
}

sub get_hostname {
	if ( $DEMO eq "y" ) {
		print "Enter demo reference (company name): [demo] ";
		$hostname = <STDIN>;
		chop $hostname;
		if ($hostname eq "") {
			$hostname = "demo";
		}
	}
	else {
		print "Enter hostname: ";
		$hostname = <STDIN>;
		chop $hostname;
	}
}

sub get_hostid {
	print "Enter hostid: ";
	$hostid = <STDIN>;
	chop $hostid;
}

sub get_expiration {
	local ($year, $month, $day, $truncyear, $valid);
	$valid = 0;
	while ( $valid == 0 ) {
		$valid = 1;
		print "Enter expiration date (01-jan-96) or (01-jan-1996) or CR for permanent: ";
		$expiration = <STDIN>;
		chop $expiration;
		if ($expiration eq "") {
			$expiration = "permanent";
			if ( $DEMO eq "y" ) {
				$valid = 0;
				print "Cannot issue permanant demo license!\n";
			}
		} else {
			# make sure we output in four digit year for Y2K
			if ($expiration =~ m/^([0-9]+)-([a-z]+)-([0-9]+)$/) {
				$truncyear = $3;
				$month = $2;
				$day = $1;
				if ($truncyear > 96 && $truncyear < 100) {
					$year = $truncyear + 1900;
				} elsif ($truncyear < 96) {
					$year = $truncyear + 2000;
				} else {
					$year = $truncyear;
				}

				# do some validation:
				# make sure month is three characters:
				if ($month !~ m/^...$/) {
					$valid = 0;
					print "Month must be three-character abbreviation!\n";
				}
				$expiration = $day . "-" . $month ."-" . $year;
			}
		}
	}
}

sub get_start {
	local(@months) = ("jan", "feb", "mar", "apr", "may", "jun",
		"jul", "aug", "sep", "oct", "nov", "dec");
	local($sec, $min, $hour, $mday, $mon, $year) = localtime(time);
	$start = $mday . "-" . $months[$mon] . "-" . $year;
}

sub write_prologue {
	# company:
	#print CRYPT $company;
	#print CRYPT "\n";

	# use existing file:
	#print CRYPT "n\n";

	# hostname:
	if ( $DEMO ne "y" ) {
		print CRYPT "SERVER ";
		print CRYPT $hostname;
		print CRYPT " ";
		print CRYPT $hostid;
		print CRYPT " ";
		# port:
		print CRYPT "7572";
		print CRYPT "\n";
	}

	# daemon:
	if ( $DEMO ne "y" ) {
		print CRYPT "VENDOR ";
		print CRYPT "setechd";
		print CRYPT "\n";
	}
}

sub write_feature {
    if ( $feature !~ /SET[.]/ ) {
	print CRYPT "INCREMENT ";
	print CRYPT $feature;
	print CRYPT " ";

	# daemon:
	print CRYPT "setechd";
	print CRYPT " ";

	# version:
	print CRYPT $version;
	print CRYPT " ";

	# expiration:
	print CRYPT $expiration;
	print CRYPT " ";

	# quantity:
	if ( $DEMO eq "y" ) {
		print CRYPT "uncounted";
	}
	else {
		print CRYPT $qty{$feature};
	}
	print CRYPT " ";

	# code:
	print CRYPT "0";
	print CRYPT " ";

	# hostlocked:
	if ( $DEMO eq "y" ) {
		print CRYPT "HOSTID=DEMO";
	}

	print CRYPT "\n";
   }
}



# do we want to do anything else, like write copyright, conditions
# of transfer, etc?

# main():

# global variables:
# 	%qty
#	$default
#	%features
#	$DEMO

open (USERNAME, "whoami|");
$username = <USERNAME>;
chop $username;
close USERNAME;
print "Enter username: ($username) ";
$tmp = <STDIN>;
chop $tmp;
if ($tmp ne "") {
	$username = $tmp;
}

print "Generate file compatible with older (discover 4/5) license daemon? (y/n) [n]: ";
if (<STDIN> =~ /^[yY]/) {
	$compatibility = " -verfmt 5_1";
}

&load_features("features.list");
&get_start();

$version = "";
@versions = keys %features;
if ( $#versions == 1 ) {
    print "Please select a version:\n";
    foreach $x (sort (keys %features)) {
	print $x . "\n";
    }
    $version = <STDIN>;
    chop $version;
} else {
    $version = $versions[0];
}

&get_company();
&get_hostname();
if ( $DEMO ne "y" ) {
	&get_hostid;
}
&get_expiration;

@featlist = split (/\\/, $features{$version});
foreach $feature (@featlist) {
	&get_quantity($feature);
}

# while I redirect error, unfortunately lmcrypt also writes error to file!
open (CRYPT, "|lmcrypt $compatibility > license.tmp 2> license.err");

&write_prologue;

print CRYPT "\n";

foreach $feature (@featlist) {
	if ($qty{$feature} != 0) {
		&write_feature($feature);
		delete $qty{$feature};
	}
}
# pick up any licenses generated as by-products of inclusion but not in featlist
foreach $feature (keys %qty) {
	if ($qty{$feature} != 0) {
		&write_feature($feature);
	}
}

# close:
print CRYPT "\n";

close (CRYPT);

# Cleanup: remove errors and append packages:

# figure out name for this file:
$newfilename = "$company.$hostname.issued-$start.expires-$expiration.$username";
if (-e $newfilename)  {
	for ($x = 1; -e "$newfilename.$x" && $x < 10; $x++) {};
	if ($x == 10) {
		die "too many licenses generated for company/host today!";
	}
	$newfilename = "$newfilename.$x";
}
open (TMPLICENSE, "license.tmp");
open (LICENSE, ">$newfilename");
$err_state = 1;
while (<TMPLICENSE>) {
	if (/(^SERVER)|(^INCREMENT)|(^FEATURE)/) {
		$err_state = 0;
	}
	if ($err_state != 1) {
		print LICENSE;
	}
}
close TMPLICENSE;

close LICENSE;

print "wrote license file $newfilename\n";
