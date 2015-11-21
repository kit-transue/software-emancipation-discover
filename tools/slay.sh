#!/bin/sh 
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
#
# Slay: a program to kill off DISCOVER and its relatives
#
# Usage: slay [-n|-nokill] [signal] [-S|-s]
#	-n|-nokill	report list of children instead of killing them
#	-[int]		specify the interrupt that kill will use
# 	-s|-S		used to kill off one of a set of discover sessions--
#			used internally and not guaranteed to be robust

Usage()
{
echo "Usage: slay [-n|-nokill] [_signal_]"
echo "       -n|-nokill        report list of children instead of killing them"
echo "       signal: (-an integer) specify the interrupt that kill will use "
echo "                         e.g. 'slay -24'"
}

ihandler()
{

	$echo
	$echo    " Interrupts are ignored while slaying."
	$necho  " If you want to abort slay type y:"
	read response
 	case "$response" in
		y*|Y*)
			KillSlayTempFiles
                        [ "$my_tmp" ] && [ -f $my_tmp -o -f $my_tmp.2 ] && rm -rf ${my_tmp}* /tmp/psout.$$
            		echo "Slay exiting."
			exit 0;;
	esac
	return 0
}



# all tmp files will be stored like:
# PID  procname except asetpids
# this procedure gets the list of asetpids,
# figures out if there is more than one of them,
# and converts them to our temp format
GenerateAsetDeadPids()
{
 if [ "`cat /tmp/asetpids.$$ | wc -l`" -gt 1 ] ; then
	if [ "$killall" ]  ; then
		response=all
	else
	    $echo
	    $echo
	    $echo "  There is more than one DISCOVER executable running"
	    $echo " (process name \"aset\"). Choose from the following"
	    $echo " pids (terminate the input with <Cntl-D>):"
	  #note: for historical reasons asetpids.$$ only has one field.   
	  cat  /tmp/asetpids.$$ | 
		while read xpid
		do
		echo $xpid
		done		
	    $echo
	    $echo " You may also type \"all\" for all aset processes"
	    $echo " OR  \"pid1 pid2 pid3..\"(separated by white space)"
	    $echo " for the aset(s) of the corresponding DISCOVER sessions "
	    if [ $kill_flag -eq 1 ] ; then
	 	$echo " you want terminated."
	    else
		$echo " you are considering terminating."
	    fi
	   
	    asets=""
	    while read response
	    do
		if [ "$asets" = "" ] ; then
			asets=$response
		else
			asets="$asets $response"
		fi
	    done
	    response="$asets"
	fi
    elif [ "`cat /tmp/asetpids.$$ | wc -l`" -eq 0 ] ; then
	$echo  "There is no aset running."
	$echo  "Trying to clean up any stray processes..." 
	return
    else
	response=all # one is all we have
    fi
    
    # have to create the PID NAME format
    if [ "$response" = all ] ; then
	cat /tmp/asetpids.$$ |
	while read y
	do
	 echo "$y aset" >> /tmp/asetdeadpids.$$
	done
    else
	for p in $response
	do
	echo "$p	aset" >>/tmp/asetdeadpids.$$
	done
     fi
}

# this goes through and adds the orphans to the finalpid list
# an orphan is defined as one of our programs with parent pid 1
GenerateOrphanPids()
{

echo "Hunting for orphans..."
# generate the list of processes with pid 1
cat /tmp/psout.$$ | 
awk '{if($ppidfield==1) print $pfield " " $NF}' \
pfield=$pfield ppidfield=$ppidfield > /tmp/orphanpids.$$

# append our executables
x=`egrep 'Configurator$|Window$|g++filt$|aset_cpp$|c_parser$|cp_parser$|maker$|frame_api$|frame_srvr$|gala$|pset-gdb$|pset-gdb-4\.14$|pset-gdb-4\.2$|pset-gdb-4\.11$|pset-gdb-4\.15$|pset-gdb-4\.15\.1$|helper$|pset-epoch$|splash$' /tmp/orphanpids.$$`

if [ "$x" != "" ] ; then
	echo $x
	echo $x >> /tmp/finalpids.$$
else
 	echo "No orphan processes were found."
fi
}

#This generates the finalpids list, which contains the PID and name of
#all processes that should be killed.  It prints them out depth first
# (max depth =3, but you could change this...), which is important
# because it ensures that child processes get deleted before their parent
# processes (keeps nasty things from happening--e.g. processes being owned by
# init

GenerateDeadPids()
{

GenerateAsetDeadPids

#generate a list of our executables with ppid 1
GenerateOrphanPids

if [ -f /tmp/asetdeadpids.$$ ] ; then 
	cat /tmp/asetdeadpids.$$ |
	while read asetpid asetprocname  #asetprocname is ignored
	do

	#vestigial logic -- probably unnecessary
	if [ "$asetpid" -ne 1 ]; then
		if [ -f /tmp/childpids.$$ ] ; then
			rm -f /tmp/childpids.$$
		fi

		# go through the process table printing PID, procs with Parent PId=asetpid
		# this awk statement translates to:
		# find the lines with parent pid = current aset pid (i.e.
		# children of the current aset), print their pid and 
		# name ($NF refers to the last field in the line, which is
		# proc name), with the awk variables being assigned values
		# from the sh variables
		# most other awk statements in this file are similar
		cat /tmp/psout.$$ |
		awk '{if($ppidfield==asetpid) print $pfield " " $NF}' \
		 pfield=$pfield ppidfield=$ppidfield asetpid=$asetpid \
		 > /tmp/childpids.$$
	
		#now add the grandchildren
		if [ -f /tmp/childpids.$$ ] ; then
		   cat /tmp/childpids.$$ | 
		   while read xpid xproc
		   do
			cat /tmp/psout.$$|
		   	awk '{if($ppidfield==xpid) print $pfield " " $NF}' \
		   	pfield=$pfield ppidfield=$ppidfield xpid=$xpid  \
		   	>> /tmp/finalpids.$$
		   done
		fi
		
		##append the childpids to the finalpids list
		# note that they are appended AFTER the grandchildren
		cat /tmp/childpids.$$ >> /tmp/finalpids.$$

	   fi
	done #while read asetpid asetproc
	# now append the whole asetdeadpid list
	cat /tmp/asetdeadpids.$$ >>/tmp/finalpids.$$
 fi
return
}

# this takes care of killing or displaying
# the pid, name combination
# the "Killed" message comes from the process or shell, not the kill statement
KillDeadPids()
{
if [ "$kill_flag" -eq 1 ] ; then
	echo "Killing the following processes:"
else
	echo "Slay would kill the following processes:"
fi

if [ -f /tmp/finalpids.$$ ] ; then
	cat /tmp/finalpids.$$ |
	while read pid proc
	do
	if [ "$proc" != "wakeup" ] ; then

		echo "PID=$pid	Name=$proc"
		if [ "$kill_flag" -eq 1  ] ;  then 
	 		$kill $signum $pid >/dev/null 2>/dev/null
			sleep 1
		fi
	fi
	done
else
  echo "No Discover-related processes were found."
fi
}



#
#I copied this verbatim from the previous version of slay. It seems to 
# report too many files in -n mode, but I don't have time to rewrite it.

KillDiscoverTempFiles()
{
TMPDIR=${TMPDIR:=/usr/tmp}
if [ ! "$killall" ] ; then
	echo "Leaving temp directories/files around in $TMPDIR."
	return
else

my_tmp="/usr/tmp/slay_tmp_ls.$$"
ls -ld $TMPDIR/DISCOVER.* \
        $TMPDIR/SharedMem* \
        $TMPDIR/SharedMem* \
        $TMPDIR/make* \
	$TMPDIR/cpp*\
	$TMPDIR/interp_pipe* \
	$TMPDIR/*.ind \
	$TMPDIR/*.sym   > $my_tmp 2> /dev/null 

if [ $kill_flag -eq 0 ] ; then
	if [ `cat $my_tmp|wc -l` -eq 0 ] ; then
		$echo "No temp files would be removed."
	else
		$echo "These temporary files would be removed:"
		cat $my_tmp
	fi
	$rm -fr $my_tmp 2> /dev/null
	exit 0
fi

if [ "$the_os" = "hp700" -o "$the_os" = "irix6" -o "$the_os" = "linux2" ] ;  then
   	grep $u $my_tmp | awk '{print $9;}' > $my_tmp.2
else
   	grep $u $my_tmp | awk '{print $8;}' > $my_tmp.2
fi
if [ `cat $my_tmp.2 | wc -l` -gt 0 ] ;  then 
	$echo "Removing temp files: "
	tmpfiles=`cat $my_tmp.2`
	cat $my_tmp.2
	sleep 3
	[ "$tmpfiles" ] && $rm -rf $tmpfiles
else
	$echo "No temporary files found."
	tmpfiles=""
fi

not_removed=""
cat $my_tmp.2 |
while read file
do
    	if [  -f "$file" ] || [ -d "$file" ] ; then 
		not_removed="$not_removed $file"
    	fi
done

if [ "$not_removed" ] ;  then
    $echo "NOT REMOVED:"
    ls -l $not_removed
fi

$rm -fr $my_tmp $my_tmp.2 > /dev/null 2>&1
fi
}

#convenient way of cleaning up.
# a less subtle way of doing this would be rm -f /tmp/*.$$
KillSlayTempFiles()
{
 echo "Deleting slay temporary files..."
 for tmpfile in $slay_tmp_file_list 
 do
	# echo "current temp file:  $tmpfile"
 	 $rm -f  $tmpfile 2>/dev/null
 done
}


#
# option processing
#
killall=1
kill_flag=1
while [ $# -gt 0 ] ; 
do
	case "$1" in 
	 -[1-9]|-[1-2][0-9]|-3[012]) 
			#signals between 0-32
			signum=$1 
			shift
			;; 
	 -s|-S) 
		# kill only specific asets
			killall=""
			shift
			;;
	-n|-nokill|-N|-NOKILL)
		# just do diagnostics
			kill_flag=0
			shift
			;;

	*)		shift
			Usage
			kill_flag=0
			exit 0
			;;
		
	esac
done

#
#defaults
#
kill=${kill:=kill}
signum=${signum:=-9}
rm=${rm:=rm}
the_os=`DISarch`

# if you add a temp file, add its location here.
slay_tmp_file_list="/tmp/asetpids.$$ /tmp/asetdeadpids.$$ /tmp/orphanpids.$$ /tmp/finalpids.$$ /tmp/childpids.$$ /tmp/psout.$$ /tmp/modelscriptpids.$$"

case "$the_os" in
sun4) whoami="${whoami:=/usr/ucb/whoami}"
	u="`$whoami`"
	psopts="cjx" # Display: internal command name, jobcontrol(PPID), nocontrolling term 
	ps="/bin/ps -$psopts"
	pfield=2     # PID field number
	ppidfield=1
	echo="/usr/5bin/echo"
	necho="/usr/5bin/echo"
	$ps | awk '$NF=="aset" {print $2}' > /tmp/asetpids.$$
get_modelscripts() {
	rm -f /tmp/modelscriptpids.$$ 2> /dev/null
	/bin/ps -jxww | egrep 'discover$|modelscript$|modelbuild$|modeladmin$|start_discover$|disXvfb$|dismb$|dismb_driver.pl$|dismb_remote.pl$' | awk '{print $2}' > /tmp/modelscriptpids.$$
}
	$ps > /tmp/psout.$$
	;;
sun5)   whoami="${whoami:=/usr/ucb/whoami}"
	u="`$whoami`"
	uid=`id | awk -F= '{print $2}' | awk -F\( '{print $1}'`
	psopts="acxl" # Display: internal command name, jobcontrol(PPID), nocontrolling term 
        ps="/usr/ucb/ps -$psopts"
	pfield=3     # PID field number
	uidfield=2
	ppidfield=4
	echo="/usr/5bin/echo"
	necho="/usr/5bin/echo"
	$ps | awk '$NF=="aset" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield > /tmp/asetpids.$$
get_modelscripts() {
	rm -f /tmp/modelscriptpids.$$ 2> /dev/null
	$ps | awk '$NF=="modelscript" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield > /tmp/modelscriptpids.$$
	$ps | awk '$NF=="discover" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
	$ps | awk '$NF=="disXvfb" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
	$ps | awk '$NF=="dismb" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
	$ps | awk '$NF=="dismb_driver.pl" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
	$ps | awk '$NF=="dismb_remote.pl" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
}
	$ps >/tmp/psout.$$
	;;
	
     linux2)
	whoami="${whoami:=/usr/bin/whoami}"
	u="`$whoami`"	
	uid=`id | awk -F= '{print $2}' | awk -F\( '{print $1}'`
	psopts="acxl"
	ps="/bin/ps -$psopts"
	pfield=3     # PID field number	
	uidfield=2
	ppidfield=4
	echo="/bin/echo"
	$ps | awk '$NF=="aset" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield > /tmp/asetpids.$$
get_modelscripts() {
	rm -f /tmp/modelscriptpids.$$ 2> /dev/null
	$ps | awk '$NF=="modelscript" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield > /tmp/modelscriptpids.$$
	$ps | awk '$NF=="discover" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
	$ps | awk '$NF=="disXvfb" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
	$ps | awk '$NF=="dismb" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
	$ps | awk '$NF=="dismb_driver.pl" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
	$ps | awk '$NF=="dismb_remote.pl" {if($uidfield==uid)print $3}' uid=$uid uidfield=$uidfield >> /tmp/modelscriptpids.$$
}
	$ps >/tmp/psout.$$
	;;

     *) # mainly hp for now.
	echon() {
		echo $* '\c'
		}
	u=`whoami`
	uid=`id -u`
	pfield=4
	ppidfield=5
	echo="echo"
	necho="echon"
	ps -el | awk '$3==uid {print $0}' uid=$uid | awk '$NF=="aset" {print $4}' > /tmp/asetpids.$$
get_modelscripts() {
	rm -f /tmp/modelscriptpids.$$ 2> /dev/null
	ps -el | awk '$3==uid {print $0}' uid=$uid | awk '$NF=="modelscript" {print $4}' > /tmp/modelscriptpids.$$
	ps -el | awk '$3==uid {print $0}' uid=$uid | awk '$NF=="discover" {print $4}' >> /tmp/modelscriptpids.$$
	ps -el | awk '$3==uid {print $0}' uid=$uid | awk '$NF=="disXvfb" {print $4}' >> /tmp/modelscriptpids.$$
	ps -el | awk '$3==uid {print $0}' uid=$uid | awk '$NF=="dismb" {print $4}' >> /tmp/modelscriptpids.$$
	ps -el | awk '$3==uid {print $0}' uid=$uid | awk '$NF=="dismb_driver.pl" {print $4}' >> /tmp/modelscriptpids.$$
	ps -el | awk '$3==uid {print $0}' uid=$uid | awk '$NF=="dismb_remote.pl" {print $4}' >> /tmp/modelscriptpids.$$
}
	ps -el | awk '$3==uid {print $0}' uid=$uid >/tmp/psout.$$
	;;
esac

trap 'ihandler'  1 2 15 

if [ "$kill_flag" -eq 0 ] ; then
    $echo "================================================="
    $echo "slay: Checking for processes related to DISCOVER."
    $echo "================================================="
else
    $echo "================================================="
    $echo "slay: Killing processes related to DISCOVER."
    $echo "(Use 'slay -n' to analyze without killing.)"
    $echo "================================================="
fi

#
# Kill the calling processes: modelscript and all its variants
# (the 'ps' we used above gets modelbuild, start_discover, too as long
# as they remain links to modelscript.)
#
if [ $kill_flag -eq 1 ] ; then
	get_modelscripts
	while [ "`cat /tmp/modelscriptpids.$$ 2> /dev/null | wc -l`" -gt 0 ]
	do
		cat /tmp/modelscriptpids.$$ |
		while read modelscriptpid
		do
			$echo "killing calling script: " $modelscriptpid
			kill $signum $modelscriptpid 2> /dev/null
		done 
		get_modelscripts
	done
	rm -f /tmp/modelscriptpids.$$ 2> /dev/null
else 
	get_modelscripts
	if  [ "`cat /tmp/modelscriptpids.$$ 2> /dev/null | wc -l`" -gt 0 ]
	then
	    $echo "DISCOVER calling scripts (discover, start_discover, modelbuild): "
	    cat /tmp/modelscriptpids.$$ 
	fi		
fi
	 
# generate the list of pids that have to die...
GenerateDeadPids
# kill (or display) all pids
KillDeadPids
# kill (or display all temp files)
KillDiscoverTempFiles
#kill all slay tmp files
KillSlayTempFiles

exit 0


