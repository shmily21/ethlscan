#ifndef __WANGBIN_ETHLSCAN_MODULE_SRC_SCRIPT_H__
#define __WANGBIN_ETHLSCAN_MODULE_SRC_SCRIPT_H__

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>

#include <lua.hpp>



enum config_table_key_word
{
    unknown_key_word        =0,
    string_key_word         =1,
    integer_key_word        =2,
    long_integer_key_word   =3,
    double_key_word         =4,
    command_key_word        =5
};

struct config_type
{
    config_table_key_word       type_name;
    std::string                 key_name;        //string key
    std::string                 string_value;       //string value
    union
    {
        int32_t                     int32_value;
        int64_t                     int64_value;
        double                      double_value;
        float                       float_value;
    };
};


class lua_script
{
public:
    explicit lua_script(const char* lpcFileName);
    virtual ~lua_script(void) throw();

public:
    template<typename _Ty>
    int get(const char* lpcNodeName,_Ty* retval);

    int get_result(const char* lpfnName,const char* sig,...);

    inline bool valid(void) const
    { return _b_valid_; }

    int read_table_item(const char* lpsztablename
                        ,std::vector<config_type>& item_list);

protected:



private:
    const char*     _lpc_filename;
    bool            _b_valid_;
    lua_State*      _lua_State;
};


#endif /* __WANGBIN_ETHLSCAN_MODULE_SRC_SCRIPT_H__ */