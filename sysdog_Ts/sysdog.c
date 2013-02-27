// Compile with gcc filename.c -o watchdog
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "wd_ts75xx.h"  //Include header that implement 'enable_wd()' and 'feed_dog()' functions

//This program is designed to run on a embedded system and try to warrantee two things.
//-> system network access
//-> system main program, continue execution

//To warrantee that this program uses the watchdog functionality of the embedded system
//In order to do that the program needs a implementation of two functions 'enable_wd()' and 'feed_dog()'
//this funtions are system specific, so need to be implemented for each different embedded system in wich this program will run.

//This program recives two arguments 'web address to check conectivity' and 'comand to run and chack out'
//Example:
//> sysdog "www.udea.edu.co" "ping www.google.com &> log.txt"
//
//In this example the first argument ("www.udea.edu.co") indicates that that is the address that will be used to check the network conection.
//the second argument "ping www.google.com &> log.txt" is the comand that will be executed, and will be watched to always be working.

//-> system network access
//      The network access will be verified making ping to the check address (first argument, or google.com by default)
//
//-> system main program, continue execution
//      The comand to execute the surveilled program. this is usually the main program that will be running on the embedded system and the
//      one that must be running always as loong as the embedded system is running.

//This program is designed to run at the beggining of the ooperating system execution, and as root.
//That can be achieved putting the execution line in the file '/etc/rc.local'
//Example:
//(this example assume the file 'sysdog' (bin file) is located in the path /root/sysdog)
//Put the following line in the file '/etc/rc.local'
//>/root/sysdog "www.udea.edu.co" "ping www.google.com"
//
//!!NOTE: Is important to verify that this line is located before the line 'exit 0' th the /etc/rc.local' file

//!!NOTE 2: The network access verification is made using '/bin/ping', and the option '-w' is used. Some imlementations of '/bin/ping' doesn't have that option.
//          You can check if your imlementation have such option executing 'ping --help' and searching such option.
//          If you doont have that option you can try to fix it updating the '/bin/ping' program by executing the followiong command 'sudo apt-get install iputils-ping'

//!!NOTE 3: The comand to execute the process is verified using the comand 'file' be sure your embedded system have this program. (if not exec 'sudo apt-get install file')


//Program version
const char version[] = "1.0";

//default address to verify network access
const char default_check_addr[] = "google.com";

//Delay before enable watch dog system
const int delay_before_wd = 30;

//Check connection time base
const int tb_chk_net = 20;

//Check PID time base
const int tb_chk_pid = 30;

//Decition ttime base
const int tb_decition = 10;


//Time before soft restart
#define TIME2RESTART 1800
const int time2Restart = TIME2RESTART;

//Time before stop feeding (~seconds)
const int time2StopFeed = TIME2RESTART+90;


#define TRUE  1
#define FALSE 0

//If you want to dissable the check activity functionality (maybe your process sepend much time sleeping, or blocked reading a socket) coment this line
#define __CHECK_PID_ACTIVITY

#ifdef __CHECK_PID_ACTIVITY
const int tb_chk_pid_active = 120;
#endif

int main(int argc, char **argv)
{
    //Status message
    printf("\nnetdog\nversion: %s\n\nRunning....\n", version);



    ///////////////
    //Get arguments

    //Get addr to check net access
    char check_addr[100];
    if(argc > 1)
        sprintf(check_addr, "%s", argv[1]);
    else
        sprintf(check_addr, "%s", default_check_addr);

    //Get comand to execute main system program
    int check_program = FALSE;
    char exec_cmd[150];
    if(argc > 2)
    {
        sprintf(exec_cmd, "%s", argv[2]);
        check_program = TRUE;
    }
    else
        exec_cmd[0] = '\0';




    ////////////////
    //Execute comand
    int check_pid = 0;  //this variable will have the launched proccess id
    if(check_program == TRUE)
    {
        //First verify the comand actually goes to a executable file


        //Get the file or comand section
        char comand[strlen(exec_cmd)];
        int cmd_size = strlen(exec_cmd);
        int tmsize;

        tmsize = (int)(strstr(exec_cmd," ")-exec_cmd);
        if((tmsize > 0) && (tmsize < cmd_size))
            cmd_size = tmsize;

        tmsize = (int)(strstr(exec_cmd,"&")-exec_cmd);
        if((tmsize > 0) && (tmsize < cmd_size))
            cmd_size = tmsize;

        tmsize = (int)(strstr(exec_cmd,">")-exec_cmd);
        if((tmsize > 0) && (tmsize < cmd_size))
            cmd_size = tmsize;

        tmsize = (int)(strstr(exec_cmd,"|")-exec_cmd);
        if((tmsize > 0) && (tmsize < cmd_size))
            cmd_size = tmsize;

        strcpy(comand,exec_cmd);
        comand[cmd_size] = '\0';

        printf("Checking comand: '%s'\n",comand);

        char cmd[strlen(exec_cmd)+100];
        sprintf(cmd, "file %s | grep executable >/dev/null 2>&1", comand);
        int err = system(cmd);

        sprintf(cmd, "file `whereis %s | awk '{print $2}'` | grep executable >/dev/null 2>&1", comand);
        int err2 = system(cmd);

        if((err != 0) && (err2 != 0))
        {
            fprintf(stderr, "Error. The comand to execute doesn't lead to an executable file. ('%s')\nProcess verification module disable.(err1:%d/err2:%d)\n",exec_cmd,err,err2);
            check_program = FALSE;
        }
        else
        {
            //Execute the comand in the background and catch its pid


            //Before execute the comand, we need to detect if the stdout and stderr are being redirected, and if not, redirect them to /dev/null
            int red_stdout = FALSE;
            int red_stderr = FALSE;

            //first check if the comand contain the redirect chars "&>" that means both, stdout and stderr are being redirected
            if(strstr(exec_cmd,"&>") != NULL)
            {
                red_stdout = TRUE;
                red_stderr = TRUE;
            }
            else
            {
                //check if the comand contain the redirect chars "2>" that means stderr is being redirected
                if(strstr(exec_cmd,"2>") != NULL)
                    red_stderr = TRUE;
                else
                {
                    //check if the comand contain the redirect chars ">" that means stdout is being redirected
                    if(strstr(exec_cmd,">") != NULL)
                        red_stdout = TRUE;
                }
                //check if the comand contain the redirect chars "1>" that means stdout is being redirected
                if(strstr(exec_cmd,"1>") != NULL)
                    red_stdout = TRUE;
            }
            //red_stderr contain if the stderr is being redirected
            //red_stdout contain if the stdout is being redirected


            //Assamble comand (redirect to /dev/NULL the not redirected stdxxx
            char cmd[strlen(exec_cmd)+30];
            sprintf(cmd, "%s", exec_cmd);
            if(red_stdout == FALSE)
                sprintf(cmd, "%s 1>/dev/null", cmd);
            if(red_stderr == FALSE)
                sprintf(cmd, "%s 2>/dev/null", cmd);
            sprintf(cmd, "%s & echo $!", cmd);

            printf("Executing:\n>%s\n", cmd);
            FILE *fp = popen( cmd, "r");
            if(fp == NULL)
            {
                fprintf(stderr, "Error. Executing the comand '%s'.\nProcess verification module disable.\n",cmd);
                check_program = FALSE;
            }
            else
            {
                //Read pid in str
                char buff[20];
                fgets(buff, 20, fp);
                //This pipe is not closed because this process should keep running and if i close the pipe, the executed comand will be closed

                //Interpret pid as a int
                check_pid = strtol(buff, NULL, 0);

                //Status
                if(check_pid == 0)
                    printf("PID readed: '0'. Verification module dissable.\n");
                else
                    printf("Process verification module enable. (PID: %d)\n",check_pid);
            }
        }
    }

    //At this point the var 'check_program' contains if there are a proccess running to surveil
    //and if tere is, the var 'check_program' contains the pid of the process to surveil





    ////////////////////
    //Sleep a while before enable the watchdog system
    sleep(delay_before_wd);





    ////////////////
    //Enable watch dog
    enable_wd();





    ///////////////
    //Goes to main loop

    //Feed flag
    int feed = TRUE;
    //Time failing
    int last_time_ok = (int)time(NULL);

    //Connection OK
    int connection_ok = TRUE;
    //Process OK
    int pid_alive   = TRUE;
    int pid_active  = TRUE;
    for(;;)
    {



        //-------------
        //Check connection module
        {
            static int mod_time = 0;
            int c_time = (int)time(NULL);
            if(c_time >= mod_time)
            {
                //Time base
                mod_time = c_time + tb_chk_net;

                //feeds dog (ping may take some time executing)
                if(feed == TRUE)
                    feed_dog();

                //Check connection
                char cmd[strlen(check_addr)+50];
                sprintf(cmd, "ping -c1 -w1 %s >/dev/null 2>&1", check_addr);
                int err = system(cmd);
                if(err != 0)
                    connection_ok = FALSE;
                else
                    connection_ok = TRUE;
            }
        }
        //-------------




        //-------------
        //Check process module
        if(check_program == TRUE)
        {
            static int mod_time = 0;
            int c_time = (int)time(NULL);
            if(c_time >= mod_time)
            {
                //Time base
                mod_time = c_time + tb_chk_pid;

                //Check if the process is running
                char cmd[50];
                sprintf(cmd, "ps %d >/dev/null 2>&1", check_pid);
                int err = system(cmd);
                if(err != 0)
                {
                    //Process is not running
                    pid_alive = FALSE;
                }
                else
                {
                    //Process is alive
                    pid_alive = TRUE;
#ifdef __CHECK_PID_ACTIVITY
                    //-------------
                    //Check process activity (this module is activated each 'tb_chk_pid_active' seconds and checs the process activity time to change)
                    {
                        static int mod_time = 0;
                        int c_time = (int)time(NULL);
                        if(c_time >= mod_time)
                        {
                            //Time base
                            mod_time = c_time + tb_chk_pid_active;

                            //Check if process is active
                            char cmd[100];
                            sprintf(cmd, "cat /proc/%d/stat | awk '{print $15}'", check_pid);

                            FILE* fp = popen(cmd, "r");
                            if(fp == NULL)
                                fprintf(stderr, "Error. Executing the comand '%s'.\nProcess activity can not be checked.\n",cmd);
                            else
                            {
                                //Read process time in str
                                char buff[20];
                                fgets(buff, 20, fp);
                                //Close pipe
                                pclose(fp);


                                static int last_process_time = -1; //First process_time readded will be 0 (this need to be diferent)
                                //Interpretn the process time
                                int process_time = strtoul(buff, NULL, 0);

                                //If time has change, then the process is active
                                if(last_process_time == process_time)
                                    pid_active = FALSE;
                                else
                                    pid_active = TRUE;

                                printf("Checking process time: tlast:%d / tnow:%d\n", last_process_time, process_time);

                                last_process_time = process_time;
                            }
                        }
                    }
                    //-------------
#endif
                }
            }
        }
        //-------------




        //-------------
        //decition module
        {
            static int mod_time = 0;
            int c_time = (int)time(NULL);
            if(c_time >= mod_time)
            {
                //Time base
                mod_time = c_time + tb_decition;


                //Check if the system is ok
                if((connection_ok == TRUE) && (pid_alive == TRUE) && (pid_active == TRUE))
                {
                    //Update the last time OK
                    last_time_ok = c_time;

                    printf("System OK. (test addr: '%s'", check_addr);
                    if(check_program == TRUE)
                        printf(" / process: '%d'", check_pid);
                    printf(")\n");
                }
                else
                {
                    //notify the failing system
                    printf("Problems detected. ");
                    if(connection_ok==FALSE)
                        printf("!connection down! ");
                    if(pid_alive==FALSE)
                        printf("!process '%d' not running! ", check_pid);
                    else if(pid_active==FALSE)
                        printf("!process '%d' inactive! ", check_pid);
                    printf("(failing time: %d)\n", c_time-last_time_ok);
                }

                //get failing time
                int fail_time = c_time-last_time_ok;

                //Restart system if fail time grater than 'time2Restart'
                static int restarting = FALSE;
                if((fail_time > time2Restart) && (restarting==FALSE))
                {
                    //Restart the system
                    char msg[] = "Sistem has failed for '%d' seconds.... ....Restarting system.\n";
                    fprintf(stdout, msg, fail_time);
                    fprintf(stderr, msg, fail_time);

                    restarting = TRUE;
                    system("reboot >/dev/null 2>&1");
                }

                //Stop feeding dog if fail time grater than 'time2StopFeed'
                if(fail_time > time2StopFeed)
                {
                    //Restart the system
                    char msg[] = "Sistem has failed for '%d' seconds.... ....Stoping watchdog feed.\n";
                    fprintf(stdout, msg, fail_time);
                    fprintf(stderr, msg, fail_time);

                    feed = FALSE;
                }
            }
        }
        //-------------




        //++++++++++++++
        //feeds dog
        if(feed == TRUE)
            feed_dog();
        //Go to sleep
        sleep(1);
    }

    return 0;
}







