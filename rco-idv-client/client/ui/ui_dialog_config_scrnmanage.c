#include "ui_main.h"
#include "ui_dialog_config.h"
#include "ui_test.h"
#ifdef IDV_CLIENT
#include "../../include/application_c_interfaces.h"
#include "rc/rc_netif.h"
#endif
#include "ui_dialog_config_scrnmanage.h"

ui_save_dpi_info show_msg;
display_info_t display_info_list;
display_info_t ext_display_info_list[MAX_EXT_SCREEN_NUM];
ext_display_ini_info_t ext_display_ini_info;
custom_displayinfo_t custom_displayinfo_list = {
{
                 {
                    .width = 1024,
                    .height = 768,
                    .flag=0,
                    .custom = 1,
                 },
                 
                 {
                     .width=1280,
                     .height=720,
                     .flag=0,
                     .custom=1,
                 },

                 {
                    .width= 1280,
                    .height = 1024,
                     .flag=0,
                    .custom = 1,
                },

                {
                    .width= 1360,
                    .height = 768,
                    .flag=0,
                    .custom = 1,
                },

                {
                    .width = 1366,
                    .height = 768,
                    .flag=0,
                    .custom = 1,
                },
                {
                    .width = 1440,
                    .height= 900,
                    .flag=0,
                    .custom = 1,
                },
                {
                    .width = 1600,
                    .height = 900,
                    .flag=0,
                    .custom = 1,
                },
                 {
                    .width = 1680,
                    .height = 1050,
                    .flag=0,
                    .custom = 1,
                },
                {
                    .width = 1920,
                    .height = 1080,
                    .flag=0,
                    .custom = 1,
                },
                {
                    .width = 2560,
                    .height = 1440,
                    .flag=0,
                    .custom = 1,
                },

                },
            0,
            0,
};

//check if strike by recovery button, NOR_TRIGGER for not strike by recovery button, BUT_TRIGGER for recovery button
static int strike_change_events;
gint screen_width_original;
gint screen_height_original;
//static GtkWidget *hscale,*vscale;
static GtkWidget *ext_combo;
static GtkWidget *ext_tip_label;
static GtkWidget *combo;

static GtkWidget *ext_screen_combo1;
static GtkWidget *ext_screen_custom_combo1;
static GtkWidget *ext_screen_combo2;
static GtkWidget *ext_screen_custom_combo2;
static GtkWidget *ext_screen_tip_label;

static GtkWidget* create_menu_bar();
static gboolean extcombe_scroll_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static gboolean combe_scroll_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void combo_selected(GtkWidget *widget, gpointer window);
static void ext_combo_selected(GtkWidget *widget, gpointer window);
static void get_display_list(display_info_t *res_info);
//static void redisplay_adapt_control(int width, int height);
static void add_diaplaylist_to_combo(GtkWidget *combo);
static void show_control_extcombo(int flag);
static void add_dislpaylsit_to_extcombo();
static GtkWidget* create_attention_label();
//static int get_res_length(const char *str);
//static void sync_display_info_ext();
//static void add_display_info_ext(const EXT_RES_INDEX * ext_display_info);
static void _update_last_cur_dpi();
//static int delete_displayinfo_ini();


/* Click save to execute the action */
static int save_dpi_to_info(ui_save_dpi_info * show_combox)
{
    int opt_index = display_info_list.num;
    
    if(display_info_list.cur != opt_index) {
        if(!show_combox->custom) {
            logi("save dpi: %dx%d\n", display_info_list.res[show_combox->res_index].width, display_info_list.res[show_combox->res_index].height);
            ui_extern_set_displayinfo(&display_info_list.res[show_combox->res_index]); /* Non-custom save */
        } else {
            logi("save custom dpi: %dx%d\n", custom_displayinfo_list.res[show_combox->res_index-1].width, custom_displayinfo_list.res[show_combox->res_index-1].height);
            ui_extern_set_displayinfo(&custom_displayinfo_list.res[show_combox->res_index-1]);/* custom save */
        }
    }

    return 0;
}


/* Click the restore execution action */
void restore_last_dpi()
{
    int custom_index;
    //int ret_extindex;

    strike_change_events = BUT_TRIGGER;
    custom_index = display_info_list.num + 1;

    if (display_info_list.last == custom_index && display_info_list.cur == custom_index) {
        logi("restore last dpi: custom -> custom: %dx%d\n", custom_displayinfo_list.res[custom_displayinfo_list.last-1].width, custom_displayinfo_list.res[custom_displayinfo_list.last-1].height);
        gtk_combo_box_set_active (GTK_COMBO_BOX(ext_combo), custom_displayinfo_list.last);

    } else if (display_info_list.last == custom_index && display_info_list.cur != custom_index) {
        logi("restore last dpi: normal -> custom: %dx%d\n", custom_displayinfo_list.res[custom_displayinfo_list.last-1].width, custom_displayinfo_list.res[custom_displayinfo_list.last-1].height);
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo), display_info_list.last);
        gtk_combo_box_set_active (GTK_COMBO_BOX(ext_combo), custom_displayinfo_list.last);

    } else if (display_info_list.last != custom_index && display_info_list.cur == custom_index) {
        logi("restore last dpi: custom -> normal: %dx%d\n", display_info_list.res[display_info_list.last].width, display_info_list.res[display_info_list.last].height);
        gtk_combo_box_set_active (GTK_COMBO_BOX(ext_combo), 0);
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo), display_info_list.last);

    } else if (display_info_list.last != custom_index && display_info_list.cur != custom_index) {
        logi("restore last dpi: normal -> normal: %dx%d\n", display_info_list.res[display_info_list.last].width, display_info_list.res[display_info_list.last].height);
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo), display_info_list.last);
    }

    _update_last_cur_dpi();
}

static void ext_tip_hide()
{
    if (ext_display_info_list[0].cur != ext_display_info_list[0].num + 1 \
            && ext_display_info_list[1].cur != ext_display_info_list[1].num + 1) {
        gtk_widget_hide(ext_screen_tip_label);
    }
}

static void show_control_extcombo(int is_show_extcombo)
{
    if (is_show_extcombo == EXT_COMBO_SHOW) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(ext_combo), 0);
        gtk_widget_show(ext_combo);
        gtk_widget_show(ext_tip_label);
    } else if (is_show_extcombo == EXT_COMBO_HIDE) {
        gtk_widget_hide(ext_combo);
        gtk_widget_hide(ext_tip_label);
    } else {
        logi("unknown ext combo flag\n");
    }
}

static void show_control_ext_custom_combo(int is_which_screen, int is_show_extcombo)
{
    if (is_which_screen == EXT_VGA && is_show_extcombo == EXT_COMBO_SHOW) {
        gtk_widget_show(ext_screen_custom_combo1);
        gtk_widget_show(ext_screen_tip_label);
    } else if (is_which_screen == EXT_VGA && is_show_extcombo == EXT_COMBO_HIDE) {
        gtk_widget_hide(ext_screen_custom_combo1);
        ext_tip_hide();
    } else {
        logi("unknown show_control_ext_custom_combo flag\n");
    }

    if (is_which_screen == EXT_HDMI && is_show_extcombo == EXT_COMBO_SHOW) {
        gtk_widget_show(ext_screen_custom_combo2);
        gtk_widget_show(ext_screen_tip_label);
    } else if (is_which_screen == EXT_HDMI && is_show_extcombo == EXT_COMBO_HIDE) {
        gtk_widget_hide(ext_screen_custom_combo2);
        ext_tip_hide();
    } else {
        logi("unknown show_control_ext_custom_combo flag\n");
    }
}

#if 0
static void sync_display_info_ext()
{
	int i = 0;
	int j = 0;
	for(i = 0; i < display_info_ext.num; i++) {
		for(j = 0; j < display_info_new.num; j++)
		{
			if(display_info_new.res[j].height == display_info_ext.res[i].height && display_info_new.res[j].width == display_info_ext.res[i].width)
			  display_info_new.res[j].custom = 1;
		}
}
#endif

/* 将自定义分辨率存入数组中 */
#if 0
static void add_display_info_ext(const EXT_RES_INDEX * ext_display_info)
{
	int i= 0;

    if (ext_display_info == NULL) {
        return;
    }
	for(i=0; i<EXT_DISPLAY_NUM; i++)
	{
		display_info_ext.res[i] =ext_display_info[i].res;
		logi("dispaly_info_ext is  %d, %dx%d, %d\n",i,display_info_ext.res[i].width, display_info_ext.res[i].height, display_info_ext.res[i].custom);
	}
    display_info_ext.num = i;
}
#endif

static void _update_last_cur_dpi()
{
    logi("update last and cur dpi");
    logi(">> last dpi: combo = %d, ext_combo = %d\n", display_info_list.last, custom_displayinfo_list.last);
    logi(">> cur dpi: combo = %d, ext_combo = %d\n", display_info_list.cur, custom_displayinfo_list.cur);
    custom_displayinfo_list.last = custom_displayinfo_list.cur;
    custom_displayinfo_list.cur = gtk_combo_box_get_active(GTK_COMBO_BOX(ext_combo));
    display_info_list.last = display_info_list.cur;
    display_info_list.cur = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    logi("<< last dpi: combo = %d, ext_combo = %d\n", display_info_list.last, custom_displayinfo_list.last);
    logi("<< cur dpi: combo = %d, ext_combo = %d\n", display_info_list.cur, custom_displayinfo_list.cur);
}

static void change_to_custom_resolution(int index)
{    
    int ret = 0;
    int height = 0;
    int width = 0;
    int custom = 0;

    if(ui_extern_is_display_info_section_exist("resolution_info")) {
        ui_extern_get_displayinfo(&width, &height, &custom);
    } else {
        width = display_info_list.res[display_info_list.best_opt].width;
        height = display_info_list.res[display_info_list.best_opt].height;
    }


    logi("ui_extern_show_dialog_tips_dpi is in\n");
    ret = ui_extern_set_resolution(custom_displayinfo_list.res[index - 1].width, custom_displayinfo_list.res[index - 1].height);
    if (ret == 0) {
        redisplay_adapt_control(custom_displayinfo_list.res[index - 1].width, custom_displayinfo_list.res[index - 1].height);
        show_msg.res_index = index;
        show_msg.custom = USER_DEFINE;
        if( strike_change_events  == NOR_TRIGGER) {
            if((width != custom_displayinfo_list.res[index - 1].width) || (height != custom_displayinfo_list.res[index - 1].height)) {
                //_update_last_cur_dpi();
                ui_extern_show_dialog_tips_dpi(save_dpi_to_info, &show_msg, restore_last_dpi, NULL);
            } else {
                save_dpi_to_info(&show_msg);//1)if resolution have same height and same width,but they have different type.2)if the last index is opt_index and cur index is best opt in custom list. 
            }
        } else {
            save_dpi_to_info(&show_msg); //if the cur index is best opt index but the last is other index.
            strike_change_events = NOR_TRIGGER;
        }
    } else {
        logi("set Resolution is failed\n");
    }
}


static void ext_combo_selected(GtkWidget *widget, gpointer window)
{

    logi("ext_combo_selected is in\n");
    int index = 0;

    index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    logi("res_index == %d\n", index);

    if (index == 0) {
        //show_control_extcombo(EXT_COMBO_HIDE);
    } else {
        _update_last_cur_dpi();
        change_to_custom_resolution(index);
    }
}
#if 0
static int delete_displayinfo_ini()
{
    int opt_index = display_info_list.num;
    int res_delete_file = 0;
    display_info_list.cur = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    if(display_info_list.cur == opt_index) {
        res_delete_file = ui_extern_delete_display_info_ini();
        if(res_delete_file == TRUE) {
            logi("delete file success\n");
            return 0;
        } else {
            logi("delete file failed\n");
        }
    }
    return 0;
}
#endif
static int delete_resolution_info_section()
{
    int opt_index = display_info_list.num;
    int res_delete_file = 0;
    display_info_list.cur = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    if(display_info_list.cur == opt_index) {
        res_delete_file = ui_extern_delete_resolution_info_section();
        if(res_delete_file == TRUE) {
            logi("delete_resolution_info_section success\n");
            return 0;
        } else {
            logi("delete_resolution_info_section failed\n");
        }
    }
    return 0;
}


static void change_to_resolution(int res_index)
{
    int width = 0;
    int height = 0;
    int custom = 0;
    int ret = 0;
    
    logi("change_to_custom_resolution's display_info_list.last is %d\n",display_info_list.last);
    
    if(ui_extern_is_display_info_section_exist("resolution_info")) {
        ui_extern_get_displayinfo(&width, &height, &custom);
    } else {
        width = display_info_list.res[display_info_list.best_opt].width;
        height = display_info_list.res[display_info_list.best_opt].height;
    }

    logi("file width = %d,file height = %d, new_width = %d, new_height = %d\n", width,height,display_info_list.res[res_index].width, display_info_list.res[res_index].height);
    ret = ui_extern_set_resolution(display_info_list.res[res_index].width, display_info_list.res[res_index].height);
    if (ret == 0) {
        redisplay_adapt_control(display_info_list.res[res_index].width, display_info_list.res[res_index].height);
        show_msg.res_index = res_index;
        show_msg.custom = SYSTEM_PROVIDED;
        if (strike_change_events == NOR_TRIGGER) {
            if((width != display_info_list.res[res_index].width) || (height != display_info_list.res[res_index].height)) {     
                //_update_last_cur_dpi(); /* 更改分辨率时确定cur与last */
                ui_extern_show_dialog_tips_dpi(save_dpi_to_info, &show_msg, restore_last_dpi, NULL);
            } else {
                save_dpi_to_info(&show_msg);//1)if resolution have same height and same width,but they have different type.2)if the last index is opt_index and cur index is best opt int system list.
            }
        } else {
            save_dpi_to_info(&show_msg);// if the cur index is best opt index but the last is other index.
            strike_change_events = NOR_TRIGGER;
        }
    } else {
        logi("set Resolution is failed\n");
    }
}

static void combo_selected(GtkWidget *widget, gpointer window)
{
    int index = 0;
    int opt_index = display_info_list.num;
    int custom_index = display_info_list.num + 1;

    index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if(index != custom_index) {
        _update_last_cur_dpi();
    }
    if (index == custom_index) {
        show_control_extcombo(EXT_COMBO_SHOW);
    } else if (index == opt_index) {
        show_control_extcombo(EXT_COMBO_HIDE);
        change_to_resolution(display_info_list.best_opt);
        delete_resolution_info_section();
    } else {
        show_control_extcombo(EXT_COMBO_HIDE);
        change_to_resolution(index);
    }
}

static void ext_screen_combo_selected(GtkWidget *widget, gpointer window)
{
    int index = 0;
    int opt_index = 0;
    int custom_index = 0;

    index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if (widget == ext_screen_combo1) {
        opt_index = ext_display_info_list[0].num;
        custom_index = ext_display_info_list[0].num + 1;
        ext_display_info_list[0].cur = index;
        if (index == custom_index) {
            show_control_ext_custom_combo(EXT_VGA, EXT_COMBO_SHOW);
        } else if (index == opt_index) {
            show_control_ext_custom_combo(EXT_VGA, EXT_COMBO_HIDE);
            ui_extern_set_ext_displayinfo(&ext_display_info_list[0].res[ext_display_info_list[0].best_opt], 0);
        } else {
            show_control_ext_custom_combo(EXT_VGA, EXT_COMBO_HIDE);
            ui_extern_set_ext_displayinfo(&ext_display_info_list[0].res[index], 0);
        }
    } else if (widget == ext_screen_combo2) {
        opt_index = ext_display_info_list[1].num;
        custom_index = ext_display_info_list[1].num + 1;
        ext_display_info_list[1].cur = index;
        if (index == custom_index) {
            show_control_ext_custom_combo(EXT_HDMI, EXT_COMBO_SHOW);
        } else if (index == opt_index) {
            show_control_ext_custom_combo(EXT_HDMI, EXT_COMBO_HIDE);
            ui_extern_set_ext_displayinfo(&ext_display_info_list[1].res[ext_display_info_list[1].best_opt], 1);
        } else {
            show_control_ext_custom_combo(EXT_HDMI, EXT_COMBO_HIDE);
            ui_extern_set_ext_displayinfo(&ext_display_info_list[1].res[index], 1);
        }
    }

}

static void ext_screen_custom_combo_selected(GtkWidget *widget, gpointer window)
{
    int index = 0;

    index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if (index == 0) {
        return;
    }
    if (widget == ext_screen_custom_combo1) {
        ui_extern_set_ext_displayinfo(&custom_displayinfo_list.res[index-1], 0);
    } else if (widget == ext_screen_custom_combo2) {
        ui_extern_set_ext_displayinfo(&custom_displayinfo_list.res[index-1], 1);
    }

}

static gboolean combe_scroll_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    /* Stop emission on current widget. Default handler also not called */
    g_signal_stop_emission_by_name(widget,"scroll-event");
    return FALSE;
}

/*停止信号的发射*/
static gboolean extcombe_scroll_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    /* Stop emission on current widget. Default handler also not called */
    g_signal_stop_emission_by_name(widget,"scroll-event");

    /* Return FALSE to propagate the event further; thus scroll window will scroll
       If TRUE returned other handlers not invoked for this event,
       thus no scroll on scroll window */

    return FALSE;
}

static void add_ext_diaplaylist_to_combo()
{
    int i = 0;
    char res_str[32] = {0,};

    for (i = 0; i < ext_display_info_list[0].num ; i++) {
        memset(res_str, 0, 32);
        sprintf(res_str, "%dx%d",ext_display_info_list[0].res[i].width,ext_display_info_list[0].res[i].height);
        gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_combo1), res_str);
        if (ext_display_info_list[0].res[i].flag & IS_RES_CURRENT) {
            ext_display_info_list[0].cur = i;
        }
        if (ext_display_info_list[0].res[i].flag & IS_RES_BEST) {
            ext_display_info_list[0].best_opt = i;
        }

        if (ext_display_ini_info.res_info.width[0] == ext_display_info_list[0].res[i].width 
                && ext_display_ini_info.res_info.height[0] == ext_display_info_list[0].res[i].height) {
            ext_display_ini_info.cur[0] = i;
            ext_display_ini_info.is_exist[0] = 1;
        }
    }

    for (i = 0; i < ext_display_info_list[1].num ; i++) {
        memset(res_str, 0, 32);
        sprintf(res_str, "%dx%d",ext_display_info_list[1].res[i].width,ext_display_info_list[1].res[i].height);
        gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_combo2), res_str);
        if (ext_display_info_list[1].res[i].flag & IS_RES_CURRENT) {
            ext_display_info_list[1].cur = i;
        }
        if (ext_display_info_list[1].res[i].flag & IS_RES_BEST) {
            ext_display_info_list[1].best_opt = i;
        }

        if (ext_display_ini_info.res_info.width[1] == ext_display_info_list[1].res[i].width 
                && ext_display_ini_info.res_info.height[1] == ext_display_info_list[1].res[i].height) {
            ext_display_ini_info.cur[1] = i;
            ext_display_ini_info.is_exist[1] = 1;
        }
    }

    sprintf(res_str, "最佳分辨率");
    gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_combo1), res_str);
    memset(res_str, 0, 32);
    sprintf(res_str, "自定义");
    gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_combo1), res_str);

    memset(res_str, 0, 32);
    sprintf(res_str, "最佳分辨率");
    gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_combo2), res_str);
    memset(res_str, 0, 32);
    sprintf(res_str, "自定义");
    gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_combo2), res_str);
}

static void add_ext_dislpaylsit_to_custom_combo()
{
    int i = 0;
    char res_str[32] = {0,};
    /* 自定义分辨率 */
    gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_custom_combo1), "请选择分辨率");
    gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_custom_combo2), "请选择分辨率");
    for (i = 0; i < EXT_DISPLAY_NUM; i++) {
        memset(res_str,0,32);
        sprintf(res_str, "%dx%d", custom_displayinfo_list.res[i].width, custom_displayinfo_list.res[i].height);
        gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_custom_combo1),res_str);
        gtk_combo_box_append_text(GTK_COMBO_BOX(ext_screen_custom_combo2),res_str);

        if (ext_display_ini_info.res_info.width[0] == custom_displayinfo_list.res[i].width 
                && ext_display_ini_info.res_info.height[0] == custom_displayinfo_list.res[i].height) {
            ext_display_ini_info.custom_cur[0] = i + 1;
        }
        if (ext_display_ini_info.res_info.width[1] == custom_displayinfo_list.res[i].width 
                && ext_display_ini_info.res_info.height[1] == custom_displayinfo_list.res[i].height) {
            ext_display_ini_info.custom_cur[1] = i + 1;
        }
    }
    logi("custom_cur[0] = %d, custom_cur[1] = %d\n", ext_display_ini_info.custom_cur[0], ext_display_ini_info.custom_cur[1]);
}

static void add_dislpaylsit_to_extcombo()
{
    int i = 0;
    char res_str[32] = {0,};
    /* 自定义分辨率 */
    gtk_combo_box_append_text(GTK_COMBO_BOX(ext_combo), "请选择分辨率");
    for (i = 0; i < EXT_DISPLAY_NUM; i++) {
        memset(res_str,0,32);
        sprintf(res_str, "%dx%d", custom_displayinfo_list.res[i].width, custom_displayinfo_list.res[i].height);
        gtk_combo_box_append_text(GTK_COMBO_BOX(ext_combo),res_str);
    }
}

static void add_diaplaylist_to_combo(GtkWidget *combo)
{
    int i = 0;
    char res_str[32] = {0,};

    for (i = 0; i < display_info_list.num ; i++) {
        memset(res_str, 0, 32);
        sprintf(res_str, "%dx%d",display_info_list.res[i].width,display_info_list.res[i].height);
        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), res_str);
        if (display_info_list.res[i].flag & IS_RES_CURRENT) {
            display_info_list.cur = i;
        }
        if (display_info_list.res[i].flag & IS_RES_BEST) {
            display_info_list.best_opt = i;
        }
    }
    sprintf(res_str, "最佳分辨率");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), res_str);
    memset(res_str, 0, 32);
    sprintf(res_str, "自定义");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), res_str);
}

static void get_display_list(display_info_t * res_info_new)
{
    int size = 0;

    size = ui_extern_get_resolution_list(res_info_new->res, MAX_DISPLAY_NUM);
    if(size == 0) {
        logi(" store resolution list error\n");
        return;
    } 
    res_info_new->num = size;
}

static void init_display_combo()
{
    //combo
    memset(&display_info_list, 0, sizeof(display_info_t));
    get_display_list(&display_info_list);
    add_diaplaylist_to_combo(combo);
    //add_display_info_ext(EXT_DISPLAY_INFO);
    //sync_display_info_ext();
    if(!ui_extern_is_display_info_section_exist("resolution_info")) {
        display_info_list.cur = display_info_list.num;
    }
    display_info_list.last = display_info_list.cur;

    //ext_combo
    add_dislpaylsit_to_extcombo();
    custom_displayinfo_list.cur = 0;
    custom_displayinfo_list.last = custom_displayinfo_list.cur;

    strike_change_events = NOR_TRIGGER;
}

static void ext_display_combo_set()
{
    logi("is_exist[0]= %d, is_exist[1]= %d, custom[0] = %d, custom[1] = %d\n", ext_display_ini_info.is_exist[0], ext_display_ini_info.is_exist[1], ext_display_ini_info.res_info.custom[0], ext_display_ini_info.res_info.custom[1]);
    if (ext_display_ini_info.is_exist[0] == 0 && ext_display_ini_info.res_info.custom[0] != 1) {
        ext_display_ini_info.cur[0] = ext_display_info_list[0].num;
        ui_extern_set_ext_displayinfo(&ext_display_info_list[0].res[ext_display_info_list[0].best_opt], 0);
    }
    if (ext_display_ini_info.is_exist[1] == 0 && ext_display_ini_info.res_info.custom[1] != 1) {
        ext_display_ini_info.cur[1] = ext_display_info_list[1].num;
        ui_extern_set_ext_displayinfo(&ext_display_info_list[1].res[ext_display_info_list[1].best_opt], 1);
    }

    logi("cur[0]= %d, cur[1]= %d, custom[0] = %d, custom[1] = %d\n", ext_display_ini_info.cur[0], ext_display_ini_info.cur[1], ext_display_ini_info.res_info.custom[0], ext_display_ini_info.res_info.custom[1]);
    if (ext_display_ini_info.res_info.custom[0] == 1) {
        gtk_widget_show(ext_screen_custom_combo1);
        gtk_widget_show(ext_tip_label);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ext_screen_combo1), ext_display_info_list[0].num + 1);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ext_screen_custom_combo1), ext_display_ini_info.custom_cur[0]);
    } else {
        gtk_widget_hide(ext_screen_custom_combo1);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ext_screen_combo1), ext_display_ini_info.cur[0]);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ext_screen_custom_combo1), 0);
    }

    if (ext_display_ini_info.res_info.custom[1] == 1) {
        gtk_widget_show(ext_screen_custom_combo2);
        gtk_widget_show(ext_tip_label);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ext_screen_combo2), ext_display_info_list[1].num + 1);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ext_screen_custom_combo2), ext_display_ini_info.custom_cur[1]);
    } else {
        gtk_widget_hide(ext_screen_custom_combo2);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ext_screen_combo2), ext_display_ini_info.cur[1]);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ext_screen_custom_combo2), 0);
    }
}

static int get_ext_display_list(display_info_t *res_info_new)
{
    int size = 0, i;
    int index = 0;

    for (i = 1; i <= ui_extern_get_display_port_number(); i++) {
        size = ui_extern_get_display_resolution_list(i, res_info_new[index].res, MAX_DISPLAY_NUM);
        if(size == 0) {
            logi(" store resolution list error\n");
            continue;
        }
        res_info_new[index].num = size;
        index++;

        if (index >= MAX_EXT_SCREEN_NUM) {
            break;
        }
    }

    return index;
}

static void init_ext_display_combo()
{
    memset(&ext_display_ini_info, 0, sizeof(ext_display_ini_info_t));
    ui_extern_get_ext_displayinfo(&ext_display_ini_info.res_info);
    add_ext_diaplaylist_to_combo();
    add_ext_dislpaylsit_to_custom_combo();
    ext_display_combo_set();
    logi("%dx%d, %dx%d\n", ext_display_ini_info.res_info.width[0], ext_display_ini_info.res_info.height[0], ext_display_ini_info.res_info.width[1], ext_display_ini_info.res_info.height[1]);
}

static GtkWidget* create_attention_label()
{
    GtkWidget *att_hbox;
    
    att_hbox = gtk_hbox_new(FALSE,0);
    ext_tip_label = gtk_label_new ("注：自定义分辨率用于调整中控设备,请确认中控设备所接屏幕支持该分辨率");
    gtk_label_set_markup(GTK_LABEL(ext_tip_label), "<span foreground='#F5A623' font_desc='10' >            注：自定义分辨率用于调整中控设备,请确认中控设备所接屏幕支持该分辨率</span>");

    gtk_widget_hide(ext_tip_label);
    
    gtk_box_pack_start(GTK_BOX(att_hbox), ext_tip_label, FALSE, FALSE, 10);
    gtk_widget_show(att_hbox);
    return att_hbox;
}

static GtkWidget* create_menu_bar()
{
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *align;
    strike_change_events = NOR_TRIGGER;

    /*添加容器*/
    hbox = gtk_hbox_new(FALSE,0);
    label = gtk_label_new("屏幕分辨率");
    gtk_label_set_markup(GTK_LABEL(label), "<span foreground='#000000' font_desc='10' >  屏幕分辨率:</span>");
    gtk_box_pack_start((GtkBox*)hbox,label,FALSE,FALSE,5);
    gtk_widget_show(label);

    combo = gtk_combo_box_new_text();
    gtk_box_pack_start((GtkBox*)hbox,combo,FALSE,FALSE,5);
    gtk_widget_show(combo);

    ext_combo = gtk_combo_box_new_text();
    gtk_box_pack_start((GtkBox*)hbox,ext_combo,TRUE,TRUE,5);
    gtk_widget_hide(ext_combo);

    init_display_combo();

    if(ui_extern_is_display_info_section_exist("resolution_info")) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo), display_info_list.cur);
    } else {
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo), display_info_list.num);//set active the best resolution
    }
    gtk_combo_box_set_active (GTK_COMBO_BOX(ext_combo),0);



    g_signal_connect(combo, "scroll-event", G_CALLBACK(combe_scroll_handler), NULL);
    g_signal_connect(ext_combo, "scroll-event", G_CALLBACK(extcombe_scroll_handler), NULL);
    g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_selected), NULL);
    g_signal_connect(G_OBJECT(ext_combo), "changed", G_CALLBACK(ext_combo_selected), NULL);

    gtk_widget_show(hbox);
    align = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show(align);
    return align;
}

static GtkWidget* create_ext_title_label()
{
    GtkWidget *att;
    GtkWidget *title_label;

    title_label = gtk_label_new("扩展屏分辨率");
    gtk_label_set_markup(GTK_LABEL(title_label), "<span foreground='#000000' font_desc='10' >扩展屏分辨率:</span>");
    gtk_widget_show(title_label);

    att = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(att), title_label);
    gtk_widget_show(att);
    return att;
}

static GtkWidget* create_ext_tip_label()
{
    GtkWidget *att_hbox;
    
    att_hbox = gtk_hbox_new(FALSE,0);
    ext_screen_tip_label = gtk_label_new ("注：自定义分辨率用于调整中控设备,请确认中控设备所接屏幕支持该分辨率");
    gtk_label_set_markup(GTK_LABEL(ext_screen_tip_label), "<span foreground='#F5A623' font_desc='10' >            注：自定义分辨率用于调整中控设备,请确认中控设备所接屏幕支持该分辨率</span>");

    gtk_widget_hide(ext_screen_tip_label);
    
    gtk_box_pack_start(GTK_BOX(att_hbox), ext_screen_tip_label, FALSE, FALSE, 10);
    gtk_widget_show(att_hbox);
    return att_hbox;
}

static GtkWidget* create_ext_menu_bar()
{
    GtkWidget *hbox1, *hbox2;
    GtkWidget *vbox;
    GtkWidget *align;
    GtkWidget *label1, *label2;

    /*添加容器*/
    vbox = gtk_vbox_new(FALSE,0);

    hbox1 = gtk_hbox_new(FALSE,0);
    label1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label1), "<span foreground='#000000' font_desc='10' >                屏幕1:</span>");
    gtk_box_pack_start((GtkBox*)hbox1,label1,FALSE,FALSE,5);
    gtk_widget_show(label1);
    
    ext_screen_combo1 = gtk_combo_box_new_text();
    gtk_box_pack_start((GtkBox*)hbox1,ext_screen_combo1,FALSE,FALSE,5);
    gtk_widget_show(ext_screen_combo1);

    ext_screen_custom_combo1 = gtk_combo_box_new_text();
    gtk_box_pack_start((GtkBox*)hbox1,ext_screen_custom_combo1,TRUE,TRUE,5);
    gtk_widget_hide(ext_screen_custom_combo1);

    hbox2 = gtk_hbox_new(FALSE,0);
    label2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label2), "<span foreground='#000000' font_desc='10' >                屏幕2:</span>");
    gtk_box_pack_start((GtkBox*)hbox2,label2,FALSE,FALSE,5);
    gtk_widget_show(label2);

    ext_screen_combo2 = gtk_combo_box_new_text();
    gtk_box_pack_start((GtkBox*)hbox2,ext_screen_combo2,FALSE,FALSE,5);
    gtk_widget_show(ext_screen_combo2);

    ext_screen_custom_combo2 = gtk_combo_box_new_text();
    gtk_box_pack_start((GtkBox*)hbox2,ext_screen_custom_combo2,TRUE,TRUE,5);
    gtk_widget_hide(ext_screen_custom_combo2);

    init_ext_display_combo();

    g_signal_connect(G_OBJECT(ext_screen_combo1), "changed", G_CALLBACK(ext_screen_combo_selected), NULL);
    g_signal_connect(G_OBJECT(ext_screen_combo2), "changed", G_CALLBACK(ext_screen_combo_selected), NULL);
    g_signal_connect(G_OBJECT(ext_screen_custom_combo1), "changed", G_CALLBACK(ext_screen_custom_combo_selected), NULL);
    g_signal_connect(G_OBJECT(ext_screen_custom_combo2), "changed", G_CALLBACK(ext_screen_custom_combo_selected), NULL);

    gtk_box_pack_start((GtkBox*)vbox,hbox1,FALSE,FALSE,10);
    gtk_box_pack_start((GtkBox*)vbox,hbox2,FALSE,FALSE,10);
    gtk_widget_show(hbox1);
    gtk_widget_show(hbox2);
    gtk_widget_show(vbox);
    align = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), vbox);
    gtk_widget_show(align);
    return align;
}

#if 0
void scale_set_default_values(GtkScale * scale)
{
	gtk_range_set_update_policy(GTK_RANGE(scale),GTK_UPDATE_CONTINUOUS);
	gtk_scale_set_digits(scale,1);
	gtk_scale_set_value_pos(scale,GTK_POS_TOP);
	gtk_scale_set_draw_value(scale,TRUE);
}

void cb_digits_scale(GtkAdjustment *adj)

{
	gtk_scale_set_digits(GTK_SCALE(hscale),(gint)adj->value);
	gtk_scale_set_digits(GTK_SCALE(vscale),(gint)adj->value);
}
static GtkWidget * mk_lum_area()
{
	GtkWidget *align;
	GtkWidget *vbox;
	GtkWidget *label;
	//GtkWidget *scale;
	GtkWidget *adj1;

	vbox = gtk_hbox_new(FALSE,5);
	label = gtk_label_new("屏幕亮度调节:");
	gtk_label_set_markup(GTK_LABEL(label), "<span foreground='#666666' font_desc='12' >屏幕亮度调节:</span>");
	set_widget_font_size(label, 14, NULL);
	gtk_widget_show(label);

	adj1 = gtk_adjustment_new(0.0,0.0,101.0,0.1,1.0,1.0);
	g_signal_connect(G_OBJECT(adj1),"value_changed",G_CALLBACK(cb_digits_scale),NULL);
	hscale = gtk_hscale_new(GTK_ADJUSTMENT(adj1));
	gtk_widget_set_size_request(GTK_WIDGET(hscale),200,-1);
	//gtk_widget_modify_bg(GTK_WIDGET(hscale), GTK_STATE_PRELIGHT, &color);
	gtk_widget_show(hscale);

	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hscale,FALSE,FALSE,0);
	gtk_widget_show(vbox);
	align = gtk_alignment_new(0.3,0, 0,0);
	gtk_container_add(GTK_CONTAINER(align), vbox);
	gtk_widget_show(align);
	return align;
}
#endif

GtkWidget *ui_config_scrnmanage_init(void)
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *hid_label;
    //GtkWidget *ext_tip_label1;
    
    // GtkWidget *lum_area;/*亮度调节区域*/
    GtkWidget *dpi = NULL;
    GtkWidget *ext_dpi = NULL;
    GtkWidget *attition = NULL;
    GtkWidget *ext_title = NULL;
    GtkWidget *ext_tip = NULL;

    //lum_area = mk_lum_area();
    dpi = create_menu_bar();
    attition = create_attention_label();
    ext_tip = create_ext_tip_label();

    memset(ext_display_info_list, 0, sizeof(display_info_t)*2);
    if (get_ext_display_list(ext_display_info_list) >= 2) {
        ext_title = create_ext_title_label();
        ext_dpi = create_ext_menu_bar();
    }

    hid_label = gtk_label_new("");
    gtk_widget_show(hid_label);

    vbox = gtk_vbox_new(FALSE,5);
    //gtk_box_pack_start(GTK_BOX(vbox),lum_area,FALSE,FALSE,10);
    gtk_box_pack_start(GTK_BOX(vbox), dpi, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), attition, FALSE, FALSE, 5);
    //gtk_box_pack_start(GTK_BOX(vbox), ext_tip_label, FALSE, FALSE, 0);

    if (ext_title && ext_dpi) {
        gtk_box_pack_start(GTK_BOX(vbox), ext_title, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(vbox), ext_dpi, FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(vbox), ext_tip, FALSE, FALSE, 0);
    gtk_widget_show(vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hid_label, FALSE, FALSE, 82);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox); 

    return hbox;

}

