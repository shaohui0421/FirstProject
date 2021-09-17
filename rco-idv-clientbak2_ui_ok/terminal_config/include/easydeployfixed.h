#ifndef EASYDEPLOYFIXED
#define EASYDEPLOYFIXED
#include <gtk/gtk.h>
#include <glib-object.h>

#define ICONPATH "/etc/RCC_Client/res/icon/deploy/"

G_BEGIN_DECLS

#define EASYDEPLOY_FIXED_TYPE            (easydeploy_fixed_get_type())
#define EASYDEPLOY_FIXED(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), EASYDEPLOY_FIXED_TYPE, EasydeployFixed))
#define EASYDEPLOY_FIXED_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), EASYDEPLOY_FIXED_TYPE, EasydeployFixedClass))
#define IS_EASYDEPLOY_FIXED(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), EASYDEPLOY_FIXED_TYPE))
#define IS_EASYDEPLOY_FIXED_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), EASYDEPLOY_FIXED_TYPE))
#define EASYDEPLOY_FIXED_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), EASYDEPLOY_FIXED_TYPE, EasydeployFixedClass))

typedef struct _EasydeployFixedClass EasydeployFixedClass;
typedef struct _EasydeployFixed EasydeployFixed ;

typedef struct btn_apend {
    GtkWidget *container;
    GtkWidget *image;
    GtkWidget *mouse_on;
    GtkWidget *label;
    GtkWidget *event;
} btn_apend_t;

struct _EasydeployFixed {
    GtkFixed parent;

    GtkWidget *mainFixed;

    GtkWidget *beginSerialFixed;
    btn_apend_t *closebtn1;

    GtkWidget *endSerialFixed;
    btn_apend_t *closebtn2;

    GtkWidget *waitSerialFixed;
    GtkWidget *waitSerialButton;
    GtkWidget *waitImage;
    GtkWidget *waitLable;
    btn_apend_t *closebtn3;
    guint timer;
    guint timer1;
    guint timer2;
    guint timer3;
    int wait_time;

    gulong signal_id;
    int serial_num;
    int serial_state;
    int r_x;
    int r_y;
};

struct _EasydeployFixedClass {
    GtkFixedClass parent_class;
};

enum SERIAL_NUM_STATE {
    SERIAL_NUM_INIT,          /*编号初始化*/
    SERIAL_NUM_WAITING,       /*编号等待*/
    SERIAL_NUM_SUCCESS,       /*编号成功*/
    SERIAL_NUM_ERROR,         /*编号异常*/
    SERIAL_NUM_TIMEOUT,       /*编号超时*/
};

GType easydeploy_fixed_get_type(void);
GtkWidget* easydeploy_fixed_new(void);
void easydeploy_fixed_show(GtkWidget *widget);
void easydeploy_fixed_destory(GtkWidget *widget);
void easydeploy_fixed_wait2begin(GtkWidget *widget);
void easydeploy_fixed_begin2wait(GtkWidget *widget);
void easydeploy_fixed_wait2end(GtkWidget *widget);
void easydeploy_fixed_set_num(GtkWidget *widget, int num);
void easydeploy_fixed_set_state(GtkWidget *widget, int state);
int easydeploy_fixed_get_state(GtkWidget *widget);
void easydeploy_fixed_init(EasydeployFixed *easydeployfixed);

G_END_DECLS

#endif

