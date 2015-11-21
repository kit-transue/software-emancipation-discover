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
require ("$ENV{PSETHOME}/mb/IMUtil.pl");

use Socket;
use Sys::Hostname;

#port for name_server
my($nameServer_port) = 28948;

sub PSET_Server_Info(@_)
{
  	my($service_name) = $_[0];
 	my($bytes_sent, $command, $res, $iaddr, $paddr);
  	my($srvr_port, $srvr_addr) = (0, 0);
 
  	my($proto) = getprotobyname("tcp");
  	socket(NMSERV_SOCKET, PF_INET, SOCK_STREAM, $proto) || &dismb_msg::die("Can't create socket: \$1", $!);
 
  	$iaddr = inet_aton(hostname());
  	$paddr = sockaddr_in($nameServer_port, $iaddr);
  	connect(NMSERV_SOCKET, $paddr) || &dismb_msg::die("Can't connect: \$1", $!);
 
  	#command = FIND_SERVER = 2
  	$command = pack('C', 2);
  	($bytes_sent = send NMSERV_SOCKET, $command, 0) || &dismb_msg::die("Can't send: \$1", $!);
 
  	$service_len = length($service_name);
 
  	$command = pack('I', $service_len);
  	($bytes_sent = send NMSERV_SOCKET, $command, 0) || &dismb_msg::die("Can't send: \$1", $!);
 
  	$command = pack("a$service_len", $service_name);
  	($bytes_sent = send NMSERV_SOCKET, $command, 0) || &dismb_msg::die("Can't send: \$1", $!);
 
  	recv(NMSERV_SOCKET, $res, 1, 0) || &dismb_msg::die("Can't receive: \$1", $!);
  	$res = unpack('C', $res);
  	if ( $res > 0 ) {
    		recv(NMSERV_SOCKET, $res, 4, 0) || &dismb_msg::die("Can't receive: \$1", $!);
    		$srvr_port = unpack('I', $res);
    		recv(NMSERV_SOCKET, $res, 4, 0) || &dismb_msg::die("Can't receive: \$1", $!);
    		$srvr_addr = unpack('I', $res);
  	}
  	close (NMSERV_SOCKET);
 
  	return ($srvr_port, $srvr_addr);
}

sub StartPSETServer(@_)
{
	$ENV{SERVICE_NAME}=$_[0];
	my($ServerPort)=0;
	my($ServerAddress)=0;

	($ServerPort, $ServerAddress) = PSET_Server_Info($ENV{SERVICE_NAME});
	
	while($ServerPort==0)
	{
		sleep(1);
		($ServerPort, $ServerAddress) = PSET_Server_Info($ENV{SERVICE_NAME});		
	}
	
}

sub WaitForPSETServerToStop(@_)
{
	$ENV{SERVICE_NAME}=$_[0];
	my($ServerPort)=0;
	my($ServerAddress)=0;

	($ServerPort, $ServerAddress) = PSET_Server_Info($ENV{SERVICE_NAME});
	print_verbose("Server port is \$1", $ServerPort);
	
	while($ServerPort!=0 && $ServerPort!=-1)
	{
		sleep(1);
		($ServerPort, $ServerAddress) = PSET_Server_Info($ENV{SERVICE_NAME});		
		print_verbose("Server port is \$1", $ServerPort);
	}
	
}

sub StopPSETServer(@_)
{
  	my($service_name)=$_[0];
  	my($command) = "stop_server -y";
  	my($buf, $bytes_sent, $cmd_len);
 
  	#add length of message to its beginning
  	$cmd_len = length($command);
  	$cmd_len++;
 
  	my($saddr);
  	my($port, $addr) = PSET_Server_Info($service_name);
  	my($proto) = getprotobyname("tcp");
  	socket(SSOCKET, PF_INET, SOCK_STREAM, $proto) || &dismb_msg::die("Can't create socket: \$1", $!);
  	$saddr = sockaddr_in($port, inet_aton($addr));
  	connect(SSOCKET, $saddr) || &dismb_msg::die("Can't connect: \$1", $!);
 
  	#$buf = "packet number" = 0;
  	$buf = pack('C', 0);
  	($bytes_sent = send SSOCKET, $buf, 0) || &dismb_msg::die("Can't send: \$1", $!);
 
  	#$buf = DATA_FINAL_PACKET = 2
  	$buf = pack('C', 2);
  	($bytes_sent += send SSOCKET, $buf, 0) || &dismb_msg::die("Can't send: \$1", $!);
 
  	#$buf = length of the command that will follow
  	$buf = pack('I', $cmd_len);
  	($bytes_sent += send SSOCKET, $buf, 0) || &dismb_msg::die("Can't send: \$1", $!);
 
  	#$buf = command/message itself
  	$buf = pack("a$cmd_len", $command);
  	($bytes_sent += send SSOCKET, $buf, 0) || &dismb_msg::die("Can't send: \$1", $!);
 
  	close(SSOCKET);	
}

sub PSET_Service_Name {
	my($model, $version) = @_;
	my($prefix) = (defined($RepositoryID) ? $RepositoryID . ":" : "");
	return ("Discover:" . $prefix . $model . "_" . $version);
}

sub PrepareService {
	GetInstallationValues();
	CheckArguments("MODELNAME","VERSION");

	$ENV{SERVICE_NAME}=PSET_Service_Name($ENV{MODELNAME}, $ENV{VERSION});
	print_verbose("Service name is \$1", $ENV{SERVICE_NAME});
}

sub JustStopModel(@_) 
{
	print_verbose("Stopping server \$1...", $ENV{SERVICE_NAME});
	
	StopPSETServer($ENV{SERVICE_NAME});

	print_verbose("Waiting for server to stop...");
	WaitForPSETServerToStop($ENV{SERVICE_NAME});
	print_verbose("Server stopped.");
}

sub StopModel(@_) 
{
	PrepareService();
	JustStopModel();
}
   
sub JustStartModel(@_)
{
	$ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
	if(! -r $ENV{ADMINDIR})
	{
	    ShowUsageErrorForFile("StartBrowser.bat", "VERSION not specified or no models exist");
		    exit(1);
	}
	if(SetupEnvironment())
	{
		if(exists($ENV{SRC_ROOT}))
		{
			$ENV{sharedSrcRoot}=$ENV{SRC_ROOT};
		}

		if($ENV{privateModelRoot}=~m@^\s*$@) {
			ShowUsageError("Private model root should be specified.");
	 	} elsif (! -d "$ENV{privateModelRoot}") {
			print_verbose("Creating private model root directry...");
			use File::Path;
			if(! mkpath("$ENV{privateModelRoot}",0,0777)){ShowUsageError("Failed to make directory \$1.", $ENV{privateModelRoot});}
	  	}
	  	
		if(!chdir("$ENV{ADMINDIR}")){die("Could not change directory to $ENV{ADMINDIR}\n");}
		print_verbose("Changed directory to:    \$1", $ENV{ADMINDIR});
		$ENV{defaultPREFS}="prefs\\build.prefs";
		print_verbose("Setting up defaultPREFS: \$1", $ENV{defaultPREFS});
                
		my($PREFSArgument)="$ENV{ADMINDIR}/prefs/build.prefs";
		print_verbose("Getting PDFArgument...");
		my($PDFArgument)=GetPref($PREFSArgument, pdfFileUser);
		$PDFArgument=~s@\%ADMINDIR%@$ENV{ADMINDIR}@g;    
		print_verbose("PDFArgument is \$1", $PDFArgument);
		print_verbose("Getting HomeProj...");
		my($HomeProj)=GetPref($PREFSArgument, projectHome);
		print_verbose("HomeProj is \$1", $HomeProj);

		#Start nameserver here. If it is already running the executable
		#just quits with a "Nameserver already running." message.           
		my(@namesrv_cmd) = ("$ENV{PSETHOME}/mb/fork", &dismb_lib::arglist_to_fork_str("$ENV{PSETHOME}/bin/nameserv.exe"));
		print_verbose("Starting nameserver \"\$1\"", &dismb_lib::arglist_to_str(@namesrv_cmd));
		my($startcode) = &dismb_lib::system(@namesrv_cmd);

		my(@pset_cmd) = ("$ENV{PSETHOME}/mb/fork", &dismb_lib::arglist_to_fork_str("$ENV{PSETHOME}/bin/pset_server.exe", "-pdf", "$PDFArgument", "-prefs", "$PREFSArgument", "-server", "-server_name", $ENV{SERVICE_NAME}, "-home", $HomeProj));
		print_verbose("Starting pset_server \"\$1\"", &dismb_lib::arglist_to_str(@pset_cmd));
		my($startcode) = &dismb_lib::system(@pset_cmd);
		if($startcode==0)
		{
			print_verbose("Waiting for server to register...\n");
			StartPSETServer($ENV{SERVICE_NAME});

			print_verbose("Server started. Service name is \$1", $ENV{SERVICE_NAME});
  		} else {
			print_verbose("Server failed ($startcode).");
    		}
	}
}

sub StartModel(@_)
{
	PrepareService();
	JustStartModel();
}

sub ProcessArgList(@_)
{
	@SubroutineArgsList=();
	my($SubroutineName)="";
	my($e_found) = 0;
	foreach my $Argument (@_)
	{
		my($ArgName)=$Argument; $ArgName=~s@^(..).*$@$1@;
		my($ArgValue)=$Argument; $ArgValue=~s@^..(.*)$@$1@;
        
		if($ENV{HostType}=~m@^NT$@){$ArgValue=~s@\\@/@g}
        
		if($ArgName=~m@-e@)
		{
			$SubroutineName=$ArgValue;
			print_verbose("ARGUMENT \$1:ExecuteCommand=\$2", $ArgName, $SubroutineName);
			$e_found = 1;
		} elsif($ArgName eq "-m" and not $e_found)
		{
			last;
		} elsif($ArgName=~m@-V@)
		{
			$Verbosity="high";
			print_verbose("ARGUMENT \$1:Verbosity=\$2", $ArgName, $Verbosity);
		} else {
			push(@SubroutineArgsList,"$Argument");
			print_verbose("ARGUMENT \$1", $Argument);
		}
	}
	&dismb_msg::define_progress_task("administering pset_server");
	my($retval) = 1;
	if ($e_found) {
		print_verbose("Subroutine is \$1", $SubroutineName);
		if($SubroutineName ne "") {
			print_verbose("Running \$1...", $SubroutineName);
			$retval = &$SubroutineName(@SubroutineArgsList);
		}
	}
	else {
		PrepareService();
		my($mode) = $ENV{MODE};
		if($mode=~m@start@)	{JustStartModel();}
		if($mode=~m@stop@)	{JustStopModel();}
	}
	&dismb_msg::report_progress("administering pset_server");
	return $retval;
}

if ($0 =~ m@NTPSETServerAdmin[^/\\]*@) {
    ProcessArgList(@ARGV);
}
else {
    1;
}
