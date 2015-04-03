#include "script.h"
#include <cassert>
#include <algorithm>

#define nullptr 0 // c++11

#define _CHECK_LUA_STACK_(_def_lua_Error) \
            ((_def_lua_Error) ? true:false)

#define _EMPTY_LUA_STACK_(_def_lua_State) \
            { \
                if(lua_gettop(_def_lua_State)) \
                    lua_settop(_def_lua_State,0); \
            }

#define lua_check_stack(param) _CHECK_LUA_STACK_(param)
#define lua_empty_stack(param) _EMPTY_LUA_STACK_(param)


template<class _Tr>
bool _get_value(const char* _N,lua_State* _lua_State, _Tr* _Rt);

template<class _Tr>
bool _get_value(const char* _N,lua_State* _lua_State, _Tr* _Rt)
{
    typedef _Tr type;
    lua_empty_stack(_lua_State);
    lua_getglobal(_lua_State,_N);
    if(!lua_isnumber(_lua_State,-1))
    {
        int isnum(0);type _retval((type)(lua_tonumberx(_lua_State,-1,&isnum)));
        if(isnum)
        {
            *_Rt=_retval;
            lua_pop(_lua_State,1);
            return true;
        }
        lua_pop(_lua_State,1);
        return false;
    }
    else
    {
        *_Rt=(type)lua_tonumber(_lua_State,-1);
        lua_pop(_lua_State,1);
        return true;
    }
}


#define _DEF_GET_VALUE_(subffix) \
    static \
    int get_lua_##subffix##_value(const char* lpNodeName,lua_State* _lua_State,subffix* _subVal) \
    { \
        return ((_get_value<subffix>(lpNodeName,_lua_State,_subVal))?0:1); \
    }

#define _get_value_(param) _DEF_GET_VALUE_(param)

_get_value_(int);
_get_value_(long);
_get_value_(float);
_get_value_(double);

lua_script::lua_script(const char* lpcszFileName)
                    :_lpc_filename(lpcszFileName)
                    ,_b_valid_(false)
                    ,_lua_State(luaL_newstate())
{
    _b_valid_=lua_check_stack(_lua_State && (0==access(lpcszFileName,0)));
    luaL_openlibs(_lua_State);
    if(_b_valid_)
    {
        if(luaL_loadfile(_lua_State,_lpc_filename)
            ||lua_pcall(_lua_State,0,0,0))
        {
            _b_valid_=false;
        }
    }
}

lua_script::~lua_script() throw()
{
    lua_close(_lua_State);
}

template< >
int lua_script::get<bool>(const char* lpcNodeName,bool* retval)
{
    assert(nullptr!=retval && nullptr!=lpcNodeName);
    if(!lua_check_stack(_lua_State && _b_valid_)) return -1;
    lua_empty_stack(_lua_State);
    lua_getglobal(_lua_State,lpcNodeName);
    *retval=(bool)lua_toboolean(_lua_State,-1);
    lua_pop(_lua_State,1);
    return 0;
}

template< >
int lua_script::get<int>(const char* lpcNodeName,int* retval)
{
    assert(nullptr!=retval && nullptr!=lpcNodeName);
    return (lua_check_stack(_lua_State && _b_valid_))?
        get_lua_int_value(lpcNodeName,_lua_State,retval):-1;
}

template< >
int lua_script::get<long>(const char* lpcNodeName,long* retval)
{
    assert(nullptr!=retval && nullptr!=lpcNodeName);
    return (lua_check_stack(_lua_State && _b_valid_))?
        get_lua_long_value(lpcNodeName,_lua_State,retval):-1;
}

template< >
int lua_script::get<float>(const char* lpcNodeName,float* retval)
{
    assert(nullptr!=retval && nullptr!=lpcNodeName);
    return (lua_check_stack(_lua_State && _b_valid_))?
        get_lua_float_value(lpcNodeName,_lua_State,retval):-1;
}

template< >
int lua_script::get<double>(const char* lpcNodeName,double* retval)
{
    assert(nullptr!=retval && nullptr!=lpcNodeName);
    return (lua_check_stack(_lua_State && _b_valid_))?
        get_lua_double_value(lpcNodeName,_lua_State,retval):-1;
}

template< >
int lua_script::get<std::string>(const char* lpcNodeName,std::string* retval)
{
    assert(nullptr!=retval && nullptr!=lpcNodeName);
    if(!lua_check_stack(_lua_State && _b_valid_)) return -1;
    lua_empty_stack(_lua_State);
    int n_return_value(-1);
    lua_getglobal(_lua_State,lpcNodeName);
    if(!lua_isstring(_lua_State,-1))
    {
        const char* lpcszStr=lua_tolstring(_lua_State,-1,NULL);
        if(lpcszStr)
        {
            *retval=std::string(lpcszStr);
            n_return_value=0;
        }else n_return_value=-1;
    }
    else
    {
        *retval=std::string(lua_tostring(_lua_State,-1));
        n_return_value=0;
    }
    lua_pop(_lua_State,1);
    return n_return_value;
}

int lua_script::get_result(const char* lpfnName,const char* sig,...)
{
    if(!lua_check_stack(_lua_State && _b_valid_)) return -1;
    lua_empty_stack(_lua_State);
    va_list vl;int narg(0),nres(0);va_start(vl,sig);
    lua_getglobal(_lua_State,lpfnName);
    while(*sig)
    {
        switch(*sig++)
        {
            case 'd':
            case 'D':lua_pushnumber(_lua_State,va_arg(vl,double));break;
            case 'i':
            case 'I':lua_pushnumber(_lua_State,va_arg(vl,int));break;
            case 's':
            case 'S':lua_pushstring(_lua_State,va_arg(vl,char*));break;
            case '>':goto __lable_End_While;
            default:lua_settop(_lua_State,0);
            return -1;
        }
        narg++;
        luaL_checkstack(_lua_State,1,"[Error] Too Many Arguments .");
    }
__lable_End_While:
    nres=std::strlen(sig);
    if(0!=lua_pcall(_lua_State,narg,nres,0))
    {
        std::fprintf(stderr, "[ERROR] Running Function '%s' With Error: %s\n",lpfnName,lua_tostring(_lua_State,-1));
        lua_settop(_lua_State,0);
        return -1;
    }
    nres=-nres;
    while(*sig)
    {
        switch(*sig++)
        {
            case 'd':
            case 'D':
                {
                    if(!lua_isnumber(_lua_State,nres))
                    {
                        lua_settop(_lua_State,0);
                        return -1;
                    }
                    *va_arg(vl,double*)=lua_tonumber(_lua_State,nres);
                }break;
            case 'i':
            case 'I':
                {
                    if(!lua_isnumber(_lua_State,nres))
                    {
                        lua_settop(_lua_State,0);
                        return -1;
                    }
                    *va_arg(vl,int*)=(int)lua_tonumber(_lua_State,nres);
                }break;
            case 's':
            case 'S':
                {
                    if(!lua_isstring(_lua_State,nres))
                    {
                        lua_settop(_lua_State,0);
                        return -1;
                    }
                    *va_arg(vl,const char**)=lua_tostring(_lua_State,nres);
                }break;
            default:
                {
                    lua_settop(_lua_State,0);
                    return -1;
                }break;
        }
        nres++;
    }
    va_end(vl);
    return 0;
}


void print_list(const config_type& config_value)
{
    switch(config_value.type_name)
    {
        case unknown_key_word:
        {
            std::fprintf(stderr, "[list] type: %s key: %s\t\t value: %s\n","unknown","unknown","unknown");
        }break;
        case string_key_word:
        {
            std::fprintf(stderr, "[list] type: %s key: %s\t\t value: %s\n","string",config_value.key_name.c_str(),config_value.string_value.c_str());
        }break;
        case integer_key_word:
        {
            std::fprintf(stderr, "[list] type: %s key: %s\t\t value: %d\n","int32_t",config_value.key_name.c_str(),(int32_t)config_value.int32_value);
        }break;
        case long_integer_key_word:
        {
            std::fprintf(stderr, "[list] type: %s key: %s\t\t value: %lld\n","int64_t",config_value.key_name.c_str(),(int64_t)config_value.int64_value);
        }break;
        case double_key_word:
        {
            std::fprintf(stderr, "[list] type: %s key: %s\t\t value: %lf\n","double",config_value.key_name.c_str(),(double)config_value.double_value);
        }break;
        case command_key_word:
        {
            std::fprintf(stderr, "[list] type: %s key: %s\t\t value: unknown\n","command",config_value.key_name.c_str());
        }break;
    }
    return ;
}

int lua_script::read_table_item(const char* lpsztablename
                        ,std::vector<config_type>& item_list)
{
    if(!lua_check_stack(_lua_State && _b_valid_)) return -1;
    lua_empty_stack(_lua_State);
    item_list.clear();
    lua_getglobal(_lua_State,lpsztablename); // 将table放入栈顶
    int n_return_value(0);

    if(!lua_istable(_lua_State,-1))
    {
        return -1;
    }
    else
    {
        // if is table
        int it=lua_gettop(_lua_State);
        lua_pushnil(_lua_State);
        while(lua_next(_lua_State,it))
        {
            if(!lua_istable(_lua_State,-1))
            {
                continue;
            }
            else
            {
                lua_pushstring(_lua_State,"type_name");
                lua_gettable(_lua_State,-2);
                if(!lua_isnumber(_lua_State,-1))
                {
                    continue;
                }
                else
                {
                    config_type config_value;
                    switch((int)(lua_tonumber(_lua_State,-1)))
                    {
                        case 1:
                            {
                                config_value.type_name=string_key_word;
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-1));
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-2));
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-3));
                                lua_pop(_lua_State,1);
//                                std::fprintf(stderr, "%s\n","");
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-1));
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-2));
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-3));
                                lua_pushstring(_lua_State,"key_name");
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-1));
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-2));
//                                std::fprintf(stderr, "lua_istable:%d\n", lua_istable(_lua_State,-3));
                                lua_gettable(_lua_State,-2);
                                if(!lua_isstring(_lua_State,-1))
                                {
                                    const char* lpcszStr=lua_tolstring(_lua_State,-1,NULL);
                                    if(lpcszStr)
                                    {
                                        config_value.key_name=std::string(lpcszStr);
                                    }
                                }
                                else
                                {
                                    config_value.key_name=std::string(lua_tostring(_lua_State,-1));
                                }
                                lua_pop(_lua_State,1);
                                lua_pushstring(_lua_State,"key_value");
                                lua_gettable(_lua_State,-2);
                                if(!lua_isstring(_lua_State,-1))
                                {
                                    const char* lpcszStr=lua_tolstring(_lua_State,-1,NULL);
                                    if(lpcszStr)
                                    {
                                        config_value.string_value=std::string(lpcszStr);
                                    }
                                }
                                else
                                {
                                    config_value.string_value=std::string(lua_tostring(_lua_State,-1));
                                }
                                lua_pop(_lua_State,1);
                            }break;
                        case 2:
                            {
                                config_value.type_name=integer_key_word;
                                lua_pop(_lua_State,1);
                                lua_pushstring(_lua_State,"key_name");
                                lua_gettable(_lua_State,-2);
                                if(!lua_isstring(_lua_State,-1))
                                {
                                    const char* lpcszStr=lua_tolstring(_lua_State,-1,NULL);
                                    if(lpcszStr)
                                    {
                                        config_value.key_name=std::string(lpcszStr);
                                    }
                                }
                                else
                                {
                                    config_value.key_name=std::string(lua_tostring(_lua_State,-1));
                                }
                                lua_pop(_lua_State,1);
                                lua_pushstring(_lua_State,"key_value");
                                lua_gettable(_lua_State,-2);
                                if(!lua_isnumber(_lua_State,-1))
                                {
                                    int isnum(0);int _retval((int)(lua_tonumberx(_lua_State,-1,&isnum)));
                                    if(isnum)
                                    {
                                        config_value.int32_value=_retval;
                                    }
                                }
                                else
                                {
                                    config_value.int32_value=(int)lua_tonumber(_lua_State,-1);
                                }
                                lua_pop(_lua_State,1);
                            }break;
                        case 3:
                            {
                                config_value.type_name=long_integer_key_word;
                                lua_pop(_lua_State,1);
                                lua_pushstring(_lua_State,"key_name");
                                lua_gettable(_lua_State,-2);
                                if(!lua_isstring(_lua_State,-1))
                                {
                                    const char* lpcszStr=lua_tolstring(_lua_State,-1,NULL);
                                    if(lpcszStr)
                                    {
                                        config_value.key_name=std::string(lpcszStr);
                                    }
                                }
                                else
                                {
                                    config_value.key_name=std::string(lua_tostring(_lua_State,-1));
                                }
                                lua_pop(_lua_State,1);
                                lua_pushstring(_lua_State,"key_value");
                                lua_gettable(_lua_State,-2);
                                if(!lua_isnumber(_lua_State,-1))
                                {
                                    int isnum(0);int64_t _retval((int64_t)(lua_tonumberx(_lua_State,-1,&isnum)));
                                    if(isnum)
                                    {
                                        config_value.int64_value=_retval;
                                    }
                                }
                                else
                                {
                                    config_value.int64_value=(int64_t)lua_tonumber(_lua_State,-1);
                                }
                                lua_pop(_lua_State,1);
                            }break;
                        case 4:
                            {
                                config_value.type_name=double_key_word;
                                lua_pop(_lua_State,1);
                                lua_pushstring(_lua_State,"key_name");
                                lua_gettable(_lua_State,-2);
                                if(!lua_isstring(_lua_State,-1))
                                {
                                    const char* lpcszStr=lua_tolstring(_lua_State,-1,NULL);
                                    if(lpcszStr)
                                    {
                                        config_value.key_name=std::string(lpcszStr);
                                    }
                                }
                                else
                                {
                                    config_value.key_name=std::string(lua_tostring(_lua_State,-1));
                                }
                                lua_pop(_lua_State,1);
                                lua_pushstring(_lua_State,"key_value");
                                lua_gettable(_lua_State,-2);
                                if(!lua_isnumber(_lua_State,-1))
                                {
                                    int isnum(0);double _retval((double)(lua_tonumberx(_lua_State,-1,&isnum)));
                                    if(isnum)
                                    {
                                        config_value.double_value=_retval;
                                    }
                                }
                                else
                                {
                                    config_value.double_value=(double)lua_tonumber(_lua_State,-1);
                                }
                                lua_pop(_lua_State,1);
                            }break;
                        case 5:
                            {
                                config_value.type_name=command_key_word;
                                lua_pop(_lua_State,1);
                                lua_pushstring(_lua_State,"key_name");
                                lua_gettable(_lua_State,-2);
                                if(!lua_isstring(_lua_State,-1))
                                {
                                    const char* lpcszStr=lua_tolstring(_lua_State,-1,NULL);
                                    if(lpcszStr)
                                    {
                                        config_value.key_name=std::string(lpcszStr);
                                    }
                                }
                                else
                                {
                                    config_value.key_name=std::string(lua_tostring(_lua_State,-1));
                                }
                                lua_pop(_lua_State,1);

                            }break;
                        default:
                            {
                                config_value.type_name=unknown_key_word;
                            }break;
                    }
                    item_list.push_back(config_value);
                }
                lua_pop(_lua_State,1);
            }
        }
    }
    lua_pop(_lua_State,1);
    std::for_each(item_list.begin(),item_list.end(),print_list);

    return -1;
}




int main(int agrc,char* argv[])
{
    lua_script script("./test.lua");
    std::vector<config_type> config;
    script.read_table_item("test_table",config);
    getchar();
    return 0;
}










