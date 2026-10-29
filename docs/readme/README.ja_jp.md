<!-- Short README in 日本語. The full guide (English): ../../README.md -->
# ETS2 ATS VR Controls: VRコントローラーで VR の Euro Truck Simulator 2 と American Truck Simulator を運転しよう

> [!CAUTION]
> **この Mod は https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases からのみダウンロードしてください。** 公式サイト、Discord、
> Nexus、Steam ワークショップはありません。他のサイト、ミラー、「インストーラー」で配布されているものは私のものではありません。実行しないでください。

![ETS2 / ATS VR Controls](../images/banner.png)

**ヘッドセットとそのコントローラーだけ**で、VR で **Euro Truck Simulator 2** と **American Truck Simulator** をプレイできます。ハンドルも、
ゲームパッドも、キーボードも、マウスも要りません。

- **コントローラーで運転:** スティック、片手または両手で握るハンドル、またはポインティングで操舵します。トリガーが
  アクセルとブレーキです。
- **ボタンに好きなキーを割り当て:** ウインカー、ホーン、ライトなどを 4 つのレイヤーで使えます（グリップを押し続けるとボタンが増えます）。
- **手の中のマウス:** ゲームのメニューやマップ用です。
- **ヘッドセット内のフルキーボード**で、お使いの国のキーボードのキーが使えます。
- **VR メニューはお使いの言語で:** ゲームの言語に自動で合わせます。

## 必要なもの

- Windows 10 / 11（64 ビット）、ETS2 または ATS、VR ヘッドセット（Quest、Index、Vive、Pico、Windows Mixed Reality など）。
- **OpenXR** の VR モードで動くゲーム。Steam: ゲームを右クリック > プロパティ > 起動オプション: `-openxr`
  （通常版のゲームでは `-experimental_vr` も）。`-openvr` と `-oculus` モードはこの Mod では動きません。

## インストール

1. [Releases](https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases) ページから最新の `ETS2-ATS-VR-Controls-....zip` をダウンロードします。
   ゲームを終了してください。
2. ずっと置いておけるフォルダー（例: `C:\ETS2 ATS VR Controls`）に展開します。
3. `install.ps1` を右クリック > **PowerShell で実行**。Windows の管理者確認で **はい** をクリックします。
4. ゲームを VR で起動します。

起動のたびに、ゲームに "Request to use advanced SDK features detected"（高度な SDK 機能の使用要求を検出）と表示されます。
これは（運転のための）この Mod です。OK を指してクリックしてください。

**VR メニュー:** **両方のグリップ + 左スティック押込**を 1 秒間押し続けます。ここで、すべてのボタン、マウス、
操舵、色、言語を設定できます。

アンインストール: 同じ方法で `uninstall.ps1` を実行し、フォルダーを削除します。

## その他

- **完全なガイド**（英語）: [README](../../README.md)。
- 問題がありますか？ [バグを報告](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new/choose)。
- お使いの言語に間違った言葉がありますか？ [翻訳の問題を報告](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml)。

*このページは AI によって翻訳されました。ネイティブスピーカーの協力を歓迎します。* ライセンス: PolyForm Noncommercial 1.0.0。
