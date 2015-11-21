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
#This script has the functionality to communicate with the name server to get 
#information about message/log viewer server. That information can then 
#be used to establish communication with message/log viewer server and
#send message to it.

#NOTE:
#Name server port number and commands to communicate with it are taken from file:
#/paraset/src/nameserver/src/nameServ.C.

use Socket;
use Sys::Hostname;

#port for name_server
$nameServer_port = 28948;

#This function does the following:
# creates a socket connect with the name server,
# sends command FIND_SERVER to find message/log viewer server,
# returns viewer server port and viewer server addr.
#Also to save some time this function saves the service name, port and addr in
#last_saved_service_name, last_saved_srvr_port, and last_saved_srvr_addr 
#variables. So that next time a message is being sent is the name os service is
#same as the one saved then saved port and addr are used instead of asking the
#nameserver.
sub viewer_client::find_service {
  my($service_name) = $_[0];
  if ($service_name eq $last_saved_service_name) {
     return ($last_saved_srvr_port, $last_saved_srvr_addr);
  }

  my($bytes_sent, $command, $res, $iaddr, $paddr);
  my($srvr_port, $srvr_addr) = (0, 0);

  my($proto) = getprotobyname("tcp");
  socket(SOCKET, PF_INET, SOCK_STREAM, $proto) || die "Can't create socket: $!";

  $iaddr = inet_aton(hostname());
  $paddr = sockaddr_in($nameServer_port, $iaddr);
  connect(SOCKET, $paddr) || die "Can't connect: $!";
  
  #command = FIND_SERVER = 2
  $command = pack('C', 2);
  ($bytes_sent = send SOCKET, $command, 0) || die "Can't send: $!";

  $service_len = length($service_name);

  $command = pack('V', $service_len);
  ($bytes_sent = send SOCKET, $command, 0) || die "Can't send: $!";

  $command = pack("a$service_len", $service_name);
  ($bytes_sent = send SOCKET, $command, 0) || die "Can't send: $!";

  my($sendaddr) = recv(SOCKET, $res, 1, 0);
  defined($sendaddr) || die "Can't receive: $!";
  $res = unpack('C', $res);
  if ( $res > 0 ) {
    $sendaddr = recv(SOCKET, $res, 4, 0);
    defined($sendaddr) || die "Can't receive: $!";
    $srvr_port = unpack('V', $res);
    $sendaddr = recv(SOCKET, $res, 4, 0);
    defined($sendaddr) || die "Can't receive: $!";
    $srvr_addr = unpack('V', $res);
  }
  close (SOCKET);

  #save service name, server port and address so that these can be used later
  #if possible. This will save some nameserver calls.
  if ($srvr_port != 0 && $srvr_addr != 0) {
    $last_saved_service_name = $service_name;
    $last_saved_srvr_port = $srvr_port;
    $last_saved_srvr_addr = $srvr_addr;
  }

  return ($srvr_port, $srvr_addr);
}

#This function connects to viewer server and returns the socket handle.
#LPSERVICE_SOCKET_HANDLE is a global variable to hold handle of the
#connection so that same connection can be used through out the life of the
#script. 
sub viewer_client::connectToService {
  if (!defined($LPSERVICE_SOCKET_HANDLE) ) {
    my($service_name) = $_[0];
    my($saddr);

    my($srvr_port, $srvr_addr) = &viewer_client::find_service($service_name);
    my($proto) = getprotobyname("tcp");
    socket(SOCKET, PF_INET, SOCK_STREAM, $proto) || die "Can't create socket: $!";
    $saddr = sockaddr_in($srvr_port, inet_aton($srvr_addr));
    connect(SOCKET, $saddr) || die "Can't connect: $!";
    $LPSERVICE_SOCKET_HANDLE = SOCKET;

    #autoflush LPSERVICE_SOCKET_HANDLE after every write
    my $old_handle = select $LPSERVICE_SOCKET_HANDLE; $| = 1; select $old_handle;
  }
  return $LPSERVICE_SOCKET_HANDLE;
}

#Close the connection whose handle is stored in LPSERVICE_SOCKET_HANDLE
sub viewer_client::shutdown_lpServiceConnection {
  if (defined($LPSERVICE_SOCKET_HANDLE) ) {

    #flush socket before closing.
    my $old_hndl = select $LPSERVICE_SOCKET_HANDLE; $| = 1; select $old_hndl;

    close $LPSERVICE_SOCKET_HANDLE;
    undef $LPSERVICE_SOCKET_HANDLE;
  }
}

END { &viewer_client::shutdown_lpServiceConnection(); }

#This function just sends message to log presentation(viewer) server. It 
#does not receive anything from server.
sub viewer_client::send_message {
  my ($service_name, $message) = @_;
  my($num_bytes, $SOCKET_HANDLE, $new_msg, $msg_len);

  #add length of message to its beginning 
  $msg_len = length($message);
  $new_msg = pack("Va$msg_len", $msg_len, $message);

  #print sending message
  #print "Sending: $message\n";

  $SOCKET_HANDLE = &viewer_client::connectToService($service_name);
  ($num_bytes = send $SOCKET_HANDLE, $new_msg, 0) || die "Can't send: $!";
}

#This function sends one message and receives a reply from server. The received
#message is returned to calling function.
sub viewer_client::send_and_receive_message {
  my ($service_name, $message) = @_;
  my($num_bytes, $SOCKET_HANDLE, $new_msg, $msg_len, $rcvd_msg);

  #add length of message to its beginning 
  $msg_len = length($message);
  $new_msg = pack("Va$msg_len", $msg_len, $message);

  #send message
  $SOCKET_HANDLE = &viewer_client::connectToService($service_name);
  ($num_bytes = send $SOCKET_HANDLE, $new_msg, 0) || die "Can't send: $!";

  #receive message, first message length then the message itself.
  recv($SOCKET_HANDLE, $rcvd_msg, 4, 0);
  $msg_len = unpack('V', $rcvd_msg);

  recv($SOCKET_HANDLE, $rcvd_msg, $msg_len, 0);
  $new_msg = unpack("a$msg_len", $rcvd_msg);

  return $new_msg;
}


###################################################################
# The following subroutines use "dish2 protocol" to communicate with
# the servers.
#

sub dish2_client::get_service_port {
  my($service_name) = $_[0];

  my($bytes_sent, $command, $res, $iaddr, $paddr);
  my($srvr_port, $srvr_addr) = (0, 0);

  my($proto) = getprotobyname("tcp");
  socket(NMSERV_SOCKET, PF_INET, SOCK_STREAM, $proto) || die "Can't create socket: $!";

  $iaddr = inet_aton(hostname());
  $paddr = sockaddr_in($nameServer_port, $iaddr);
  connect(NMSERV_SOCKET, $paddr) || die "Can't connect: $!";
  
  #command = FIND_SERVER = 2
  $command = pack('C', 2);
  ($bytes_sent = send NMSERV_SOCKET, $command, 0) || die "Can't send: $!";

  $service_len = length($service_name);

  $command = pack('V', $service_len);
  ($bytes_sent = send NMSERV_SOCKET, $command, 0) || die "Can't send: $!";

  $command = pack("a$service_len", $service_name);
  ($bytes_sent = send NMSERV_SOCKET, $command, 0) || die "Can't send: $!";

  my($sendaddr) = recv(NMSERV_SOCKET, $res, 1, 0);
  defined($sendaddr) || die "Can't receive: $!";
  $res = unpack('C', $res);
  if ( $res > 0 ) {
    $sendaddr = recv(NMSERV_SOCKET, $res, 4, 0);
    defined($sendaddr) || die "Can't receive: $!";
    $srvr_port = unpack('V', $res);
    $senaddr = recv(NMSERV_SOCKET, $res, 4, 0);
    defined($sendaddr) || die "Can't receive: $!";
    $srvr_addr = unpack('V', $res);
  }
  close (NMSERV_SOCKET);

  return ($srvr_port, $srvr_addr);
}

sub dish2_client::is_server_running {
  my($service_name) = $_[0];
  my($is_running) = 0;
  my($port, $addr) = &dish2_client::get_service_port($service_name);
  if ($port != 0 && $addr != 0) {
    $is_running = 1;
  }
  return $is_running;
}

sub dish2_client::send_and_receive_command {
  my ($service_name, $command) = @_;
  my($buf, $bytes_sent, $cmd_len);

  #add length of message to its beginning 
  $cmd_len = length($command);
  $cmd_len++;

  my($saddr);
  my($port, $addr) = &dish2_client::get_service_port($service_name);
  my($proto) = getprotobyname("tcp");
  socket(SSOCKET, PF_INET, SOCK_STREAM, $proto) || die "Can't create socket: $!";
  $saddr = sockaddr_in($port, inet_aton($addr));
  connect(SSOCKET, $saddr) || die "Can't connect: $!";

  #$buf = "packet number" = 0;
  $buf = pack('C', 0);
  ($bytes_sent = send SSOCKET, $buf, 0) || die "Can't send: $!";

  #$buf = DATA_FINAL_PACKET = 2
  $buf = pack('C', 2);
  ($bytes_sent += send SSOCKET, $buf, 0) || die "Can't send: $!";

  #$buf = length of the command that will follow
  $buf = pack('V', $cmd_len);
  ($bytes_sent += send SSOCKET, $buf, 0) || die "Can't send: $!";

  #$buf = command/message itself
  $buf = pack("a$cmd_len", $command);
  ($bytes_sent += send SSOCKET, $buf, 0) || die "Can't send: $!";

  #Now receive a reply from the server.
  my($recv_cmd_len, $recv_cmd, $return_code);

  recv(SSOCKET, $buf, 1, 0) || die "Can't receive: $!";
  #$buf = unpack('C', $buf); #$buf = packet number

  recv(SSOCKET, $buf, 1, 0) || die "Can't receive: $!";
  #$buf = unpack('C', $buf); #$buf = packet type

  recv(SSOCKET, $buf, 1, 0) || die "Can't receive: $!";
  $return_code = unpack('C', $buf); #$buf = return code

  recv(SSOCKET, $buf, 4, 0) || die "Can't receive: $!";
  $recv_cmd_len = unpack('V', $buf); #$buf = length of the command that will follow

  recv(SSOCKET, $buf, 1024, 0) || die "Can't receive: $!";
  $recv_cmd_len--;
  $recv_cmd = unpack("a$recv_cmd_len", $buf); #$buf = command itself

  close(SSOCKET);

  return ($return_code, $recv_cmd);
}

sub dish2_client::send_command {
  my ($service_name, $command) = @_;
  my($buf, $bytes_sent, $cmd_len);

  #add length of message to its beginning 
  $cmd_len = length($command);
  $cmd_len++;

  my($saddr);
  my($port, $addr) = &dish2_client::get_service_port($service_name);
  my($proto) = getprotobyname("tcp");
  socket(SSOCKET, PF_INET, SOCK_STREAM, $proto) || die "Can't create socket: $!";
  $saddr = sockaddr_in($port, inet_aton($addr));
  connect(SSOCKET, $saddr) || die "Can't connect: $!";

  #$buf = "packet number" = 0;
  $buf = pack('C', 0);
  ($bytes_sent = send SSOCKET, $buf, 0) || die "Can't send: $!";

  #$buf = DATA_FINAL_PACKET = 2
  $buf = pack('C', 2);
  ($bytes_sent += send SSOCKET, $buf, 0) || die "Can't send: $!";

  #$buf = length of the command that will follow
  $buf = pack('V', $cmd_len);
  ($bytes_sent += send SSOCKET, $buf, 0) || die "Can't send: $!";

  #$buf = command/message itself
  $buf = pack("a$cmd_len", $command);
  ($bytes_sent += send SSOCKET, $buf, 0) || die "Can't send: $!";

  close(SSOCKET);
}

return 1;
