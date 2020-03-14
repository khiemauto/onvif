#include "soapDeviceBindingProxy.h"
#include "soapMediaBindingProxy.h"
#include "soapImagingBindingProxy.h"
#include "soapPTZBindingProxy.h"

#include "plugin/wsseapi.h" //WS-Sercurity
#include "wsdd.nsmap"       //Namespaces

#define USERNAME "admin"
#define PASSWORD "xilinx123"
#define HOSTNAME "http://192.168.200.217/onvif/device_service"
// to report an error
void report_error(struct soap *soap)
{
  std::cerr << "Oops, something went wrong:" << std::endl;
  soap_stream_fault(soap, std::cerr);
  exit(EXIT_FAILURE);
}

// to set the timestamp and authentication credentials in a request message
void set_credentials(struct soap *soap)
{
  soap_wsse_delete_Security(soap);
  //Access with username, password and lifetime
  if (soap_wsse_add_Timestamp(soap, "Time", 10)
   || soap_wsse_add_UsernameTokenDigest(soap, "Auth", USERNAME, PASSWORD))
    report_error(soap);
}

int main()
{
    std::cout << "START" << std::endl;
  // create a context with strict XML validation and exclusive XML canonicalization for WS-Security enabled
  struct soap *soap = soap_new();
  soap->connect_timeout = soap->recv_timeout = soap->send_timeout = 10; // 10 sec
  soap_register_plugin(soap, soap_wsse);

  // enable https connections with server certificate verification using cacerts.pem
//  if (soap_ssl_client_context(soap, SOAP_SSL_SKIP_HOST_CHECK, NULL, NULL, "cacerts.pem", NULL, NULL))
//    report_error(soap);

  // create the proxies to access the ONVIF service API at HOSTNAME
  DeviceBindingProxy proxyDevice(soap);
  MediaBindingProxy proxyMedia(soap);
  ImagingBindingProxy proxyImaging(soap);
  PTZBindingProxy proxyPTZ(soap);

//  soap_register_plugin(proxyDevice.soap, soap_wsse);  //Not need register soap again
//  soap_register_plugin(proxyMedia.soap, soap_wsse);
//  soap_register_plugin(proxyImaging.soap, soap_wsse);
  // get device info and print
  proxyDevice.soap_endpoint = HOSTNAME;
  _tds__GetDeviceInformation GetDeviceInformation;
  _tds__GetDeviceInformationResponse GetDeviceInformationResponse;
  GetDeviceInformation.soap = soap;
  GetDeviceInformationResponse.soap = soap;
  set_credentials(soap);
  if (proxyDevice.GetDeviceInformation(&GetDeviceInformation, GetDeviceInformationResponse))
    report_error(soap);
  std::cout << "Manufacturer:    " << GetDeviceInformationResponse.Manufacturer << std::endl;
  std::cout << "Model:           " << GetDeviceInformationResponse.Model << std::endl;
  std::cout << "FirmwareVersion: " << GetDeviceInformationResponse.FirmwareVersion << std::endl;
  std::cout << "SerialNumber:    " << GetDeviceInformationResponse.SerialNumber << std::endl;
  std::cout << "HardwareId:      " << GetDeviceInformationResponse.HardwareId << std::endl;

  // get device capabilities and print media
  _tds__GetCapabilities GetCapabilities;
  _tds__GetCapabilitiesResponse GetCapabilitiesResponse;
  GetCapabilities.soap = soap;
  GetCapabilitiesResponse.soap = soap;
  set_credentials(soap);
  if (proxyDevice.GetCapabilities(&GetCapabilities, GetCapabilitiesResponse))
    report_error(soap);
  if (!GetCapabilitiesResponse.Capabilities || !GetCapabilitiesResponse.Capabilities->Media || !GetCapabilitiesResponse.Capabilities->Imaging)
  {
    std::cerr << "Missing device capabilities info" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "Media XAddr:  " << GetCapabilitiesResponse.Capabilities->Media->XAddr << std::endl;
  std::cout << "Imaging XAddr:" << GetCapabilitiesResponse.Capabilities->Imaging->XAddr << std::endl;
  std::cout << "PTZ XAddr:" << GetCapabilitiesResponse.Capabilities->PTZ->XAddr << std::endl;

  // set the Media proxy endpoint to XAddr
  proxyMedia.soap_endpoint = GetCapabilitiesResponse.Capabilities->Media->XAddr.c_str();
  proxyImaging.soap_endpoint = GetCapabilitiesResponse.Capabilities->Imaging->XAddr.c_str();
  proxyPTZ.soap_endpoint = GetCapabilitiesResponse.Capabilities->PTZ->XAddr.c_str();

    _trt__GetVideoSources GetVideoSources;
    _trt__GetVideoSourcesResponse GetVideoSourcesResponse;
    set_credentials(soap);
    if (proxyMedia.GetVideoSources(&GetVideoSources, GetVideoSourcesResponse))
        report_error(soap);
    std::string strVideoToken = GetVideoSourcesResponse.VideoSources[0]->token;

    std::cout << "Video Token:" << strVideoToken << std::endl;

    _timg__SetImagingSettings *SetImagingSettings = soap_new__timg__SetImagingSettings(soap, -1);
    _timg__SetImagingSettingsResponse SetImagingSettingsResponse;

    SetImagingSettings->VideoSourceToken = strVideoToken;   //Set Video Token
    SetImagingSettings->ImagingSettings = soap_new_tt__ImagingSettings20(soap, -1);

    float Brightness = 10.0;
    float ColorSaturation = 35.0;
    float Contrast = 25.0;
    bool ForcePersistence = true; //You can add some more para
    float ExposureTime = 10000.0;

    SetImagingSettings->ImagingSettings->Brightness = &Brightness;
    SetImagingSettings->ImagingSettings->ColorSaturation = &ColorSaturation;
    SetImagingSettings->ImagingSettings->Contrast = &Contrast;
    SetImagingSettings->ImagingSettings->Exposure = soap_new_tt__Exposure20(soap, -1);
    SetImagingSettings->ImagingSettings->Exposure->MaxExposureTime = &ExposureTime;
    SetImagingSettings->ForcePersistence = &ForcePersistence;

    _tptz__AbsoluteMove *AbsoluteMove = soap_new__tptz__AbsoluteMove(soap, -1);


    while (true) {
        std::cout << "Set BrightNess:"; /*std::cin >>*/ Brightness++;
        std::cout << "Set ColorSaturation:"; /*std::cin >>*/ ColorSaturation++;
        std::cout << "Set Contrast:"; /*std::cin >>*/ Contrast++;
        std::cout << "Set ExposureTime:"; /*std::cin >>*/ ExposureTime++;
        set_credentials(soap);

        if (proxyImaging.SetImagingSettings(SetImagingSettings, SetImagingSettingsResponse)) {
            report_error(soap);
        }
        //Check again
        _timg__GetImagingSettings *GetImagingSettings = soap_new__timg__GetImagingSettings(soap, -1);
        _timg__GetImagingSettingsResponse GetImagingSettingsResponse;
        GetImagingSettings->VideoSourceToken = strVideoToken;
        set_credentials(soap);
        if (proxyImaging.GetImagingSettings(GetImagingSettings, GetImagingSettingsResponse))
            report_error(soap);
        if (GetImagingSettingsResponse.ImagingSettings->Brightness != nullptr) {
            std::cout << "Get Brightness:    " << *(GetImagingSettingsResponse.ImagingSettings->Brightness) << std::endl;
        }
        if (GetImagingSettingsResponse.ImagingSettings->Contrast != nullptr) {
            std::cout << "Get Contrast:    " << *(GetImagingSettingsResponse.ImagingSettings->Contrast) << std::endl;
        }
        if (GetImagingSettingsResponse.ImagingSettings->ColorSaturation != nullptr) {
            std::cout << "Get ColorSaturation:    " << *(GetImagingSettingsResponse.ImagingSettings->ColorSaturation) << std::endl;
        }
        if (GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime != nullptr) {
            std::cout << "Get ExposureTime:    " << *(GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime) << std::endl;
        }
    }

  // free all deserialized and managed data, we can still reuse the context and proxies after this
  soap_destroy(soap);
  soap_end(soap);

  // free the shared context, proxy classes must terminate as well after this
  soap_free(soap);

  return 0;
}

