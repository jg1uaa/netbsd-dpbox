#! /bin/sh
#
# /sbin/init.d/dpbox
#
# Setup by Joachim Schurig, DL8HBS <jschurig@zedat.fu-berlin.de>
# Used a skeleton of the SuSE-config-scripts

if test -e /etc/rc.config ; then
. /etc/rc.config
fi

if [ "$rc_done" == "" ]; then
     rc_done="\033[71G\033[32mdone\033[m"
   rc_failed="\033[71G\033[31m\033[1mfailed\033[m"
fi

# Determine the base and follow a runlevel link name.
base=${0##*/}
link=${base#*[SK][0-9][0-9]}

# The echo return value for success (defined in /etc/rc.config).
return=$rc_done
case "$1" in
    start)
	echo -n "Starting service dpbox"
	## Start daemon with startproc(8). If this fails
	## the echo return value is set appropriate.

	startproc -q /usr/sbin/dpbox -i /etc/dpbox/dpbox.ini || return=$rc_failed

	echo -e "$return"
	;;
    stop)
	echo -n "Shutting down service dpbox"
      	killproc -TERM /usr/sbin/dpbox || return=$rc_failed
        echo -e "$return"
	;;
    restart)
	$0 stop  && sleep 5 &&  $0 start  ||  return=$rc_failed
      	;;
    reload)
      	echo -n "Reloading configuration for service dpbox"
	killproc -q -HUP /usr/sbin/dpbox || return=$rc_failed
        echo -e "$return"
	;;
    status)
	echo -n "Checking for service dpbox: "
	## Check status with checkproc(8), if process is running
	## checkproc will return with exit status 0.

	checkproc /usr/sbin/dpbox && echo OK || echo No process
	;;
    *)
	echo "Usage: $0 {start|stop|status|restart|reload}"
	exit 1
	;;
esac

# Inform the caller not only verbosely and set an exit status.
test "$return" = "$rc_done" || exit 1
exit 0
