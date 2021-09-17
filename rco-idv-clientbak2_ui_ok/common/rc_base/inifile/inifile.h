#ifndef INIFILE_H
#define INIFILE_H
#include <glib.h>

typedef struct _IniLine     IniLine;
typedef struct _IniSection  IniSection;
typedef struct _IniFile  IniFile;

struct _IniLine
{
    gchar * key;
    gchar *value;
};

struct _IniSection
{
    gchar * name;
    GList *lines;
};

struct _IniFile
{
    gchar * filename;
    GList *sections;
    gboolean changed;
};

IniSection* ini_file_create_section(IniFile* ini, gchar* name);
IniLine* ini_file_create_string(IniSection* section, gchar* key, gchar* value);
IniFile * ini_file_new();
IniFile* ini_file_open_file(gchar* filename);
gboolean ini_file_write_file(IniFile* ini, gchar* filename);
void ini_file_free(IniFile* ini);

gboolean ini_file_read_string(IniFile* ini, gchar* section, gchar* key, gchar** value);
gboolean ini_file_read_int(IniFile* ini, gchar* section, gchar* key, gint* value);
gboolean ini_file_read_boolean(IniFile* ini, gchar* section, gchar* key, gboolean* value);

void ini_file_write_string(IniFile* ini, gchar* section, gchar* key, gchar* value);
void ini_file_write_int(IniFile* ini, gchar* section, gchar* key, gint value);
void ini_file_write_boolean(IniFile* ini, gchar* section, gchar* key, gboolean value);

void ini_file_rename_section(IniFile* ini, gchar* section, gchar* section_name);
void ini_file_remove_key(IniFile* ini, gchar* section, gchar* key);
void ini_file_remove_section(IniFile* ini, gchar* section);

gboolean ini_file_modify_boolvalue(IniFile* ini, gchar* section, gchar* key, gboolean newvalue);

#endif // INIFILE_H
























