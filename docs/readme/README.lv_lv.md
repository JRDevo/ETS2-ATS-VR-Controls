<!-- Short README in Latviešu. The full guide (English): ../../README.md -->
# ETS2 ATS VR Controls: brauciet Euro Truck Simulator 2 un American Truck Simulator VR ar VR kontrolieriem

> [!CAUTION]
> **Lejupielādējiet šo modu tikai no https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases.** Nav ne mājaslapas, ne Discord,
> ne Nexus, ne Steam Workshop. Jebkura cita vietne, spogulis vai „instalētājs”, kas to piedāvā, nav mans: nepalaidiet to.

![ETS2 / ATS VR Controls](../images/banner.png)

Spēlējiet **Euro Truck Simulator 2** un **American Truck Simulator** VR **tikai ar austiņām un to kontrolieriem**. Nekādas
stūres, nekāda spēļu kontrollera, nekādas tastatūras, nekādas peles.

- **Braukšana ar kontrolieriem:** stūrējiet ar stiku, ar stūri, ko turat vienā vai abās rokās, vai vienkārši norādot. Mēlītes
  ir gāze un bremze.
- **Jebkurš taustiņš uz pogas:** pagriezienu rādītāji, signāls, gaismas ... četros slāņos (turiet gripu, lai iegūtu vairāk pogu).
- **Pele jūsu rokā:** spēles izvēlnēm un kartei.
- **Pilna tastatūra austiņās** ar jūsu valsts tastatūras taustiņiem.
- **VR izvēlne jūsu valodā:** tā pati seko spēles valodai.

## Kas nepieciešams

- Windows 10 / 11 (64 bitu), ETS2 vai ATS un VR austiņas (Quest, Index, Vive, Pico, Windows Mixed Reality ...).
- Spēle **OpenXR** VR režīmā. Steam: ar peles labo pogu uz spēles > Properties > Palaišanas opcijas: `-openxr`
  (parastajā spēles versijā arī `-experimental_vr`). Režīmi `-openvr` un `-oculus` ar šo modu nedarbojas.

## Uzstādīšana

1. Lejupielādējiet jaunāko `ETS2-ATS-VR-Controls-....zip` no
   [Releases](https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases) lapas. Aizveriet spēli.
2. Izpakojiet to mapē, kur tā var palikt (piemēram, `C:\ETS2 ATS VR Controls`).
3. Ar peles labo pogu uz `install.ps1` > **Run with PowerShell**. Windows administratora uzvednē noklikšķiniet uz **Jā**.
4. Startējiet spēli VR.

Katrā startā spēle parāda „Request to use advanced SDK features detected”. Tas ir šis mods (braukšanai): norādiet uz
OK un noklikšķiniet.

**VR izvēlne:** turiet **abus gripus + kreisā stika spiedienu** 1 sekundi. Tur iestatāt katru pogu, peli,
stūrēšanu, krāsas un valodu.

Atinstalēšana: tāpat palaidiet `uninstall.ps1`, pēc tam izdzēsiet mapi.

## Vairāk

- **Pilnā pamācība** (angļu valodā): [README](../../README.md).
- Problēma? [Ziņot par kļūdu](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new/choose).
- Nepareizs vārds jūsu valodā? [Ziņot par tulkojuma problēmu](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml).

*Šo lapu iztulkoja MI. Dzimtās valodas runātāju palīdzība ir laipni gaidīta.* Licence: PolyForm Noncommercial 1.0.0.
