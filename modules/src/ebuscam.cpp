#include "ebuscam.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <list>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <dlfcn.h>

#include <PvResult.h>
#include <PvSystem.h>
#include <PvDeviceFinderWnd.h>
#include <PvDevice.h>
#include <PvBuffer.h>
#include <PvStream.h>
#include <PvStreamRaw.h>
#include <PvConfigurationReader.h>
#include <PvConfigurationWriter.h>
#include <PvFilterRGB.h>
#include <PvBufferWriter.h>
#include <PvPixelType.h>

#include <opencv2/opencv.hpp>


static int nCurrent(0);

#define BUFFER_COUNT 8

int set_device_enum_value(const std::string& value,const char* key)
{
    int _nCurrent=get_current();
    if(ebus_camera[_nCurrent].lDevice.IsConnected())
    {
        PvGenParameterArray* lDeviceParams=ebus_camera[_nCurrent].lDevice.GetGenParameters();
        PvGenParameter *lGenParameter=lDeviceParams->Get(key);
        if(!lGenParameter->IsVisible( PvGenVisibilityGuru ) || !lGenParameter->IsAvailable())
        {
            std::fprintf( stderr,"[ebuscamera.so] %s Parameter Is Not Visible Or Available!\n", key );
            return -3;
        }

        PvGenEnum *lPvGenEnum=dynamic_cast<PvGenEnum *>( lGenParameter );

        PvString lValue(value.c_str());
        PvResult ret = lPvGenEnum->SetValue(lValue);
        if ( !ret.IsOK() )
        {
            std::fprintf( stderr, "[ebuscamera.so] Error set %s enum value to device! Error info:%s\n", key, ret.GetDescription().GetAscii() );
            return -3;
        }
        return 0;
    }
    else
    {
        std::fprintf(stderr, "[ebuscamera.so] %s\n","Not Connected.");
        return -3;
    }
}

int set_device_int_value(const long long int& value,const char* key)
{
    int _nCurrent=get_current();
    if(ebus_camera[_nCurrent].lDevice.IsConnected())
    {
        PvGenParameterArray* lDeviceParams=ebus_camera[_nCurrent].lDevice.GetGenParameters();
        PvGenParameter *lGenParameter=lDeviceParams->Get(key);
        if(!lGenParameter->IsVisible( PvGenVisibilityGuru ) || !lGenParameter->IsAvailable())
        {
            std::fprintf( stderr,"[ebuscamera.so] %s Parameter Is Not Visible Or Available!\n", key );
            return -3;
        }

        PvGenInteger* lPvGenInt = dynamic_cast<PvGenInteger*>( lGenParameter );

        PvInt64 lValue(value);
        PvResult ret = lPvGenInt->SetValue(lValue);
        if ( !ret.IsOK() )
        {
            std::fprintf( stderr, "[ebuscamera.so] Error set %s int value to device! Error info:%s\n", key, ret.GetDescription().GetAscii() );
            return -3;
        }
        return 0;
    }
    else
    {
        std::fprintf(stderr, "[ebuscamera.so] %s\n","Not Connected.");
        return -3;
    }
}

int get_device_int_value(long int& value,const char* lpszFeature)
{
    if(!&value) return -2;
    int _nCurrent=get_current();
    if(!ebus_camera[_nCurrent].lDevice.IsConnected()) return -3;
    PvGenParameterArray* lDeviceParams = ebus_camera[_nCurrent].lDevice.GetGenParameters();
    PvGenParameter* lGenParameter=lDeviceParams->Get(lpszFeature);
    if(!lGenParameter->IsVisible(PvGenVisibilityGuru) || !lGenParameter->IsAvailable())
    {
        std::fprintf(stderr, "[ebuscamera.so] %s Parameter is Not Visible or Available \n",lpszFeature);
        return -3;
    }
    PvGenInteger *lPvGenInt=static_cast<PvGenInteger*>(lGenParameter);
    PvInt64 lValue;
    PvResult ret=lPvGenInt->GetValue(lValue);
    if(!ret.IsOK())
    {
        std::fprintf(stderr, "[ebuscamera.so] Error get %s int value from Device! Error info: %s \n",lpszFeature,ret.GetDescription().GetAscii());
        return -3;
    }
    value=lValue;
    return 0;
}

int get_stream_int_value(long int& value,const char* lpszFeature)
{
    if(!&value) return -2;
    int _nCurrent=get_current();
    if(!ebus_camera[_nCurrent].lStream.IsOpen())
    {
        std::fprintf(stderr, "[ebuscamera.so] %s\n","Failed,Stream Is Not Open.");
        return -3;
    }

    PvGenParameterArray* lGenParameters=ebus_camera[_nCurrent].lStream.GetParameters();
    PvGenParameter *lGenParameter=lGenParameters->Get(lpszFeature);

    if(!lGenParameter->IsVisible(PvGenVisibilityGuru) || !lGenParameter->IsAvailable())
    {
        std::fprintf(stderr, "[ebuscamera.so] %s Parameter is Not Visible or Available \n",lpszFeature);
        return -3;
    }

    PvGenInteger *lPvGenInt=static_cast<PvGenInteger*>(lGenParameter);
    PvInt64 lValue;
    PvResult ret=lPvGenInt->GetValue(lValue);
    if(!ret.IsOK())
    {
        std::fprintf(stderr, "[ebuscamera.so] Error get %s int value from stream! Error info: %s \n",lpszFeature,ret.GetDescription().GetAscii());
    }   return -3;

    value=lValue;
    return 0;
}

int set_stream_int_value(const long long int value,const char* key)
{
    int _nCurrent=get_current();
    if(!ebus_camera[_nCurrent].lStream.IsOpen())
    {
        std::fprintf(stderr, "[ebuscamera.so] %s\n","Stream Is Not Open!");
        return -3;
    }
    PvGenParameterArray* lGenParameters=ebus_camera[_nCurrent].lStream.GetParameters();
    PvGenParameter *lGenParameter=lGenParameters->Get(key);

    if ( !lGenParameter->IsVisible( PvGenVisibilityGuru ) || !lGenParameter->IsAvailable() )
    {
        std::fprintf(stderr, "[ebuscamera.so] %s Parameter is not Visible or Available\n",key);
        return -3;
    }

    PvGenInteger* lPvGenInt=static_cast<PvGenInteger*>(lGenParameter);
    PvInt64 lValue=value;
    PvResult ret=lPvGenInt->SetValue(lValue);

    if ( !ret.IsOK() )
    {
        std::fprintf( stderr,"[ebuscamera.so] Error set %s int value to stream!Error info:%s\n", key, ret.GetDescription().GetAscii() );
        return -3;
    }
    return 0;
}


int auto_config(const char* table_name)
{

    return -1;
}






int load_config(const char* lpszConfig)
{
    return -1;
}

int unload_config(void)
{
    return -1;
}

int restore_config(void)
{
    return -1;
}

int open()
{
    return -1;
}


int close()
{
    return -1;
}

int set_buffer()
{
    return -1;
}


int unset_buffer()
{
    return -1;
}

int set_current(int)
{
    return -1;
}

int get_current(void)
{
    return -1;
}

int set_whitebalance(float r,float g,float b)
{
    return -1;
}

int get_whitebalance(float& r,float& g,float& b)
{
    return -1;
}

int set_callback(eBusCameraCallBackFunction FUNC)
{
    return -1;
}

int unset_callback(void)
{
    return -1;
}

int set_trigger_free(void)
{
    return -1;
}

int set_trigget_hardware(void)
{
    return -1;
}

int set_width(long int nWidth)
{
    return -1;
}

int get_width(long int& nHeight)
{
    return -1;
}

int set_height(long int nHeight)
{
    return -1;
}

int get_height(long int& nHeight)
{
    return -1;
}
int set_exposure_abs(long int rexp,long int gexp,long int bexp)
{
    return -1;
}


int get_exposure_abs(long int& rexp,long int& gexp,long int& bexp)
{
    return -1;
}

int grab()
{
    return -1;
}

int stop()
{
    return -1;
}