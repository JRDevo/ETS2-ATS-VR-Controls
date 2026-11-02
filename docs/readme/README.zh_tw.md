<!-- Short README in 繁體中文. The full guide (English): ../../README.md -->
# ETS2 ATS VR Controls：用 VR 控制器在 VR 中駕駛《歐洲卡車模擬 2》（Euro Truck Simulator 2）和《美國卡車模擬》（American Truck Simulator）

> [!CAUTION]
> **只能從 https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases 下載此 mod。** 沒有網站、沒有 Discord、
> 沒有 Nexus、也沒有 Steam 工作坊。任何其他提供它的網站、鏡像或「安裝程式」都不是我發布的：請不要執行。

![ETS2 / ATS VR Controls](../images/banner.png)

**只用頭戴式顯示器和它的控制器**，就能在 VR 中遊玩 **Euro Truck Simulator 2** 和 **American Truck Simulator**。不需要方向盤、
遊戲手把、鍵盤和滑鼠。

- **用控制器駕駛：** 用搖桿轉向，或用單手或雙手握住的方向盤轉向，也可以用指向的方式。扳機就是油門和煞車。
- **任意按鍵綁定到控制器按鍵：** 方向燈、喇叭、車燈 ……共四層（按住握把可使用更多按鍵）。
- **手中的滑鼠：** 用於遊戲選單和地圖。
- **頭戴式顯示器中的完整鍵盤**，按鍵與你所在國家的鍵盤一致。
- **VR 選單使用你的語言：** 自動跟隨遊戲的語言。

## 你需要什麼

- Windows 10 / 11（64 位元）、ETS2 或 ATS，以及一台 VR 頭戴式顯示器（Quest、Index、Vive、Pico、Windows Mixed Reality ……）。
- 遊戲使用 **OpenXR** VR 模式。Steam：在遊戲上按右鍵 > 內容 > 啟動選項：`-openxr`
  （一般版遊戲還要加 `-experimental_vr`）。`-openvr` 和 `-oculus` 模式無法搭配此 mod 使用。

## 安裝

1. 從 [Releases](https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases) 頁面下載最新的 `ETS2-ATS-VR-Controls-Setup-....exe`。
   關閉遊戲。
2. 執行它。在 Windows 的系統管理員權限提示中按一下 **是**，然後一路按 **下一步**。此 mod 會安裝到
   `C:\Program Files\ETS2 ATS VR Controls`（可以選擇其他資料夾）。你的設定（`config.ini`）和記錄會儲存到
   `C:\ProgramData\ETS2 ATS VR Controls`。
3. 在 VR 中啟動遊戲。

每次啟動時，遊戲都會顯示「Request to use advanced SDK features detected」（偵測到使用進階 SDK 功能的要求）。這就是此 mod
（用於駕駛）：指向 OK 並按下。

**VR 選單：** 同時按住**兩個握把 + 左搖桿按下** 1 秒。在這裡可以設定每個按鍵、滑鼠、轉向、顏色和語言。

解除安裝：Windows **設定 > 應用程式 > 已安裝的應用程式 > ETS2 ATS VR Controls > 解除安裝**。它會移除此 mod、它的登錄項目和記錄。它會詢問
是否也刪除你的設定（否 = 保留以供日後安裝）。

不使用安裝程式：同一頁面上的 `.zip` 包含 `install.ps1` 和 `uninstall.ps1`（見完整指南）。

## 更多

- **完整指南**（英文）：[README](../../README.md)。
- 有問題？[回報錯誤](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new/choose)。
- 你的語言中有用詞不當？[回報翻譯問題](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml)。

*本頁由 AI 翻譯。歡迎母語人士協助改進。* 授權條款：PolyForm Noncommercial 1.0.0。
