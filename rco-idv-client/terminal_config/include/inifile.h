#ifndef INIFILE_H
#define INIFILE_H
//#include <gtk/gtk.h>
#include <glib.h>

typedef struct _IniLine     IniLine;//定义INI文件的行
typedef struct _IniSection  IniSection;//定义INI文件的节
typedef struct _IniFile  IniFile;//定义INI文件对象

struct _IniLine
{
    gchar * key;  //键
    gchar *value; //值
};

struct _IniSection
{
    gchar * name; //节名
    GList *lines;  //链表
};

struct _IniFile
{
    gchar * filename; //文件名
    GList *sections;  //节链表
    gboolean changed;  //是否改动
};

IniSection* ini_file_create_section(IniFile* ini, gchar* name);//创建某个部分关键字
IniLine* ini_file_create_string(IniSection* section, gchar* key, gchar* value);//创建某个关键字
IniFile * ini_file_new();  //创建INI文件对象
IniFile* ini_file_open_file(gchar* filename);  //创建INI文件对象并打开文件
gboolean ini_file_write_file(IniFile* ini, gchar* filename);  //将INI文件对象保存为文件
void ini_file_free(IniFile* ini);
//以下为读INI文件的键值的函数
gboolean ini_file_read_string(IniFile* ini, gchar* section, gchar* key, gchar** value);
gboolean ini_file_read_int(IniFile* ini, gchar* section, gchar* key, gint* value);
gboolean ini_file_read_boolean(IniFile* ini, gchar* section, gchar* key, gboolean* value);
//以下为写INI文件的键值的函数
void ini_file_write_string(IniFile* ini, gchar* section, gchar* key, gchar* value);
void ini_file_write_int(IniFile* ini, gchar* section, gchar* key, gint value);
void ini_file_write_boolean(IniFile* ini, gchar* section, gchar* key, gboolean value);

void ini_file_rename_section(IniFile* ini, gchar* section, gchar* section_name);
void ini_file_remove_key(IniFile* ini, gchar* section, gchar* key);
void ini_file_remove_section(IniFile* ini, gchar* section);

gboolean ini_file_modify_boolvalue(IniFile* ini, gchar* section, gchar* key, gboolean newvalue);

#endif // INIFILE_H
























