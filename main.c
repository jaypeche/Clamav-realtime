/* Nécessaire pour obtenir la définition de O_LARGEFILE */
// #define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fanotify.h>
#include <unistd.h>

/* Lire tous les événements fanotify disponibles à partir du descripteur
   de fichier « fd » */

static void
handle_events(int fd)
{
    const struct fanotify_event_metadata *metadata;
    char buf[4096];
    ssize_t len;
    char path[PATH_MAX];
    ssize_t path_len;
    char procfd_path[PATH_MAX];
    struct fanotify_response response;

    /* Boucler tant que les événements peuvent être lus à partir du
       descripteur de fichier fanotify */

    for(;;) {

        /* Lire certains événements */

        len = read(fd, (void *) &buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Vérifier si la fin des données disponibles est atteinte */

        if (len <= 0)
            break;

        /* Pointer vers le premier événement du tampon */

        metadata = (struct fanotify_event_metadata *) buf;

        /* Boucler sur tous les événements du tampon */

        while (FAN_EVENT_OK(metadata, len)) {

            /* Vérifier que les structures au moment de l’exécution et
               de la compilation correspondent */

            if (metadata->vers != FANOTIFY_METADATA_VERSION) {
                fprintf(stderr,
            "Non correspondance de version de métadonnées fanotify.\n");
                exit(EXIT_FAILURE);
            }

            /* metadata->fd contient soit FAN_NOFD, indiquant un
               dépassement de file, soit un descripteur de fichier
               (un entier positif).
               Ici, le dépassement de file est simplement ignoré. */

            if (metadata->fd >= 0) {

                /* Traiter l’événement de permission d’ouverture */

                if (metadata->mask & FAN_OPEN_PERM) {
                    printf("FAN_OPEN_PERM : ");

                    /* Permettre d’ouvrir le fichier */

                    response.fd = metadata->fd;
                    response.response = FAN_ALLOW;
                    write(fd, &response,
                            sizeof(struct fanotify_response));
                }

                /* Traiter l’événement de fermeture de fichier ouvert
                   en écriture */

                if (metadata->mask & FAN_CLOSE_WRITE)
                    printf("FAN_CLOSE_WRITE : ");

                /* Récupérer et afficher le chemin du fichier accédé */

                snprintf(procfd_path, sizeof(procfd_path),
                         "/proc/self/fd/%d", metadata->fd);
                path_len = readlink(procfd_path, path,
                                    sizeof(path) - 1);
                if (path_len == -1) {
                    perror("readlink");
                    exit(EXIT_FAILURE);
                }

                path[path_len] = '\0';
                printf("Fichier %s\n", path);

                /* Fermer le descripteur de fichier de l’événement */

                close(metadata->fd);
            }

            /* Avancer au prochain événement */

            metadata = FAN_EVENT_NEXT(metadata, len);
        }
    }
}

int
main(int argc, char *argv[])
{
    char buf;
    int fd, poll_num;
    nfds_t nfds;
    struct pollfd fds[2];

    /* Vérifier qu’un point de montage est fourni */

    if (argc != 2) {
        fprintf(stderr, "Utilisation : %s MONTAGE\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Appuyer sur la touche Entrée pour quitter.\n");

    /* Créer le descripteur de fichier pour accéder à l’interface de
       programmation fanotify */

    fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT | FAN_NONBLOCK,
                       O_RDONLY | O_LARGEFILE);
    if (fd == -1) {
        perror("fanotify_init");
        exit(EXIT_FAILURE);
    }

    /* Marquer le montage pour :
       - les événements de permission avant d’ouvrir les fichiers ;
       - les événements de notification après fermeture de descripteur
         de fichier ouvert en écriture. */

    if (fanotify_mark(fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
                      FAN_OPEN_PERM | FAN_CLOSE_WRITE, -1,
                      argv[1]) == -1) {
        perror("fanotify_mark");
        exit(EXIT_FAILURE);
    }

    /* Préparer pour la scrutation (polling) */

    nfds = 2;

    /* Entrée de console */

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    /* Entrée de fanotify */

    fds[1].fd = fd;
    fds[1].events = POLLIN;

    /* Boucle en attente d’arrivée d’événements */

    printf("En écoute d’événements.\n");

    while (1) {
        poll_num = poll(fds, nfds, -1);
        if (poll_num == -1) {
            if (errno == EINTR)     /* Interrompu par un signal */
                continue;           /* Redémarrage de poll() */

            perror("poll");         /* Erreur inattendue */
            exit(EXIT_FAILURE);
        }

        if (poll_num > 0) {
            if (fds[0].revents & POLLIN) {

                /* Entrée de console disponible :
                   vider l’entrée standard et quitter */

                while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
                    continue;
                break;
            }

            if (fds[1].revents & POLLIN) {

                /* Événements fanotify disponibles */

                handle_events(fd);
            }
        }
    }

    printf("Arrêt de l’écoute d’événements.\n");
    exit(EXIT_SUCCESS);
}
