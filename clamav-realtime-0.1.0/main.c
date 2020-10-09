/*  This is clamav-realtime sources build with gcc-4.8.3
 *  Version 0.1.0 - 19 jan 2015
 *  Distributed under the terms of General Public License GPLv3
 */


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
#include <clamav.h>
#include "functions.h"


/* Variables definition */
#define EVENT_SIZE  (sizeof (struct inotify_event))
#define BUF_LEN        (16 * (EVENT_SIZE + 16))

/* Main program */
int main()
{
    /* Are you running as root ? */
    if(geteuid() == 0) { printf("ERROR: This application does not need to be execute as root !\n");
			 printf("       For security, it should be run as user.\n");
			 exit(1); }

     /*initialization : */
     printf("Loading clamav-realtime process %d, please wait...\n", getpid());

    /*  Create inotify instance */
    int fdinit;
    fdinit = inotify_init();
    if (fdinit < 0)
        perror("inotify_not_init()"); /* Return error if not initialized */

    /* Watching inotify events into $HOME */

	char *getenv(const char *name);
	char *targetdir=getenv("HOME");
	int wd;

    wd = inotify_add_watch(fdinit, targetdir, IN_CLOSE_WRITE );
    if (wd < 0)
        perror("inotify_add_watch"); /* Return error add_watch */

    char buf[BUF_LEN]; /* Initialize events buffer */
    int len;

/* ClamaAV initialization declaration */
int fd, ret;
unsigned int sigs = 0;
unsigned long int size = 0;
const char *virname;
char *icon = "/usr/share/pixmaps/clamav-realtime.png";
struct cl_engine *engine;
struct cl_scan_options options;

/* Initialisation libClamAV */
if((ret = cl_init(CL_INIT_DEFAULT)) != CL_SUCCESS) {
printf("Can't initialize libclamav: %s\n", cl_strerror(ret));
return 2;
}

/* Send Loading Notification */
 	show_notify_loading();

/* New engine Test */
if(!(engine = cl_engine_new())) {
printf("Can't create new engine\n");
return 2;
}

/* load all available databases from default directory */
if((ret = cl_load(cl_retdbdir(), engine, &sigs, CL_DB_STDOPT)) != CL_SUCCESS) {
printf("cl_load: %s\n", cl_strerror(ret));
    cl_engine_free(engine);
return 2;
}
printf("Loaded %u signatures.\n", sigs);

/* Create quarantine */
struct stat file_stat;
char path_quarantine[PATH_MAX];
strcpy(path_quarantine, "/tmp");
strcat(path_quarantine, "/.quarantine/");

if ( stat(path_quarantine, &file_stat) !=0) {
	printf("Create quarantine in specific folder : %s\n",path_quarantine);
	mkdir(path_quarantine, 0755);
	} else {
		printf("Quarantine folder was already created into : %s\n",path_quarantine);
	}

/* build engine */
if((ret = cl_engine_compile(engine)) != CL_SUCCESS) {
	printf("Database initialization error: %s\n", cl_strerror(ret));;
    	cl_engine_free(engine);
	return 2;
	} else {
        	printf("Engine initialization success !\n\n");
	}	

while (( len = read( fdinit, buf, BUF_LEN )) > 0 ) {

    if ( len < 0 ) {
    perror( "length read error" ); /* Return length error */
    }

    /*  Actually read return the list of change events happens.
	Here, read the change event one by one and process it
	accordingly.*/

    int i = 0;

    while (i < len)
    {

		struct inotify_event *event;
            	event = (struct inotify_event *) &buf[i];

            if (event->len)
		//printf("New file created %s", event->name);

            if (event->mask & IN_CLOSE_WRITE)
                printf("Scanning file : %s\n", event->name);

		char *filename = event->name;
		char current_path[PATH_MAX];

		strcpy(current_path, targetdir);
		strcat(current_path, "/");
		strcat(current_path, filename);

		printf("Current path : %s\n", current_path);

		/* Open File Descriptor ! */
		if((fd = open(current_path, O_RDONLY)) == -1) {
		printf("Can't open file %s\n", current_path);
		return 2;
		}

		/* scan file descriptor */
    		memset(&options, 0, sizeof(struct cl_scan_options));
    		options.parse |= ~0;                           /* enable all parsers */
    		options.general |= CL_SCAN_GENERAL_HEURISTICS; /* enable heuristic alert options */

		/* Scan engine */
		if((ret = cl_scandesc(fd, filename, &virname, &size, engine, &options)) == CL_VIRUS) {
			printf("Virus detected : %s\n", virname);
			close(fd);

			/* Move to quarantine */
			char path_to_quarantine[PATH_MAX];
			strcpy(path_to_quarantine, path_quarantine);
			strcat(path_to_quarantine, filename);
			strcat(path_to_quarantine, ".quarantine");

			int status;
			status = rename(current_path,path_to_quarantine);
			if(status == 0) {
				printf("File was moved to %s\n\n", path_to_quarantine);
				} else {
				perror("Error: unable to rename the file\n");
				printf("errno = %d.\n", errno);
				return 2;
			}

			/* Chmod 644 */
			/*char mode[] = "0644";
			int j;
			j = strtol(mode, 0, 8);
			if (chmod (path_to_quarantine,j) < 0) {
				perror("Error: Failed to change mode 644\n");
				printf("errno = %d.\n", errno);
				return 2;
			}*/

			/* Generate body of notification */
			char alert_threat[] = "1 threat found :\n";
			char body_threat[128];

			strcpy(body_threat, alert_threat);
			strcat(body_threat, virname);
			strcat(body_threat, "\n");
			strcat(body_threat, current_path);
			strcat(body_threat, " was moved to quarantine !");

			/* Send Notification */
		        printf ("\a");
			notify_init ("ClamAV Realtime");
			NotifyNotification * threat = notify_notification_new ("ClamAV Realtime", body_threat, icon);
			notify_notification_set_urgency (threat, NOTIFY_URGENCY_CRITICAL);
			notify_notification_show (threat, NULL);
			} else {
				if(ret == CL_CLEAN) {
					printf("No virus detected\n\n");
					close(fd);

					/* Send Notification nothreat */
					show_notify_nothreat();

					} else {
						printf("Error: %s\n", cl_strerror(ret));
						cl_engine_free(engine);
						close(fd);
						return 2;
						}

		}
		close(fd);
            	i += EVENT_SIZE + event->len;
     }
}
cl_engine_free(engine);
return 0;
}
