#include <malloc.h>
#include <unistd.h>
#include "ui_api.h"
#include "liblog.h"
static int unconfig;
static int user;
static int login;

void wait_int(int *p)
{
    while (1) {
        sleep(5);
        if (*p) {
            break;
        }
    }
}


void ui_set_unconfig()
{
    unconfig = 1;
}

void ui_set_user()
{
    user = 1;
}

void ui_set_login()
{
    login = 1;
}


void update_progress()
{

    l2u_download_t *arg;
    int i;
#if 1
    for (i = 0; i <= 5; i++) {
        arg = malloc(sizeof(l2u_download_t));
        arg->status = 1;
        arg->process = i;
        arg->speed = NULL;
        l2u_show_download(arg);
        sleep(1);
    }
    
    for (i = 0; i <= 5; i++) {
        arg = malloc(sizeof(l2u_download_t));
        arg->status = 0;
        arg->process = i;
        arg->speed = "100MB/s";
        l2u_show_download(arg);
        sleep(1);
    }
#endif

    arg = malloc(sizeof(l2u_download_t));
    arg->status = 2;
    arg->process = 54;
    arg->speed = NULL;
    arg->err_code = 1;
    l2u_show_download(arg);
    sleep(5);

    return;

}

void print_hell (void * args)
{
	g_print("chenw here %ld\n", (long)args);
}

void ui_test_main(void)
{
    printf("test: start!\n");

    l2u_show_userbind();
    sleep(10);
    l2u_show_userlogin();

    /*
    l2u_show_easydeploy_tips(print_hell, (void *)1);
    sleep(7);
    l2u_show_vmerr_tips(print_hell, (void *)2);
    sleep(7);
    l2u_show_vm_lasterr_tips(print_hell, (void *)3);
    sleep(7);
    l2u_show_auto_shutdown();

    l2u_show_tips(11);
    sleep(3);

    l2u_show_settype();
    sleep(3);
    l2u_result_settype(1);
    sleep(10);
    l2u_result_settype(2);
    sleep(10);
*/

//    l2u_show_userlogin();


/*
    l2u_show_tips(7);
    l2u_ctrl_winbtn(FALSE);

    sleep(10);
    l2u_show_tips(8);
*/


    //l2u_show_settype();
    //init
 /*
    sleep(1);
    l2u_show_updateclient();
    sleep(10);
    l2u_result_updateclient(0);
    sleep(2);
    l2u_result_updateclient(1);

    sleep(2);

    //l2u_show_tips(7);

    l2u_show_tips(8);
*/
    /*
    l2u_show_userlogin();
    sleep(1);
    l2u_result_user(10);
    sleep(1);
    l2u_result_user(11);
    sleep(1);
    l2u_result_user(3);



    l2u_show_server_err(1);
    sleep(3);

    l2u_show_server_err(2);
    sleep(3);
    l2u_show_server_err(3);
    sleep(3);

    l2u_show_server_err(4);
    sleep(3);
    l2u_show_server_err(5);
    sleep(3);

    l2u_show_server_err(6);
    sleep(3);

    l2u_show_server_err(7);
    //l2u_show_settype();
    //l2u_show_userlogin();
    sleep(2);
    l2u_show_userbind();
    sleep(2);

    l2u_result_binduser(1);
 */
#if 0
    //l2u_show_settype();
#endif

#if 0
    l2u_result_config(0);
    sleep(2);
    l2u_result_config(1);
    sleep(2);
    l2u_result_config(2);
#endif
#if 0
    l2u_show_tips(1);
    sleep(2);
    l2u_show_tips(2);
        sleep(2);
    l2u_show_tips(4);
#endif
#if 0
    l2u_show_download(NULL);

    sleep(2);

    update_progress();

    l2u_show_imageupdating();
#endif
#if 0
    sleep(5);

    l2u_show_unconfig();

    l2u_show_imageupdate();

    sleep(10);

    l2u_show_download(NULL);
#endif
    //update_progress();
#if 0
    l2u_show_unconfig();

    wait_int(&unconfig);

    l2u_result_config(0);

    sleep(1);

    l2u_show_userlogin();
    l2u_result_user(10);
    int i;
    for (i = 0; i < 100; i++) {
        wait_int(&login);
        l2u_result_user(2);
        login = 0;
        wait_int(&login);
        l2u_result_user(1);
        login = 0;
    }

#endif

#if 0
    sleep(1);
    l2u_show_updateclient();
    sleep(1);
    l2u_show_connecting();

    sleep(1);
    l2u_result_updateclient(0);
    sleep(1);
    l2u_result_updateclient(1);
    sleep(1);
    l2u_show_publiclogin();

    sleep(1);
    l2u_show_disconnect();

    sleep(3);
    l2u_show_userlogin();

    sleep(3);

    l2u_show_connecting();

    sleep(3);
    
    l2u_show_userlogin();

    sleep(3);
    l2u_show_userbind();
#endif
#if 0
    wait_int(&login);

    update_progress();


    wait_int(&user);

    ui_io_set(UI_CTRL_SHOW_DOWNLOAD, XXX);
#endif

}

















