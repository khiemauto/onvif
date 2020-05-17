#include "onvifptzcontrol.h"
#include "wsdd.nsmap"       //Namespaces

OnvifPTZControl::OnvifPTZControl(std::string strCameraIP, std::string strUserName,
                                 std::string strPassWord)
    : strCameraIP(strCameraIP), strUserName(strUserName),
      strPassWord(strPassWord) {

}

OnvifPTZControl::~OnvifPTZControl()
{
  DestroyControl();
}

bool OnvifPTZControl::InitControl() {
  if (this->m_soap != nullptr) {
    this->DestroyControl();
  }

  this->m_soap = soap_new();
  this->m_soap->connect_timeout = m_soap->recv_timeout = m_soap->send_timeout = 10; // 10 sec
  soap_register_plugin(this->m_soap, soap_wsse);

  this->proxyDevice = new DeviceBindingProxy(this->m_soap);

  // get device info and print
  thread_local std::string strDeviceUrl;
  strDeviceUrl = "http://" + this->strCameraIP + "/onvif/device_service";
  this->proxyDevice->soap_endpoint = strDeviceUrl.c_str();
  std::cout << "OnvifDeviceUrl:" << this->proxyDevice->soap_endpoint << std::endl;
  _tds__GetDeviceInformation *GetDeviceInformation = soap_new__tds__GetDeviceInformation(m_soap);
  _tds__GetDeviceInformationResponse GetDeviceInformationResponse;
  if (!set_credentials()) {
    DestroyControl();
    return false;
  }
  if (proxyDevice->GetDeviceInformation(GetDeviceInformation, GetDeviceInformationResponse)) {
    report_error();
    DestroyControl();
    return false;
  }
  std::cout << "Manufacturer:    " << GetDeviceInformationResponse.Manufacturer << std::endl;
  std::cout << "Model:           " << GetDeviceInformationResponse.Model << std::endl;
  std::cout << "FirmwareVersion: " << GetDeviceInformationResponse.FirmwareVersion << std::endl;
  std::cout << "SerialNumber:    " << GetDeviceInformationResponse.SerialNumber << std::endl;
  std::cout << "HardwareId:      " << GetDeviceInformationResponse.HardwareId << std::endl;

  // get device capabilities and print media
  _tds__GetCapabilities *GetCapabilities = soap_new__tds__GetCapabilities(m_soap);
  _tds__GetCapabilitiesResponse GetCapabilitiesResponse;
  if (!set_credentials()) {
    DestroyControl();
    return false;
  }
  if (proxyDevice->GetCapabilities(GetCapabilities, GetCapabilitiesResponse))
    report_error();
  if (!GetCapabilitiesResponse.Capabilities || !GetCapabilitiesResponse.Capabilities->Media ||
      !GetCapabilitiesResponse.Capabilities->Imaging) {
    report_error();
    DestroyControl();
    return false;
  }

  if (GetCapabilitiesResponse.Capabilities->Media != nullptr) {
    thread_local std::string strUrl;
    strUrl = GetCapabilitiesResponse.Capabilities->Media->XAddr;
    int indexFooter = strUrl.find("/onvif");
    // Check if contains onvif then replace cameraip to header
    if (indexFooter > 0) {
      strUrl.erase(0, indexFooter);
      strUrl.insert(0, "http://" + this->strCameraIP);
    }
    std::cout << "Media XAddr:  " << strUrl << std::endl;
    proxyMedia = new MediaBindingProxy(m_soap);
    proxyMedia->soap_endpoint = strUrl.c_str();
  }
  if (GetCapabilitiesResponse.Capabilities->Imaging != nullptr) {
    thread_local std::string strUrl;
    strUrl = GetCapabilitiesResponse.Capabilities->Imaging->XAddr;
    int indexFooter = strUrl.find("/onvif");
    // Check if contains onvif then replace cameraip to header
    if (indexFooter > 0) {
      strUrl.erase(0, indexFooter);
      strUrl.insert(0, "http://" + this->strCameraIP);
    }
    std::cout << "Imaging XAddr:  " << strUrl << std::endl;
    proxyImaging = new ImagingBindingProxy(m_soap);
    proxyImaging->soap_endpoint = strUrl.c_str();
  }
  if (GetCapabilitiesResponse.Capabilities->PTZ != nullptr) {
    thread_local std::string strUrl;
    strUrl = GetCapabilitiesResponse.Capabilities->PTZ->XAddr;
    int indexFooter = strUrl.find("/onvif");
    // Check if contains onvif then replace cameraip to header
    if (indexFooter > 0) {
      strUrl.erase(0, indexFooter);
      strUrl.insert(0, "http://" + this->strCameraIP);
    }
    std::cout << "PTZ XAddr:  " << strUrl << std::endl;

    proxyPTZ = new PTZBindingProxy(m_soap);
    proxyPTZ->soap_endpoint = strUrl.c_str();

    _trt__GetProfiles *GetProfiles = soap_new__trt__GetProfiles(m_soap);
    _trt__GetProfilesResponse GetProfilesResponse;
    if (!set_credentials()) {
      DestroyControl();
      return false;
    }

    if (proxyMedia->GetProfiles(GetProfiles, GetProfilesResponse)) {
      report_error();
      DestroyControl();
      return false;
    }

    std::cout << "MediaProfile token:" << GetProfilesResponse.Profiles[0]->token << std::endl;
    this->strMediaProfileToken = GetProfilesResponse.Profiles[0]->token;

    _tptz__GetConfigurationOptions *GetConfigurationOptions = soap_new__tptz__GetConfigurationOptions(m_soap);
    _tptz__GetConfigurationOptionsResponse GetConfigurationOptionsResponse;
    GetConfigurationOptions->ConfigurationToken = GetProfilesResponse.Profiles[0]->PTZConfiguration->token;
    if (!set_credentials()) {
      DestroyControl();
      return false;
    }
    if (proxyPTZ->GetConfigurationOptions(GetConfigurationOptions, GetConfigurationOptionsResponse)) {
      report_error();
      DestroyControl();
      return false;
    }
    MinPan = GetConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[0]->XRange->Min;
    MaxPan = GetConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[0]->XRange->Max;
    MinTilt = GetConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[0]->YRange->Min;
    MaxTilt = GetConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[0]->YRange->Max;
    MinZoom = GetConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace[0]->XRange->Min;
    MaxZoom = GetConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace[0]->XRange->Max;

    std::cout << "Pan[" << MinPan << "~" << MaxPan << "] Tilt[" << MinTilt << "~" << MaxTilt << "] Zoom[" << MinZoom << "~" << MaxZoom << "]" << std::endl;
  }
  return true;
}

void OnvifPTZControl::DestroyControl() {
  // free all deserialized and managed data, we can still reuse the context and proxies after this
  if (m_soap != nullptr) {
    soap_destroy(m_soap);
    soap_end(m_soap);
    // free the shared context, proxy classes must terminate as well after this
    soap_free(m_soap);
  }
  delete proxyDevice;
  delete proxyMedia;
  delete proxyImaging;
  delete proxyPTZ;

  m_soap = nullptr;
  proxyDevice = nullptr;
  proxyMedia = nullptr;
  proxyImaging = nullptr;
  proxyPTZ = nullptr;
}

void OnvifPTZControl::report_error() {
  std::cerr << "Oops, something went wrong:" << std::endl;
  soap_stream_fault(m_soap, std::cerr);
}

bool OnvifPTZControl::set_credentials() {
  soap_wsse_delete_Security(m_soap);
  // Access with username, password and lifetime
  if (soap_wsse_add_Timestamp(m_soap, "Time", 10) ||
      soap_wsse_add_UsernameTokenDigest(m_soap, "Auth", strUserName.c_str(), strPassWord.c_str())) {
    report_error();
    return false;
  }
  return true;
}

bool OnvifPTZControl::PTZ_AbsMove(float pan, float tilt, float zoom) {
  if (proxyPTZ == nullptr) {
    std::cerr << "Unknow proxyPTZ" << std::endl;
    return false;
  }

  _tptz__AbsoluteMove *AbsoluteMove = soap_new__tptz__AbsoluteMove(m_soap);
  _tptz__AbsoluteMoveResponse AbsoluteMoveResponse;

  AbsoluteMove->ProfileToken = strMediaProfileToken;
  if (AbsoluteMove->Position == nullptr)
    AbsoluteMove->Position = soap_new_tt__PTZVector(m_soap);
  if (AbsoluteMove->Position->PanTilt == nullptr)
    AbsoluteMove->Position->PanTilt = soap_new_tt__Vector2D(m_soap);
  if (AbsoluteMove->Position->Zoom == nullptr)
    AbsoluteMove->Position->Zoom = soap_new_tt__Vector1D(m_soap);
  AbsoluteMove->Position->PanTilt->x = pan;
  AbsoluteMove->Position->PanTilt->y = tilt;
  AbsoluteMove->Position->Zoom->x = zoom;

  if (!set_credentials())
    return false;

  if (proxyPTZ->AbsoluteMove(AbsoluteMove, AbsoluteMoveResponse)) {
    report_error();
    return false;
  }
}

bool OnvifPTZControl::PTZ_AbsMove(float pan, float tilt, float zoom, float panSpeed,
                                  float tiltSpeed, float zoomSpeed) {
  if (proxyPTZ == nullptr) {
    std::cerr << "Unknow proxyPTZ" << std::endl;
    return false;
  }

  std::cout << panSpeed << tiltSpeed << zoomSpeed << std::endl;

  _tptz__AbsoluteMove *AbsoluteMove = soap_new__tptz__AbsoluteMove(m_soap);
  _tptz__AbsoluteMoveResponse AbsoluteMoveResponse;

  AbsoluteMove->ProfileToken = strMediaProfileToken;
  if (AbsoluteMove->Position == nullptr)
    AbsoluteMove->Position = soap_new_tt__PTZVector(m_soap);
  if (AbsoluteMove->Position->PanTilt == nullptr)
    AbsoluteMove->Position->PanTilt = soap_new_tt__Vector2D(m_soap);
  if (AbsoluteMove->Position->Zoom == nullptr)
    AbsoluteMove->Position->Zoom = soap_new_tt__Vector1D(m_soap);
  AbsoluteMove->Position->PanTilt->x = pan;
  AbsoluteMove->Position->PanTilt->y = tilt;
  AbsoluteMove->Position->Zoom->x = zoom;

  if (AbsoluteMove->Speed == nullptr)
    AbsoluteMove->Speed = soap_new_tt__PTZSpeed(m_soap);
  if (AbsoluteMove->Speed->PanTilt == nullptr)
    AbsoluteMove->Speed->PanTilt = soap_new_tt__Vector2D(m_soap);
  if (AbsoluteMove->Speed->Zoom == nullptr)
    AbsoluteMove->Speed->Zoom = soap_new_tt__Vector1D(m_soap);
  AbsoluteMove->Speed->PanTilt->x = panSpeed;
  AbsoluteMove->Speed->PanTilt->y = tiltSpeed;
  AbsoluteMove->Speed->Zoom->x = zoomSpeed;

  if (!set_credentials())
    return false;
  if (proxyPTZ->AbsoluteMove(AbsoluteMove, AbsoluteMoveResponse)) {
    report_error();
    return false;
  }
}

tt__MoveStatus OnvifPTZControl::PTZ_GetStatus(float &pan, float &tilt, float &zoom) {
  if (proxyPTZ == nullptr) {
    std::cerr << "Unknow proxyPTZ" << std::endl;
    return tt__MoveStatus__UNKNOWN;
  }
  _tptz__GetStatus *GetStatus = soap_new__tptz__GetStatus(m_soap);
  _tptz__GetStatusResponse GetStatusResponse;
  GetStatus->ProfileToken = strMediaProfileToken;
  if (!set_credentials())
    return tt__MoveStatus__UNKNOWN;
  if (proxyPTZ->GetStatus(GetStatus, GetStatusResponse)) {
    report_error();
    return tt__MoveStatus__UNKNOWN;
  }
  std::cout << "Pan:" << GetStatusResponse.PTZStatus->Position->PanTilt->x << std::endl;
  std::cout << "Tilt:" << GetStatusResponse.PTZStatus->Position->PanTilt->y << std::endl;
  std::cout << "Zoom:" << GetStatusResponse.PTZStatus->Position->Zoom->x << std::endl;

  pan = GetStatusResponse.PTZStatus->Position->PanTilt->x;
  tilt = GetStatusResponse.PTZStatus->Position->PanTilt->y;
  zoom = GetStatusResponse.PTZStatus->Position->Zoom->x;

  if (GetStatusResponse.PTZStatus->MoveStatus->PanTilt &&
      GetStatusResponse.PTZStatus->MoveStatus->Zoom) {
    if (*GetStatusResponse.PTZStatus->MoveStatus->PanTilt == tt__MoveStatus__UNKNOWN ||
        *GetStatusResponse.PTZStatus->MoveStatus->Zoom == tt__MoveStatus__UNKNOWN) {
      return tt__MoveStatus__UNKNOWN;
    }
    if (*GetStatusResponse.PTZStatus->MoveStatus->PanTilt == tt__MoveStatus__MOVING ||
        *GetStatusResponse.PTZStatus->MoveStatus->Zoom == tt__MoveStatus__MOVING) {
      return tt__MoveStatus__MOVING;
    }
    if (*GetStatusResponse.PTZStatus->MoveStatus->PanTilt == tt__MoveStatus__IDLE &&
        *GetStatusResponse.PTZStatus->MoveStatus->Zoom == tt__MoveStatus__IDLE) {
      return tt__MoveStatus__IDLE;
    }
  }
  return tt__MoveStatus__UNKNOWN;
}
