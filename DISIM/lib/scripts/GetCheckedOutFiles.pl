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

open(InFile,"cleartool lsprivate -s -co|");
#open(InFile,"find $ENV{privateSrcRoot} -print|");
while(<InFile>)										
{											
#========================================== translate paths ========================================================

	s@/mvfs/r2/chorus/@/mvfs/siofs/src/chorus/@g;
	s@/mvfs/r2/chorus_3.1.2/@/mvfs/siofs/src/chorus_3.1.2/@g;
	s@chorus/gen-files@gen-files@g;
	s@chorus.debug/gen-files@gen-files@g;
	s@solaris/gen-files@gen-files@g;
	s@solaris.debug/gen-files@gen-files@g;
	s@/vobs/mvfs/netmgr/netmgr@/mvfs/netmgr@g;
	s@/vobs/mvfs/network/network@/mvfs/network@g;
	s@/vobs/mvfs/opt/opt@/mvfs/opt@g;
	s@/vobs/mvfs/siofs/siofs@/mvfs/siofs@g;

	s@/vobs/network.bridge/frame_relay@/mvfs/network/src/frame_relay@g;
	s@/vobs/network.interface/interface@/mvfs/network/src/interface@g;
	s@/vobs/network.interface/IP_over_ATM@/mvfs/network/src/IP_over_ATM@g;
	s@/vobs/network.route.IP/IP/@/mvfs/network/src/route/IP/@g;
	s@/vobs/network.route.IPX/IPX/@/mvfs/network/src/route/IPX@g;
	s@/vobs/network.fpsm.mgr/mgr/@/mvfs/network/src/fpsm/mgr/@g;
	s@/vobs/network.fpsm.sim/sim/@/mvfs/network/src/fpsm/sim/@g;

	s@/vobs/r2/siofs.src.chorus_3.1.2/chorus_3.1.2@/mvfs/r2/chorus@g;
	s@/vobs/siofs/siofs.src.chorus_3.1.2/chorus_3.1.2@/mvfs/siofs/src/chorus@g;
	s@/vobs/stage.chorus/cool@/mvfs/r2/cool-v4.0/release@g;

	s@/vobs/[^\./]+/@/mvfs/@g;
	s@/vobs/([^\./]+)\.([^\./]+)\.[^/]+/@/mvfs/$1/$2/@g;
	s@/vobs/([^\./]+)\.[^/]+/@/mvfs/$1/@g;

	# From here on, any translations at the /mvfs level are O.K.
	s@/mvfs/r2/foundation/drivers@/mvfs/siofs/src/drivers@g;

	# Here are the translations for the siofs/drivers/Led area:
	s@/mvfs/siofs/src/drivers/Led/CES@/mvfs/siofs/src/drivers/Led/ces_IOM@g;
	s@/mvfs/siofs/src/drivers/Led/NCM2@/mvfs/siofs/src/drivers/Led/NCM@g;
	s@/mvfs/siofs/src/drivers/Led/ces@/mvfs/siofs/src/drivers/Led/ces_IOM@g;
	s@/mvfs/siofs/src/drivers/Led/ds3c@/mvfs/siofs/src/drivers/Led/ces_IOM@g;
	s@/mvfs/siofs/src/drivers/Led/fr2@/mvfs/siofs/src/drivers/Led/FR@g;
	s@/mvfs/siofs/src/drivers/Led/frame_relay@/mvfs/siofs/src/drivers/Led/FR@g;
	s@/mvfs/siofs/src/drivers/Led/ncm2@/mvfs/siofs/src/drivers/Led/NCM@g;
	s@/mvfs/siofs/src/drivers/Led/sonet_lite@/mvfs/siofs/src/drivers/Led/oc3@g;

	print "$_";									
}
close InFile;

