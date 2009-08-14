/* Filename: userdaemon.c
 * Encoding: UTF-8
 * Design for called by userdaemons.
 * This program use the user (argv[1]) and cpu_rt_runtime (argv[2])
 * to run a sleep process as the user, then set cpu_rt_runtime to give
 * value, so the real program of the user running after it can do RT
 * Schedule :)
 * For program as pulseaudio, jack, mpd, this is only way to offer
 * realtime cpu bandwidth for them before they starting.
 */
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char const* argv[])
{
    if (getuid() != 0) {
        fprintf (stderr, "run me as root.\n");
        return 1;
    }
    if (argc != 3) {
        fprintf (stderr, "give one user and his sched_rt_runtime.\n");
        return 2;
    }
    // check whether we have /sys/kernel/uids/0/sched_rt_runtime
    if ( access ("/sys/kernel/uids/0/cpu_rt_runtime", F_OK|W_OK) != 0) {
        fprintf (stderr, "please set \"CONFIG_USER_SCHED=y\" in your kernel config first\n");
        return 3;
    }
    struct passwd *pw;
    int fork_res;
    pw = getpwnam ( argv[1] );
    if ( pw == NULL)
        return 4;
    // we successfule get the uid as require, 
    // start a pipe for parent and child talking
    int pipefd[2];
    if (pipe( pipefd ) != 0) {
        perror ("fail create pipe");
        exit (EXIT_FAILURE);
    }
    // now, we can fork this process
    fork_res = fork ();
    switch (fork_res) {
        case (pid_t) -1: {
                    // fail to fork T_T
                    return 5;
                 }
                 break;
        case (pid_t) 0: {
                    // this is the child, change the uid
                    close ( pipefd[0] ); // close read fd
                    if ( setuid( pw->pw_uid ) )
                    {
                        // fail
                        if ( write (pipefd[1], "1", 1) == -1) {
                            perror ( "child write");
                            exit (EXIT_FAILURE);
                        };
                    } else {
                        // successful
                        if (write (pipefd[1], "0", 1) == -1) {
                            perror ( "child write");
                            exit (EXIT_FAILURE);
                        };
                    }
                    close (pipefd[1]); // close write fd
                    // now time to sleep
                    while (1) {
                        sleep (600000);
                    }
                }
                break;
        default: {
                    // this is the parent.
                    close (pipefd[1]); // close write fd
                    char buf;
                    if (read (pipefd[0], &buf, 1) == -1) {
                        perror ( "parent read");
                        exit (EXIT_FAILURE);
                    };
                    close (pipefd[0]);
                    if (buf != '0') {
                        // fail to change uid in child T_T
                        fprintf (stderr, "Fail to setuid in child process...\n");
                        exit (EXIT_FAILURE);
                    }

                    // 1. check whether we have /var/run/userdaemon
                    char pid_file[50];
                    strcpy (pid_file, "/var/run/userdaemon/");
                    if ( access (pid_file, F_OK) == -1 ) {
                        if (mkdir (pid_file, 00755) == -1 ) {
                                perror ("mkdir");
                                exit (EXIT_FAILURE);
                            }
                    }
                    sprintf (pid_file, "/var/run/userdaemon/%u", pw->pw_uid);
                    FILE *ptr_f;
                    if ( access (pid_file, F_OK) == 0 ) {
                        // already have running pid ?
                        ptr_f = fopen (pid_file, "r");
                        int r_pid = 0;
                        if ( fscanf (ptr_f, "%d", &r_pid) == 1 ) {
                            // we got running pid..., try to kill it
                            // first.
                            if ( kill ((pid_t) r_pid, SIGKILL ) == -1 ) {
                                perror ("kill");
                                exit (EXIT_FAILURE);
                            }
                        }
                        fclose(ptr_f);
                    }
                    ptr_f = fopen(pid_file, "w");
                    fprintf (ptr_f, "%d\n", fork_res);
                    fclose(ptr_f);
                    // now write the sched_rt_runtime for given user
                    // 1. prepare the file path;
                    char rt_file[50];
                    sprintf (rt_file, 
                            "/sys/kernel/uids/%d/cpu_rt_runtime", pw->pw_uid);
                    // 2. check the file path
                    if ( access (rt_file, F_OK|W_OK) != 0 ) {
                        fprintf (stderr, "fail to acess:\"%s\"\n", rt_file);
                        return 6;
                    }
                    // 3. write the cpu_rt_runtime file
                    ptr_f = fopen (rt_file, "w");
                    fprintf (ptr_f, "%s", argv[2]);
                    fclose(ptr_f);
                    return 0;
                 }
    }
}
