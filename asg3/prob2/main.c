#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <utmpx.h>
#include <time.h>
#include <syslog.h>
#include <daemon.h>

#define LOG_FILE "/var/log/user_login_daemon.txt"
#define SLEEP_TIME 10

void login_accounting_log_login_message(void)
{
    
    openlog("LoiND", LOG_DAEMON | LOG_PID | LOG_CONS | LOG_NOWAIT, LOG_DAEMON);
    setutxent();
    syslog(LOG_USER | LOG_INFO, "user type PID line id host date/time\n");

    struct utmpx *ut;
    while ((ut = getutxent()) != NULL) 
    {
        syslog(LOG_USER | LOG_INFO, "%-8s ", ut->ut_user);
        syslog(LOG_USER | LOG_INFO, "%-9.9s ",
        (ut->ut_type == EMPTY) ? "EMPTY" :
        (ut->ut_type == BOOT_TIME) ? "BOOT_TIME" :
        (ut->ut_type == NEW_TIME) ? "NEW_TIME" :
        (ut->ut_type == OLD_TIME) ? "OLD_TIME" :
        (ut->ut_type == INIT_PROCESS) ? "INIT_PR" :
        (ut->ut_type == LOGIN_PROCESS) ? "LOGIN_PR" :
        (ut->ut_type == USER_PROCESS) ? "USER_PR" :
        (ut->ut_type == DEAD_PROCESS) ? "DEAD_PR" : "???");
        syslog(LOG_USER | LOG_INFO, "%5ld %-6.6s %-3.5s %-9.9s ", (long) ut->ut_pid,
        ut->ut_line, ut->ut_id, ut->ut_host);
        syslog(LOG_USER | LOG_INFO, "%s", ctime((time_t *) &(ut->ut_tv.tv_sec)));
    }
    endutxent();
    closelog();
}
int main(int argc, char **argv)
{
    becomeDaemon(0);
    login_accounting_log_login_message();
	return (0);
}