<!-- Short README in Norsk (bokmål). The full guide (English): ../../README.md -->
# ETS2 ATS VR Controls: kjør Euro Truck Simulator 2 og American Truck Simulator i VR med VR-kontrollerne dine

> [!CAUTION]
> **Last bare ned denne modden fra https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases.** Det finnes ingen nettside, ingen Discord,
> ingen Nexus, ingen Steam Workshop. Alle andre nettsteder, speil eller «installasjonsprogrammer» som tilbyr den er ikke mine: ikke kjør dem.

![ETS2 / ATS VR Controls](../images/banner.png)

Spill **Euro Truck Simulator 2** og **American Truck Simulator** i VR med **bare headsettet og kontrollerne**. Ingen
ratt, ingen spillkontroll, ikke noe tastatur, ingen mus.

- **Kjør med kontrollerne:** styr med spaken, med et ratt du holder med én eller begge hender, eller ved å peke. Triggerne
  er gass og brems.
- **Hvilken som helst tast på en knapp:** blinklys, horn, lys ... i fire lag (hold en grep for flere knapper).
- **En mus i hånden:** for menyene i spillet og kartet.
- **Et helt tastatur i headsettet**, med tastene fra tastaturet i landet ditt.
- **VR-menyen på ditt språk:** den følger språket i spillet av seg selv.

## Dette trenger du

- Windows 10 / 11 (64-bit), ETS2 eller ATS og et VR-headset (Quest, Index, Vive, Pico, Windows Mixed Reality ...).
- Spillet i **OpenXR** VR-modus. Steam: høyreklikk på spillet > Egenskaper > Startalternativer: `-openxr`
  (i den vanlige spillversjonen også `-experimental_vr`). Modusene `-openvr` og `-oculus` fungerer ikke med denne modden.

## Installere

1. Last ned den nyeste `ETS2-ATS-VR-Controls-....zip` fra siden
   [Releases](https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases). Lukk spillet.
2. Pakk den ut i en mappe der den kan bli liggende (for eksempel `C:\ETS2 ATS VR Controls`).
3. Høyreklikk på `install.ps1` > **Kjør med PowerShell**. Klikk **Ja** i Windows' administratorvindu.
4. Start spillet i VR.

Ved hver start viser spillet «Request to use advanced SDK features detected». Det er denne modden (for kjøringen): pek på
OK og klikk.

**VR-menyen:** hold **begge grepene + klikket på venstre spak** i 1 sekund. Der stiller du inn hver knapp, musen,
styringen, fargene og språket.

Avinstallere: kjør `uninstall.ps1` på samme måte, og slett deretter mappen.

## Mer

- **Hele veiledningen** (på engelsk): [README](../../README.md).
- Et problem? [Meld fra om en feil](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new/choose).
- Feil ord på språket ditt? [Meld fra om et oversettelsesproblem](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml).

*Denne siden er oversatt av KI. Hjelp fra morsmålsbrukere er velkommen.* Lisens: PolyForm Noncommercial 1.0.0.
