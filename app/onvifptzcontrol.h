/*
 * Author: Khiem Tran .Mail:khiembka1992@gmail.com
 * OnvifPTZControl ver 1.2
 * Date: 01/04/2020
 * Make onvif easier to use
*/
#ifndef ONVIFPTZCONTROL_H
#define ONVIFPTZCONTROL_H

#include "soapDeviceBindingProxy.h"
#include "soapMediaBindingProxy.h"
#include "soapImagingBindingProxy.h"
#include "soapPTZBindingProxy.h"
#include "plugin/wsseapi.h" //WS-Sercurity
#include <iostream>
#include <string>

//#define LOG(s) void(0)

#define LOG(s)                                                                                     \
  std::cout << "[" << __FILE__ << "][" << __FUNCTION__ << "][" << __LINE__ << "][Log:" << s << "]" \
            << std::endl // note I leave ; out

#define LOG_ERROR(s)                                                                               \
  std::cerr << "[" << __FILE__ << "][" << __FUNCTION__ << "][" << __LINE__ << "][Error:" << s      \
            << "]" << std::endl // note I leave ; out


class OnvifPTZControl
{
public:
  OnvifPTZControl(std::string strCameraIP, std::string strUserName = "", std::string strPassWord= "");
  ~OnvifPTZControl();

  bool InitControl();
  void DestroyControl();

  void report_error();

  // to set the timestamp and authentication credentials in a request message
  bool set_credentials();

  bool PTZ_AbsMove(float pan, float tilt, float zoom);
  bool PTZ_AbsMove(float pan, float tilt, float zoom, float panSpeed, float tiltSpeed, float zoomSpeed);

  tt__MoveStatus PTZ_GetStatus(float &pan, float &tilt, float &zoom);

  float MinPan = -1;
  float MaxPan = 1;
  float MinTilt = -1;
  float MaxTilt = 1;
  float MinZoom = 0;
  float MaxZoom = 1;

private:
  soap   *m_soap = nullptr;  //Soap for onvif
  DeviceBindingProxy  *proxyDevice  = nullptr;    //Device API
  MediaBindingProxy   *proxyMedia   = nullptr;    //Media API
  ImagingBindingProxy *proxyImaging = nullptr;    //Imaging API
  PTZBindingProxy     *proxyPTZ     = nullptr;    //PTZ API


  std::string strCameraIP;
  std::string strMediaProfileToken;

  std::string strUserName;
  std::string strPassWord;

};

#endif // ONVIFPTZCONTROL_H
