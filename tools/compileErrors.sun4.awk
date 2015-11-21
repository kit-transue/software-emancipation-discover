BEGIN {totalerrors=0}
/^CC / { printed=0;p=0; elis[p]="-----------" $(NF)"--------" ;}
/^.*line.*:/ {elis[++p]=$0; printed=1; totalerrors++} 
	     {if (printed!=1)elis[++p]=$0; printed=0; }
/Successful/ { totalerrors= totalerrors - p }
/^gmake.*Error/ {for(i=0;i<=p;print elis[i++]); p=0; error=0; printed=0 }
END { print ; print "-------------TOTAL ERRORS:       " totalerrors}

