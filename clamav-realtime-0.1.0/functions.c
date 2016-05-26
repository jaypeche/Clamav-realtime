#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/inotify.h>
#include <libnotify/notify.h>

void show_notify_loading(void) {

		static char *icon = "/usr/share/pixmaps/clamav-realtime.png";

	/* Send Loading Notification */

                printf ("\a");
                notify_init ("ClamAV Realtime loading...");
                NotifyNotification * Loading = notify_notification_new("ClamAV Realtime\n", "Loading...", icon);
                notify_notification_show (Loading, NULL);
}

void show_notify_nothreat(void) {

		static char *icon = "/usr/share/pixmaps/clamav-realtime.png";

	/* Send nothreat notification */

		printf ("\a");
		notify_init ("ClamAV Realtime");
		NotifyNotification * nothreat = notify_notification_new ("ClamAV Realtime", "No threat found !", icon);
		notify_notification_show (nothreat, NULL);
}
