<!-- Short README in Türkçe. The full guide (English): ../../README.md -->
# ETS2 ATS VR Controls: Euro Truck Simulator 2 ve American Truck Simulator'ı VR kontrolcüleriyle VR'da oynayın

> [!CAUTION]
> **Bu modu yalnızca https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases adresinden indirin.** Web sitesi, Discord,
> Nexus ya da Steam Workshop yoktur. Modu sunan başka her site, yansı veya "yükleyici" bana ait değildir: çalıştırmayın.

![ETS2 / ATS VR Controls](../images/banner.png)

**Euro Truck Simulator 2** ve **American Truck Simulator**'ı VR'da **yalnızca başlığınız ve kontrolcüleriyle** oynayın.
Direksiyon, oyun kolu, klavye ve fare gerekmez.

- **Kontrolcülerle sürüş:** çubukla, bir ya da iki elinizle tuttuğunuz direksiyonla ya da nişan alarak yönlendirin.
  Tetikler gaz ve frendir.
- **Bir düğmede istediğiniz tuş:** sinyaller, korna, farlar ... dört katmanda (daha fazla düğme için bir grip'i basılı tutun).
- **Elinizde bir fare:** oyunun menüleri ve harita için.
- **Başlığın içinde tam bir klavye**, ülkenizin klavyesinin tuşlarıyla.
- **Kendi dilinizde VR menüsü:** oyunun diline kendiliğinden uyar.

## Gerekenler

- Windows 10 / 11 (64 bit), ETS2 veya ATS ve bir VR başlığı (Quest, Index, Vive, Pico, Windows Mixed Reality ...).
- Oyun **OpenXR** VR modunda. Steam: oyuna sağ tıklayın > Özellikler > Başlatma seçenekleri: `-openxr`
  (normal oyun sürümünde ayrıca `-experimental_vr`). `-openvr` ve `-oculus` modları bu modla çalışmaz.

## Kurulum

1. [Releases](https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases) sayfasından en yeni `ETS2-ATS-VR-Controls-....zip`
   dosyasını indirin. Oyunu kapatın.
2. Kalabileceği bir klasöre çıkarın (örneğin `C:\ETS2 ATS VR Controls`).
3. `install.ps1` dosyasına sağ tıklayın > **PowerShell ile çalıştır**. Windows yönetici isteminde **Evet**'e tıklayın.
4. Oyunu VR'da başlatın.

Oyun her açılışta "Request to use advanced SDK features detected" (gelişmiş SDK özelliklerini kullanma isteği algılandı)
iletisini gösterir. Bu, bu moddur (sürüş için): OK'e nişan alıp tıklayın.

**VR menüsü:** **iki grip + sol çubuk tıklaması** tuşlarını 1 saniye basılı tutun. Orada tüm düğmeleri, fareyi, direksiyonu,
renkleri ve dili ayarlarsınız.

Kaldırma: `uninstall.ps1` dosyasını aynı şekilde çalıştırın, sonra klasörü silin.

## Daha fazlası

- **Tam kılavuz** (İngilizce): [README](../../README.md).
- Bir sorun mu var? [Hata bildirin](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new/choose).
- Dilinizde yanlış bir sözcük mü var? [Çeviri sorunu bildirin](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml).

*Bu sayfa yapay zekâ ile çevrilmiştir. Ana dili konuşanların yardımı memnuniyetle karşılanır.* Lisans: PolyForm Noncommercial 1.0.0.
