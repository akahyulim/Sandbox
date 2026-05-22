#include "stdafx.h"
#include "FileUtils.h"

namespace Dive
{
    namespace FileUtils
    {
        std::filesystem::path OpenFile(const char* filter, HWND owner, const std::string& initialDir)
        {
            std::vector<char> file(MAX_PATH, '\0');

            // 윈도우가 최근 경로를 기억하지 못하게 경로를 절대 경로로 명확히 지정
            std::string absoluteInitialDir = "";
            if (!initialDir.empty()) 
            {
                absoluteInitialDir = std::filesystem::absolute(initialDir).generic_string();
            }

            OPENFILENAMEA ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = owner;
            ofn.lpstrFile = file.data();
            ofn.nMaxFile = static_cast<DWORD>(MAX_PATH);

            // 1. 초기 경로 설정 (절대 경로가 아니면 윈도우가 무시할 때가 많습니다)
            if (!absoluteInitialDir.empty()) 
            {
                ofn.lpstrInitialDir = absoluteInitialDir.c_str();
            }

            ofn.lpstrFilter = filter;
            ofn.nFilterIndex = 1;

            // 2. OFN_NOCHANGEDIR와 함께, 윈도우가 최근 경로를 강제로 덮어쓰지 않도록 설정
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn)) {
                return std::filesystem::path(ofn.lpstrFile);
            }

            return {};
        }

        std::filesystem::path SaveFile(const char* filter, HWND owner, const std::string& defaultPath)
        {
            OPENFILENAMEA ofn;       // 공용 대화 상자 구조체 (ANSI 버전)
            char szFile[260] = { 0 }; // 선택된 파일 경로가 저장될 버퍼
            char currentDir[260] = { 0 };

            // 1. 현재 작업 디렉토리 백업 (GetSaveFileName이 작업 디렉토리를 바꿀 수 있음)
            std::filesystem::path originalPath = std::filesystem::current_path();

            // 2. 구조체 초기화
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = owner;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = filter;
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

            // 3. 초기 경로 설정 (프로젝트 자산 폴더 등)
            if (!defaultPath.empty())
            {
                ofn.lpstrInitialDir = defaultPath.c_str();
            }

            // 4. 대화 상자 실행
            if (GetSaveFileNameA(&ofn))
            {
                // 작업 디렉토리 복구
                std::filesystem::current_path(originalPath);
                return std::filesystem::path(szFile);
            }

            // 사용자가 취소한 경우
            std::filesystem::current_path(originalPath);
            return std::filesystem::path();
        }

        std::string SelectFolder(HWND owner)
        {
            std::string folderPath;

            using Microsoft::WRL::ComPtr;
            ComPtr<IFileOpenDialog> fileDialog; // IFileDialog보다 구체적인 IFileOpenDialog 권장

            // 1. 인스턴스 생성
            HRESULT result = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog));

            if (SUCCEEDED(result))
            {
                DWORD options;
                if (SUCCEEDED(fileDialog->GetOptions(&options)))
                    fileDialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM); // 파일 시스템 경로만 허용

                // 2. 부모 윈도우 핸들 전달 (에디터가 뒤에서 클릭되지 않게 방어)
                if (SUCCEEDED(fileDialog->Show(owner)))
                {
                    ComPtr<IShellItem> shellItem;
                    if (SUCCEEDED(fileDialog->GetResult(&shellItem)))
                    {
                        PWSTR path = nullptr;
                        if (SUCCEEDED(shellItem->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                        {
                            // 3. UTF-8 변환 (인코딩 안전성 확보)
                            int size_needed = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
                            if (size_needed > 0)
                            {
                                folderPath.resize(size_needed - 1);
                                WideCharToMultiByte(CP_UTF8, 0, path, -1, &folderPath[0], size_needed, NULL, NULL);
                            }
                            CoTaskMemFree(path);
                        }
                    }
                }
            }

            return folderPath; // COM 초기화/해제는 호출부 밖(Main)에서 관리하는 것을 추천!
        }
    }
}
