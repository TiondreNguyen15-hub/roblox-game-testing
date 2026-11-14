#include "apex/hud.hpp"
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Storage.Streams.h>
#include <stdexcept>
namespace apex {namespace {
class WindowsHudReader final:public HudTextReader {
 bool apartment_=false;winrt::Windows::Media::Ocr::OcrEngine engine_{nullptr};std::string language_;
public:
 WindowsHudReader(){try{winrt::init_apartment(winrt::apartment_type::multi_threaded);apartment_=true;engine_=winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();if(!engine_)engine_=winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromLanguage(winrt::Windows::Globalization::Language(L"en-US"));if(!engine_)throw std::runtime_error("OCR_LANGUAGE_UNAVAILABLE: Windows text recognition language is missing. Install text recognition for your Windows language in Settings > Time & language > Language options.");language_=winrt::to_string(engine_.RecognizerLanguage().LanguageTag());}catch(const winrt::hresult_error& e){if(apartment_)winrt::uninit_apartment();throw std::runtime_error("Windows OCR initialization: "+winrt::to_string(e.message()));}catch(...){if(apartment_)winrt::uninit_apartment();throw;}}
 ~WindowsHudReader()override{engine_=nullptr;if(apartment_)winrt::uninit_apartment();}
 std::string description()const override{return "Windows OCR / "+language_+" (CPU; separate from CUDA model benchmark)";}
 std::string recognize(const HudImage& image)override{try{
  if(!image.width||!image.height||image.width>1600||image.height>1600||image.rgba.size()!=size_t(image.width)*image.height*4||image.width>winrt::Windows::Media::Ocr::OcrEngine::MaxImageDimension()||image.height>winrt::Windows::Media::Ocr::OcrEngine::MaxImageDimension())throw std::runtime_error("OCR image exceeds supported dimensions");
  auto pixels=image.rgba;for(size_t i=0;i<pixels.size();i+=4){std::swap(pixels[i],pixels[i+2]);pixels[i+3]=255;}
  winrt::Windows::Storage::Streams::DataWriter writer;writer.WriteBytes(winrt::array_view<const uint8_t>(pixels.data(),pixels.data()+pixels.size()));auto buffer=writer.DetachBuffer();winrt::Windows::Graphics::Imaging::SoftwareBitmap bitmap(winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,image.width,image.height,winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Ignore);bitmap.CopyFromBuffer(buffer);
  auto result=engine_.RecognizeAsync(bitmap).get();auto text=winrt::to_string(result.Text());bitmap.Close();return text.substr(0,4096);
 }catch(const winrt::hresult_error& e){throw std::runtime_error("Windows OCR recognition: "+winrt::to_string(e.message()));}}
};
}
std::unique_ptr<HudTextReader> makeHudTextReader(){return std::make_unique<WindowsHudReader>();}
}
