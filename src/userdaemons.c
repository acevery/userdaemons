/* This is a very tiny program which I learn to use some very basic
 * building block of C in Linux :)
 *
 */

#include <stdlib.h> // for exit(), EXIT_FAILURE, and EXIT_SUCESS
#include <stdio.h> // for printf(), stderr
#include <string.h> // for strcpy(), strncpy()
#include <getopt.h> //for getopt_long(), struct option 
#include <pthread.h> // for pthread_t, pthread_create(), pthread_join()
#include <sys/types.h> // for pid_t, uid_t
#include <signal.h> // for SIGKILL
#include <unistd.h> // for access(), getuid(), optarg, optind
#include <dirent.h> // for DIR, struct dirent, opendir(), readdir()
#include <sys/stat.h> // for struct stat, S_ISREG, stat()

const char *usage =
"Usage:\n\
    start:  userdaemons -s\n\
    stop:   userdaemons -p\n\
Options:\n\
    -h, --help          show this help message and exit\n\
    -c CONF, --config=CONF\n\
                        Set the configuration file, default:\n\
                        /etc/userdaeoms.conf\n\
    -s, --start         Start userdaemons\n\
    -p, --stop          Stop userdaemons\n";

typedef struct _usr_node usr_node;
struct _usr_node {
    char name[50];
    unsigned int runtime;
    pthread_t threadid;
    usr_node *prev,
             *next;
};

typedef struct _uid_node uid_node;
struct _uid_node {
    char uid[40];
    unsigned int pid;
    unsigned int runtime;
    char path[256];
    pthread_t threadid;
    uid_node *prev,
             *next;
};

void print_usage(void)
{
    printf ("%s\n", usage);
}

// start_routine to call userdaemon
void *call_userdaemon ( void *cu_arg )
{
    usr_node *this_node = (usr_node *) cu_arg;
    char cmdstr[100];
    sprintf (cmdstr, "userdaemon %s %u", this_node->name, this_node->runtime);
    if (system (cmdstr) != 0) {
        fprintf (stderr, "error in start userdaemon with uid: %s\n", this_node->name);
    }
    return NULL;
}

// start_routine when wipe out pids
void *clean_pid ( void *cp_arg )
{
    uid_node *node = (uid_node *) cp_arg;
    // 1. kill the userdaemon process 
    FILE *f_p = fopen (node->path, "r");
    unsigned int k_pid = 0;
    if ( fscanf (f_p, "%u", &k_pid) == 1 ) {
        // got the pid
        // get the runtime
        char r_file[256];
        sprintf (r_file, "/sys/kernel/uids/%s/cpu_rt_runtime", node->uid);
        FILE *f_r = fopen (r_file, "r");
        if ( fscanf (f_r, "%u", &node->runtime) != 1)
            node->runtime = 0;
        fclose (f_r);
        f_r = fopen (r_file, "w");
        fprintf (f_r, "0");
        fclose (f_r);
        kill ( (pid_t) k_pid, SIGKILL);
        node->pid = 0;
    }
    fclose(f_p);
    remove (node->path);
    pthread_exit(NULL);
}

int main (int argc, char *const argv[])
{
    // must run this as root
    //if ( getuid() != 0 ) {
    //    fprintf (stderr, "must run me as root.\n");
    //    exit (EXIT_FAILURE);
    //}

    if (argc == 1) {
        print_usage();
        return -1;
    }
    // check the /sys/kernel/uids/0/cpu_rt_runtime
    const char *ptr_root_rt = "/sys/kernel/uids/0/cpu_rt_runtime";
    if ( access (ptr_root_rt, W_OK) != 0) {
        perror ("access cpu_rt_runtime of root");
        exit (EXIT_FAILURE);
    }
    // check the /proc/sys/kernel/sched_rt_runtime_us
    const char *ptr_sys_rt = "/proc/sys/kernel/sched_rt_runtime_us";
    if ( access (ptr_sys_rt, R_OK) != 0) {
        perror ("access sched_rt_runtime_us");
        exit (EXIT_FAILURE);
    }
    FILE *ptr_rt = fopen (ptr_sys_rt, "r");
    unsigned int sys_rt;
    if (fscanf (ptr_rt, "%u", &sys_rt) != 1) {
        perror ("read sched_rt_runtime_us");
        exit (EXIT_FAILURE);
    }
    fclose(ptr_rt);

    // the configuration file
    char config[256];
    strcpy (config, "/etc/userdaemons.conf");
    
    // start or stop
    int b_start=1;
    // first of all parse the options from cmd
    int c;
    extern int optind;      // option index
    extern char *optarg;    // option argument
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"start", 0, 0, 's'},
            {"stop", 0, 0, 'p'},
            {"conf", 1, 0, 'c'},
            {0, 0, 0, 0}
        };
        c = getopt_long (argc, argv, "spc:",
                long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 's':
                b_start = 1;
                break;
            case 'p':
                b_start = 0;
                break;
            case 'c':
                strncpy (config, optarg, 255);
                break;
            default:
                print_usage ();
                return -1;
        }
    } // end of options parsing

    // now start to do the main theme :)
    // 1. stop or start
    if (b_start == 1) {
        // 1. check whether we have the conf file
        if ( access (config, F_OK | R_OK) != 0 ) {
            perror ("config file");
            exit (EXIT_FAILURE);
        }
        // 2. read from conf
        FILE *f_c = fopen (config, "r");
        usr_node *node = NULL;
        usr_node *last_node=NULL;
        char *rf;
        int rc;
        char line[256];
        char name[50];
        unsigned int runtime;
        do {
            rf = fgets (line, 256, f_c);  
            if (line[0] == '#')
                continue;
            rc = sscanf (line, " %50[a-z] : %u\n", name, &runtime);
            if (rc == 2) {
                last_node = node;
                node = (usr_node *) malloc (sizeof(usr_node));
                if (last_node != NULL)
                    last_node->next = node;
                node->prev = last_node;
                node->next = NULL;
                strncpy (node->name, name, 50);
                node->runtime = runtime;
            }
        } while (rf != NULL); 
        
        // reset link list to its head
        if (node == NULL)
            exit (EXIT_FAILURE);
        while (node->prev != NULL) {
            node = node->prev;
        }
        last_node = node;
        
        // 3. calcuate the sched_rt_runtime for root 
        unsigned int total_runtime = 0;
        while (last_node != NULL) {
            total_runtime += last_node->runtime;
            last_node = last_node->next;
        }
        // now check whether the total runtime is within the system
        // runtime range.
        if (total_runtime > sys_rt) {
            fprintf (stderr, "the total runtime (%u) > system runtime (%u)",
                    total_runtime, sys_rt);
            exit (EXIT_FAILURE);
        }
        // then, set the runtime for root
        ptr_rt = fopen (ptr_root_rt, "w");
        if (fprintf (ptr_rt, "%u", sys_rt - total_runtime) < 0 ) {
            fprintf (stderr, "fail to set root cpu_rt_runtime.\n");
            exit (EXIT_FAILURE);
        }
        fclose(ptr_rt);

        // 4. use pthread to call userdaemon to create process
        last_node = node;
        pthread_attr_t s_attr;
        pthread_attr_init (&s_attr);
        pthread_attr_setdetachstate (&s_attr, PTHREAD_CREATE_JOINABLE);
        int s_res;
        while (last_node != NULL) {
            s_res= pthread_create (&last_node->threadid, &s_attr, call_userdaemon, (void *) last_node);
            if (s_res != 0) {
                fprintf (stderr, "%s\n", strerror(s_res));
                exit(EXIT_FAILURE);
            }
            last_node = last_node->next;
        }
        // now join threads
        void *s_status;
        last_node = node;
        while (last_node != NULL) {
            s_res= pthread_join (last_node->threadid, &s_status);
            if (s_res != 0) {
                fprintf (stderr, "%s\n", strerror(s_res));
                exit(EXIT_FAILURE);
            }
            last_node = last_node->next;
        }
    } else {
        // stop the daemon
        // 1. check whether we have pids in /var/run/userdaemons
        // link list node ptr we need :)
        uid_node    *p_node = NULL;
        uid_node    *p_last_node = NULL;
        // OK now, let's start
        const char *var_path = "/var/run/userdaemon";
        if (access ( var_path, F_OK) != 0 ) {
            perror (var_path);
            exit (EXIT_FAILURE);
        }
        DIR *dp;
        struct dirent *ep;
        struct stat st;
        unsigned int r_pid = 0;
        FILE *ptr_r_pid;
        char r_pid_path[256];
        dp = opendir (var_path);
        if (dp != NULL) {
            while (ep = readdir (dp)) {
                if (ep->d_name[0]  == '.' )
                    continue;
                sprintf(r_pid_path, "%s/%s", var_path, ep->d_name);

                if ( stat (r_pid_path, &st) != -1 || S_ISREG(st.st_mode)) {
                    ptr_r_pid = fopen (r_pid_path, "r");
                    if (fscanf (ptr_r_pid, "%u", &r_pid) != 1) {
                        perror (r_pid_path);
                        exit (EXIT_FAILURE);
                    }
                    fclose(ptr_r_pid);
                    // now we got the running pid, so add a new node
                    // to link list
                    p_last_node = p_node;
                    p_node = (uid_node *) malloc (sizeof(uid_node));
                    if (p_last_node != NULL)
                        p_last_node->next = p_node;
                    p_node->prev = p_last_node;
                    p_node->next = NULL;
                    strncpy (p_node->uid, ep->d_name, 40);
                    p_node->pid = r_pid;
                    strncpy (p_node->path, r_pid_path, 256);
                }
            }
            (void) closedir (dp);
        }
        else {
            perror ("could't open the dir\n");
            exit (EXIT_FAILURE);
        }
        if (p_node == NULL) {
            fprintf (stderr, "no pid files in \"%s/\"\n", var_path);
            exit (EXIT_FAILURE);
        }
        // set p_node to the head of link list
        while (p_node->prev != NULL)
            p_node = p_node->prev;
        // set p_last_node to the head of link list
        p_last_node = p_node;

        // 2. call clean_pid to wipe out all running userdaemon 
        pthread_attr_t p_attr;
        pthread_attr_init (&p_attr);
        pthread_attr_setdetachstate (&p_attr, PTHREAD_CREATE_JOINABLE);
        int p_res;
        while (p_last_node != NULL) {
            p_res= pthread_create (&p_last_node->threadid,
                    &p_attr, clean_pid, (void *) p_last_node);
            if (p_res != 0) {
                fprintf (stderr, "%s\n", strerror(p_res));
                exit(EXIT_FAILURE);
            }
            p_last_node = p_last_node->next;
        }
        p_last_node = p_node;
        void *p_status;
        while (p_last_node != NULL) {
            p_res= pthread_join (p_last_node->threadid, &p_status);
            if (p_res != 0) {
                fprintf (stderr, "%s\n", strerror(p_res));
                exit(EXIT_FAILURE);
            }
            p_last_node = p_last_node->next;
        }
        // now return the runtime to root :)
        //
        ptr_rt = fopen (ptr_root_rt, "w");

        if (fprintf(ptr_rt, "%u", sys_rt) < 0 ) {
            perror ("reset cpu_rt_runtime for root");
            exit (EXIT_FAILURE);
        }
        fclose(ptr_rt);
    }
    exit (EXIT_SUCCESS);
}
