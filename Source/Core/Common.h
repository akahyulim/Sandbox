#pragma once
#include <random>
#include <cstdint>
#include <string>
#include <Windows.h>

#define DV_DELETE(p) { if(p) delete (p); (p) = nullptr; }
#define DV_DELETE_ARRAY(p) { if(p) delete[](p); (p) = nullptr; }
#define DV_RELEASE(p) { if(p) (p)->Release(); (p) = nullptr; }

namespace Dive
{
    inline uint64_t GenerateUniqueID(uint64_t min = 0, uint64_t max = UINT64_MAX)
    {
        static std::mt19937_64 engine{ std::random_device{}() };
        static std::uniform_int_distribution<uint64_t> dist(min, max);
        return dist(engine);
    }

    namespace ErrorUtils
    {
        // HRESULT를 16진수 문자열로 반환
        inline std::string ToHex(HRESULT hr)
        {
            char buffer[16];
            sprintf_s(buffer, sizeof(buffer), "0x%08X", static_cast<uint32_t>(hr));
            return std::string(buffer);
        }

        // 대표적인 HRESULT를 이름으로 매핑
        inline std::string ToName(HRESULT hr)
        {
            switch (hr)
            {
            case S_OK: return "S_OK";
            case E_ACCESSDENIED: return "E_ACCESSDENIED";
            case E_INVALIDARG: return "E_INVALIDARG";
            case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
            case E_FAIL: return "E_FAIL";
            case DXGI_ERROR_DEVICE_REMOVED: return "DXGI_ERROR_DEVICE_REMOVED";
            case DXGI_ERROR_INVALID_CALL: return "DXGI_ERROR_INVALID_CALL";
            case DXGI_ERROR_DEVICE_HUNG: return "DXGI_ERROR_DEVICE_HUNG";
            case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
            default: return "Unknown HRESULT";
            }
        }

        // 시스템 메시지 포함 전체 문자열 반환 (예: 에러 로그용)
        inline std::string ToVerbose(HRESULT hr)
        {
            std::string result = ToHex(hr) + " (" + ToName(hr) + ")";

            // 시스템 메시지 붙이기
            char* sysMsg = nullptr;
            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                hr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&sysMsg),
                0,
                nullptr
            );

            if (sysMsg)
            {
                result += " - ";
                result += sysMsg;
                LocalFree(sysMsg);
            }

            return result;
        }
    }
}