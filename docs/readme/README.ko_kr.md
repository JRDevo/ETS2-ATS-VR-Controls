<!-- Short README in 한국어. The full guide (English): ../../README.md -->
# ETS2 ATS VR Controls: VR 컨트롤러로 VR에서 유로 트럭 시뮬레이터 2(Euro Truck Simulator 2)와 아메리칸 트럭 시뮬레이터(American Truck Simulator)를 운전하세요

> [!CAUTION]
> **이 모드는 https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases 에서만 다운로드하세요.** 웹사이트도, Discord도,
> Nexus도, Steam 창작마당도 없습니다. 이 모드를 제공한다는 다른 사이트, 미러, "설치 프로그램"은 제가 만든 것이 아닙니다. 실행하지 마세요.

![ETS2 / ATS VR Controls](../images/banner.png)

**헤드셋과 컨트롤러만으로** VR에서 **Euro Truck Simulator 2**와 **American Truck Simulator**를 즐기세요. 핸들도,
게임패드도, 키보드도, 마우스도 필요 없습니다.

- **컨트롤러로 운전:** 스틱으로, 한 손이나 두 손으로 잡는 핸들로, 또는 가리켜서 조향합니다. 트리거가
  가속과 브레이크입니다.
- **버튼에 원하는 키 지정:** 방향지시등, 경적, 라이트 등을 4개의 레이어로 쓸 수 있습니다(그립을 누르고 있으면 버튼이 더 늘어납니다).
- **손에 쥔 마우스:** 게임 메뉴와 지도용입니다.
- **헤드셋 안의 전체 키보드**, 사용 중인 국가 키보드의 키를 그대로 씁니다.
- **내 언어로 된 VR 메뉴:** 게임 언어를 자동으로 따릅니다.

## 필요한 것

- Windows 10 / 11(64비트), ETS2 또는 ATS, VR 헤드셋(Quest, Index, Vive, Pico, Windows Mixed Reality 등).
- **OpenXR** VR 모드의 게임. Steam: 게임을 마우스 오른쪽 버튼으로 클릭 > 속성 > 실행 옵션: `-openxr`
  (일반 게임 버전에서는 `-experimental_vr`도). `-openvr` 및 `-oculus` 모드는 이 모드와 함께 작동하지 않습니다.

## 설치

1. [Releases](https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases) 페이지에서 최신 `ETS2-ATS-VR-Controls-Setup-....exe`를 다운로드합니다.
   게임을 종료하세요.
2. 그것을 실행하세요. Windows 관리자 확인 창에서 **예**를 클릭한 다음 끝까지 **다음**을 클릭합니다. 모드는
   `C:\Program Files\ETS2 ATS VR Controls`에 설치됩니다(다른 폴더를 선택할 수 있습니다). 설정(`config.ini`)과 로그는
   `C:\ProgramData\ETS2 ATS VR Controls`에 저장됩니다.
3. VR에서 게임을 시작합니다.

게임을 시작할 때마다 "Request to use advanced SDK features detected"(고급 SDK 기능 사용 요청 감지됨)가 표시됩니다.
이것이 (운전을 위한) 이 모드입니다. OK를 가리키고 클릭하세요.

**VR 메뉴:** **양쪽 그립 + 왼쪽 스틱 누름**을 1초 동안 누르세요. 여기서 모든 버튼, 마우스,
조향, 색상, 언어를 설정합니다.

제거: Windows **설정 > 앱 > 설치된 앱 > ETS2 ATS VR Controls > 제거**. 모드와 해당 레지스트리 항목, 로그를 제거합니다. 설정도
삭제할지 묻습니다(아니요 = 나중에 다시 설치할 때를 위해 보관).

설치 프로그램 없이: 같은 페이지의 `.zip`에 `install.ps1`과 `uninstall.ps1`이 들어 있습니다(전체 가이드 참조).

## 더 보기

- **전체 가이드**(영어): [README](../../README.md).
- 문제가 있나요? [버그 신고](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new/choose).
- 내 언어에 잘못된 단어가 있나요? [번역 문제 신고](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml).

*이 페이지는 AI가 번역했습니다. 원어민의 도움을 환영합니다.* 라이선스: PolyForm Noncommercial 1.0.0.
