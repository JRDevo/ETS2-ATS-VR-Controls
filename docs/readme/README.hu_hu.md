<!-- Short README in Magyar. The full guide (English): ../../README.md -->
# ETS2 ATS VR Controls: vezesd az Euro Truck Simulator 2-t és az American Truck Simulatort VR-ban a VR-kontrollereiddel

> [!CAUTION]
> **Ezt a modot csak innen töltsd le: https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases.** Nincs weboldal, nincs Discord,
> nincs Nexus, nincs Steam Workshop. Bármely más oldal, tükör vagy „telepítő”, amely kínálja, nem az enyém: ne futtasd.

![ETS2 / ATS VR Controls](../images/banner.png)

Játssz az **Euro Truck Simulator 2**-vel és az **American Truck Simulatorral** VR-ban **csak a headsettel és a kontrollerekkel**. Nincs
kormány, nincs gamepad, nincs billentyűzet, nincs egér.

- **Vezetés a kontrollerekkel:** kormányozz a sticckel, egy vagy két kézzel fogott kormánnyal, vagy mutatással. A
  ravaszok a gáz és a fék.
- **Bármelyik billentyű egy gombon:** index, duda, lámpák ... négy rétegben (tarts nyomva egy gripet több gombért).
- **Egér a kezedben:** a játék menüihez és a térképhez.
- **Teljes billentyűzet a headsetben**, az országod billentyűzetének billentyűivel.
- **A VR-menü a te nyelveden:** magától követi a játék nyelvét.

## Mire van szükséged

- Windows 10 / 11 (64 bites), ETS2 vagy ATS, és egy VR-headset (Quest, Index, Vive, Pico, Windows Mixed Reality ...).
- A játék **OpenXR** VR módban. Steam: kattints jobb gombbal a játékra > Tulajdonságok > Indítási beállítások: `-openxr`
  (a normál játékverzióban `-experimental_vr` is). Az `-openvr` és az `-oculus` mód nem működik ezzel a moddal.

## Telepítés

1. Töltsd le a legújabb `ETS2-ATS-VR-Controls-....zip` fájlt a
   [Releases](https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases) oldalról. Zárd be a játékot.
2. Csomagold ki egy olyan mappába, ahol maradhat (például `C:\ETS2 ATS VR Controls`).
3. Kattints jobb gombbal az `install.ps1` fájlra > **Futtatás a PowerShell-lel**. A Windows rendszergazdai kérdésénél kattints az **Igen** gombra.
4. Indítsd el a játékot VR-ban.

A játék minden indításkor megjeleníti: „Request to use advanced SDK features detected” (kérés speciális SDK-funkciók
használatára). Ezt ez a mod okozza (a vezetéshez): mutass az OK-ra, és kattints.

**A VR-menü:** tartsd nyomva **mindkét gripet + a bal stick lenyomását** 1 másodpercig. Itt állítod be az összes gombot, az egeret, a
kormányzást, a színeket és a nyelvet.

Eltávolítás: futtasd ugyanígy az `uninstall.ps1` fájlt, majd töröld a mappát.

## Továbbiak

- **A teljes útmutató** (angolul): [README](../../README.md).
- Probléma van? [Hiba jelentése](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new/choose).
- Rossz szó a nyelveden? [Fordítási hiba jelentése](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml).

*Ezt az oldalt mesterséges intelligencia fordította. Az anyanyelvi beszélők segítségét szívesen fogadjuk.* Licenc: PolyForm Noncommercial 1.0.0.
