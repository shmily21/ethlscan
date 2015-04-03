#!/usr/bin/env lua
---------------------------------------------------------------------------------------------------
-- FileName     : config_function.lua
--
--
--
--
--
--
---------------------------------------------------------------------------------------------------

_G_THIS_FILE=[[config_function.lua]]
_G_THIS_FILE_GUID="2A719C2E-3230-4A68-95F1-A891292AD058"

_G_SUPPORT_LIST={}

string_key_word=1 --"string"
function string_config(key,value)
    return
    {
        type_name   =string_key_word,
        key_name    =tostring(key),
        key_value   =tostring(value)
    }
end

integer_key_word=2 --"int32_t"

function int32_config(key,value)
    return
    {
        type_name   =integer_key_word,
        key_name    =tostring(key),
        key_value   =tonumber(value)
    }
end

long_integer_key_word=3 --"int64_t"
function int64_config(key,value)
    return
    {
        type_name   =long_integer_key_word,
        key_name    =tostring(key),
        key_value   =tonumber(value)
    }
end

double_key_word=4 --"double"
function double_config(key,value)
    return
    {
        type_name   =double_key_word,
        key_name    =tostring(key),
        key_value   =tonumber(value)
    }
end

command_key_word=5 --"command"
function command_config(key,value)
    return
    {
        type_name   =command_key_word,
        key_name    =tostring(key),
        key_value   =nil
    }
end


load_config=
{
    max_index=0
}

unload_config={}

restore_config={}

open={}

close={}

set_buffer={}

unset_buffer={}

set_current={}

get_current={}

set_whitebalance={}

get_whitebalance={}

set_callback={}

unset_callback={}

set_trigger_free={}

set_trigger_hardware={}

set_width={}

get_width={}

set_height={}

get_height={}

set_exposure_abs={}

get_exposure_abs={}

grab={}

stop={}

