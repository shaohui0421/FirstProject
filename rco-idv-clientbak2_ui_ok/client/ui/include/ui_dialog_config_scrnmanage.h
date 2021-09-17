#ifndef UI_DIALOG_SCRNMANAGE_H
#define UI_DIALOG_SCRNMANAGE_H

#include "ui_api.h"


#define MAX_DISPLAY_NUM        35
#define IS_RES_BEST           (1<<0)
#define IS_RES_CURRENT        (1<<1)

#define USER_DEFINE             1
#define SYSTEM_PROVIDED         0

#define EXT_COMBO_SHOW          1
#define EXT_COMBO_HIDE          0

#define EXT_DISPLAY_NUM         10

#define NOR_TRIGGER             1
#define BUT_TRIGGER             0

#define EXT_VGA                 1
#define EXT_HDMI                0

typedef struct Display_info {
    ui_res_info  res[MAX_DISPLAY_NUM];  //resolution
    int last;                        //last resolution index
    int num;                         //resolution num
    int cur;                         //cur resolution index
    int best_opt;                    //the pos of best resolution in the resolution list
} display_info_t;

typedef struct Ext_Display_Ini_info {
    ui_ext_res_info res_info;
    int cur[2];
    int custom_cur[2];
    int is_exist[2];
}ext_display_ini_info_t;

typedef struct Custom_Displayinfo {
    ui_res_info res[EXT_DISPLAY_NUM];
    int cur;
    int last;
} custom_displayinfo_t;

void restore_last_dpi();


#endif 
