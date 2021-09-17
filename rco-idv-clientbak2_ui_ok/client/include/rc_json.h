#ifndef RC_JSON_H__
#define RC_JSON_H__

#include <string>
#include <limits.h>

namespace RcJson
{
    enum RcJsonError
    {
        ERROR_INT = INT_MIN,
    };
    
    const std::string rc_json_get_string(const std::string& input, const std::string& key);

    int rc_json_get_int(const std::string& input, const std::string& key);

    const std::string rc_json_get_child(const std::string& input, const std::string& key);

    void rc_json_add_string(std::string& input, const std::string& key, const std::string& value);

    void rc_json_add_int(std::string& input, const std::string& key, int value);

    void rc_json_add_bool(std::string& input, const std::string& key, bool value);

    void rc_json_add_child(std::string& input, const std::string& key, const std::string& value);

}

#endif//RC_JSON_H__
