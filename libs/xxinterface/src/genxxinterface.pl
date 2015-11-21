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

# genxxinterface.pl
# 15.Sep.98 Kit Transue
# generate xxinterface.C containing encrypted text for licenses.

# Notes: currently (FLEXlm 6.1), there is a global buffer for the strings, because FLEXlm
# keeps a pointer to the feature name for use in diagnostic messages.
# This way, we don't encrypt the error message before it's printed.
# But if this changes, we can change WRITESCRAMBLE to call fix just
# like WRITEUNSCRAMBLE does.  And we can make "l" local instead of file-static.

sub EncryptChar {
	local ($position) = @_;
	local ($charkey, $tmp);
	if ($position < $keylength) {
		$tmp = substr($key, $position, 1);
	} else {
		$tmp = $OverflowKey;
	}
	$charkey = unpack("H2", $tmp);
	return "0x${charkey}";
    }

sub BuildKey {
	local ($seed, $ok, $i);

	for ($i = 0; $i < $keylength; $i++) {
		# start from a random number, and make sure it's not the same as
		# any character in the strings for its position:
		$seed = int(rand(253));
		$ok = 0;
		while ( $ok == 0 ) {
			$ok = 1;
			$seed = ($seed % 253) + 1;
			foreach $feat (@features) {
				if ($i <= length($feat)
					&& ((ord(substr($feat, $i, 1)) ^ $seed) % 0xff) == 0
				) {
					$ok = 0;
				}
			}
		}
		$key = $key . pack(c, $seed);
	}

	# figure overflow key:

	# start from a random number, and make sure it's not in the list
	# of remaining characters:
	$seed = int(rand(253));
	$ok = 0;
	while ( $ok == 0 ) {
		$ok = 1;
		$seed = ($seed % 253) + 1;
		if (defined($RemainingChars{pack(c, $seed)} )) {
			$ok = 0;
		}
	}
	$OverflowKey = pack(c, $seed);

    }



sub RegisterString {
	# the idea here is to come up with a crypt/decrypt mechanism that
	# will accomodate all the license strings that might be requested.
	# No longer should we worry about "Don't put a "g" in the third
	# position"

	# maintain the length of the longest string to encrypt:

	local ($feature) = @_;
	local ($i);

	push(@features, $feature);
	if (length($feature) > $keylength) {
		if ($FIXEDKEYLENGTH == 1) {
			# add remaining characters 
			for ($i = $keylength; $i < length($feature); $i++) {
				# maintain assoc array of used characters:
				$RemainingChars{substr($feature, $i, 1)} = 1;
			}
		}
		else {
			$keylength = length($feature);
		}
	}
    }

sub WriteDecrypt {
	local ($i, $ec);
	print "static void unscramble(char *l)\n";
	print "{\n\tbool done = (l[0] == 0);\n";
	$i = 0;
	for ($i = 0; $i < $keylength; $i++) {
		$ec = EncryptChar($i);
		print("\tif (!done) { l[$i] ^= $ec; done = (l[");
		print($i + 1);
		print ("] == 0); }\n");
	}
	print ("\tfor (int i = $i; !done; done = (l[++i] == 0)) {\n");
	$ec = EncryptChar($keylength);
	print ("\t\t l[i] ^= $ec;\n\t}\n}\n");
}

sub EncryptString {
	local($string, $arrlen) = @_;
	local($i, $c, $ec);

        $retval =  "{";
	for ( $i = 0; $i < $arrlen; $i++ ) {
            if ( $i != 0 ) {
		$retval .= ",";
	    }
	    if ( $i < length($string) ) {
		$c = substr($string, $i, 1);
		if ($c eq "\\") {
		    $c = "\\\\";
		}
		$ec = EncryptChar($i);
		$retval .= " '${c}' ^ ${ec}";
	    } else {
		$retval .= " 0";
	    }
        }	
	$retval .= " }";
	return $retval
}

sub ScanForStrings {
	local($filename) = @_;
	local($string, $varname);
	open(FILE, $filename) || die ("Error opening $filename.\n");
	while (<FILE>) {
		if ( /SCRAMBLE/ ) {
			while ( /SCRAMBLE/ ) {
				($string, $length) = /SCRAMBLE[ \t]*\("([^"]*)"[ \t]*,[ \t]*([0-9]+)[ \t]*\)/;
	        		RegisterString($string);
				s/SCRAMBLE[ \t]*\("([^"]*)"[ \t]*,[ \t]*([0-9]+)[ \t]*\)//;
			}
		}	
	}
}

sub ReplaceStrings {
	local($filename) = @_;
	local($string, $varname);
	open(FILE, $filename) || die ("Error opening $filename.\n");
	while (<FILE>) {
		if ( /SCRAMBLE/ ) {
			while ( /SCRAMBLE/ ) {
				($string, $length) = /SCRAMBLE[ \t]*\("([^"]*)"[ \t]*,[ \t]*([0-9]+)[ \t]*\)/;
				s/SCRAMBLE[ \t]*\("([^"]*)"[ \t]*,[ \t]*([0-9]+)[ \t]*\)/EncryptString($string, $length)/e;
			}
			print;
		} elsif ( /#WRITEDECODERFUNCT/ ) {
			WriteDecrypt;
		} else {
			print;
		}
	}
}


sub Main {
	ScanForStrings("xxinterface.pseudo");
	BuildKey();
	ReplaceStrings("xxinterface.pseudo");
}

$FIXEDKEYLENGTH = 1;
$keylength = 4;
Main();
