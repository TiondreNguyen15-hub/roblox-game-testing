#include "apex/hud.hpp"
#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
struct BitmapFixture {
 HDC dc=CreateCompatibleDC(nullptr);HBITMAP bitmap=nullptr;HFONT font=nullptr;HGDIOBJ oldBitmap=nullptr,oldFont=nullptr;void* data=nullptr;
 ~BitmapFixture(){if(oldFont)SelectObject(dc,oldFont);if(oldBitmap)SelectObject(dc,oldBitmap);if(font)DeleteObject(font);if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);}
 apex::CapturedFrame draw(const wchar_t* text){BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=900;info.bmiHeader.biHeight=-160;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&data,nullptr,0);if(!bitmap)throw std::runtime_error("Actual OCR fixture bitmap failed");oldBitmap=SelectObject(dc,bitmap);RECT bounds{0,0,900,160};FillRect(dc,&bounds,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));font=CreateFontW(-72,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,L"Segoe UI");oldFont=SelectObject(dc,font);SetTextColor(dc,RGB(255,255,255));SetBkColor(dc,RGB(0,0,0));if(!TextOutW(dc,30,30,text,lstrlenW(text)))throw std::runtime_error("Actual OCR fixture text drawing failed");GdiFlush();apex::CapturedFrame f;f.width=900;f.height=160;f.sequence=1;f.sourceTimestamp100ns=10000000;auto* bytes=static_cast<unsigned char*>(data);f.rgba.assign(bytes,bytes+900*160*4);for(size_t i=0;i<f.rgba.size();i+=4){std::swap(f.rgba[i],f.rgba[i+2]);f.rgba[i+3]=255;}return f;}
};
int main(){try{
 BitmapFixture fixture;auto frame=fixture.draw(L"28 / 160");apex::HudOptions options;options.scale=1;auto image=apex::cropHudFrame(frame,options);auto reader=apex::makeHudTextReader();auto text=reader->recognize(image);auto parsed=apex::interpretHudText(text,apex::HudMode::Ammo);std::cout<<"Backend: "<<reader->description()<<"; actual recognized text: "<<text<<'\n';if(parsed.magazine!=28||parsed.reserve!=160)throw std::runtime_error("Actual Windows OCR did not read known numeric fixture; no result fabricated");reader.reset();
 apex::HudOcrService service;service.submit(std::make_shared<const apex::CapturedFrame>(frame),options,"original generated text fixture",true);auto begin=std::chrono::steady_clock::now();while(service.snapshot().completed==0&&std::chrono::steady_clock::now()-begin<std::chrono::seconds(20)){auto state=service.snapshot();if(state.status=="unavailable")throw std::runtime_error(state.message);std::this_thread::sleep_for(std::chrono::milliseconds(20));}auto result=service.snapshot();service.stop();if(result.latest.magazine!=28||result.latest.reserve!=160||result.completed!=1)throw std::runtime_error("Actual OCR worker integration failed");
 auto output=std::filesystem::temp_directory_path()/"apex-actual-ocr-test.csv";service.exportCsv(output);if(std::filesystem::file_size(output)<100)throw std::runtime_error("Actual OCR CSV export failed");std::filesystem::remove(output);
 std::cout<<"PASS actual pixel recognition, worker delivery, numeric candidate and CSV export; not a gameplay accuracy test\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
