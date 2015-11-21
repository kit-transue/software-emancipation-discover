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
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

# ==========================================================================
# Scan a line from a build log,
# and if that line deals with the working directory, tag that line
# with one of the triggers ~cd~, ~pushd~, or ~popd~.
# All other lines are left unmodified.

# A line output with a tag of ~cd~ must begin with ~cd~ and the remainder
# of the line consists of a space and the name of the new working directory.

# ~pushd~ and ~popd~ track the working directory in case the build log
# has a pop command such that a stack is required for correct tracking.
# The ~pushd~ is followed by the name of a working directory,
# while the ~popd~ does not require one.

# TrackWorkingDirectory takes one argument (a build log line) and returns
# either the tagged line or the original, unmodified line.
#
sub FilterBuildLog::TrackWorkingDirectory {
    ($_) = @_;

    s@^Initial working directory was +@~cd~ @;		# Keep working directory information.
    s@^ *making +all +in +([^ ]*)\.\.\..*$@~cd~ $ENV{MainSrcRoot}/$1@;
    s@^\*+ +:directory +([^;]*[^; ]) +\*+$@~cd~ $1@;	# IM 18900
    s@^cd +"([^;]*[^\\;])"[^"]*$@~cd~ $1@;
    s@^cd +([^;]+).*@~cd~ $1@;
    if (m@^\s*cd @) {
	   &dismb_msg::wrn ("Unrecognized cd directive in build log: \$1", @_);
    }
    s@^[^\s]+:\s+Entering directory\s+[\"\'\`]([^\"\'\`]+)[\"\'\`].*@~pushd~ $1@;
    s@^[^\s]+:\s+Leaving directory.*$@~popd~@;

    return $_;
}

1;
