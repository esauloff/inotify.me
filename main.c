#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>

#define INOTIFY_EVENT_SIZE (sizeof (struct inotify_event))
#define BUFFER_LENGTH (1024 * INOTIFY_EVENT_SIZE)

int main(int argc, char** argv) {
    int keep_running = 1;

    char path[255];
    struct stat path_stat;

    int inotify_fd;             // inotify file descriptor
    int inotify_wd;             // inotify watch descriptor
    int mask = IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO;

    struct inotify_event* event;
    char buffer[BUFFER_LENGTH];
    int length;
    int i;

    printf("Directory to follow: ");
    scanf("%254s", path);

    stat(path, &path_stat);
    if(!S_ISDIR(path_stat.st_mode)) {
        printf("Input is not local directory\n\n");

        return 1;
    }

    printf("Following directory \"%s\"\n\n", path);

    inotify_fd = inotify_init();
    if(inotify_fd == -1) {
        printf("Failed to initialize inotify_fd: %s\n\n", strerror(errno));

        return 1;
    }

    inotify_wd = inotify_add_watch(inotify_fd, path, mask);
    if(inotify_wd == -1) {
        printf("Failed to initialize inotify_wd: %s\n\n", strerror(errno));

        close(inotify_fd);

        return 1;
    }

    while(keep_running) {
        i = 0;
        length = read(inotify_fd, buffer, BUFFER_LENGTH);
        if(length == -1) {
            printf("Failed to read event information: %s\n\n", strerror(errno));

            inotify_rm_watch(inotify_fd, inotify_wd);
            close(inotify_fd);

            return 1;
        }

        while(i < length) {
            event = (struct inotify_event*)&buffer[i];
            if(!event) {
                break;
            }

            if(event->len) {
                printf("** Event occurred\n");
                if(event->mask & IN_CREATE) {
                    printf("Filesystem object %s was created\n", event->name);
                }
                else if(event->mask & IN_DELETE) {
                    printf("Filesystem object %s was deleted\n", event->name);
                }
            }

            i += INOTIFY_EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(inotify_fd, inotify_wd);
    close(inotify_fd);

    return 0;
}

