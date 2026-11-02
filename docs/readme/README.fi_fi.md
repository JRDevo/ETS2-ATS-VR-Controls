<!-- Short README in Suomi. The full guide (English): ../../README.md -->
# ETS2 ATS VR Controls: aja Euro Truck Simulator 2:ta ja American Truck Simulatoria VR:ssä VR-ohjaimilla

> [!CAUTION]
> **Lataa tämä modi vain osoitteesta https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases.** Verkkosivua, Discordia,
> Nexusta tai Steam Workshopia ei ole. Mikään muu sivusto, peilipalvelin tai "asennusohjelma", joka tarjoaa sitä, ei ole minun: älä suorita sitä.

![ETS2 / ATS VR Controls](../images/banner.png)

Pelaa **Euro Truck Simulator 2:ta** ja **American Truck Simulatoria** VR:ssä **pelkillä VR-laseilla ja niiden ohjaimilla**. Ei
ohjauspyörää, ei peliohjainta, ei näppäimistöä, ei hiirtä.

- **Aja ohjaimilla:** ohjaa tatilla, yhdellä tai kahdella kädellä pidettävällä rattirenkaalla tai osoittamalla. Liipaisimet
  ovat kaasu ja jarru.
- **Mikä tahansa näppäin painikkeeseen:** vilkut, äänitorvi, valot ... neljällä tasolla (pidä grippiä pohjassa, niin saat lisää painikkeita).
- **Hiiri kädessäsi:** pelin valikoihin ja karttaan.
- **Täysi näppäimistö VR-laseissa**, oman maasi näppäimistön näppäimillä.
- **VR-valikko omalla kielelläsi:** se seuraa pelin kieltä itse.

## Mitä tarvitset

- Windows 10 / 11 (64-bittinen), ETS2 tai ATS ja VR-laitteisto (Quest, Index, Vive, Pico, Windows Mixed Reality ...).
- Peli **OpenXR**-VR-tilassa. Steam: napsauta peliä hiiren kakkospainikkeella > Ominaisuudet > Käynnistysasetukset: `-openxr`
  (tavallisessa peliversiossa myös `-experimental_vr`). Tilat `-openvr` ja `-oculus` eivät toimi tämän modin kanssa.

## Asennus

1. Lataa uusin `ETS2-ATS-VR-Controls-Setup-....exe` [Releases](https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases)-sivulta.
   Sulje peli.
2. Suorita se. Napsauta Windowsin järjestelmänvalvojan kehotteessa **Kyllä** ja sitten loppuun asti **Seuraava**. Modi asennetaan kansioon
   `C:\Program Files\ETS2 ATS VR Controls` (voit valita toisen kansion). Asetuksesi (`config.ini`) ja lokit menevät kansioon
   `C:\ProgramData\ETS2 ATS VR Controls`.
3. Käynnistä peli VR:ssä.

Joka käynnistyksellä peli näyttää ilmoituksen "Request to use advanced SDK features detected" (pyyntö käyttää
kehittyneitä SDK-ominaisuuksia). Se on tämä modi (ajamista varten): osoita OK:ta ja napsauta.

**VR-valikko:** pidä **molempia gripejä + vasenta tatin painallusta** 1 sekunnin ajan. Siellä määrität jokaisen painikkeen, hiiren,
ohjauksen, värit ja kielen.

Asennuksen poisto: Windows **Asetukset > Sovellukset > Asennetut sovellukset > ETS2 ATS VR Controls > Poista asennus**. Se poistaa modin,
sen rekisterimerkinnät ja lokit. Se kysyy, poistetaanko myös asetuksesi (Ei = säilytä ne myöhempää asennusta varten).

Ilman asennusohjelmaa: samalla sivulla oleva `.zip` sisältää tiedostot `install.ps1` ja `uninstall.ps1` (katso täydellinen opas).

## Lisää

- **Täydellinen opas** (englanniksi): [README](../../README.md).
- Ongelma? [Ilmoita virheestä](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new/choose).
- Väärä sana omalla kielelläsi? [Ilmoita käännösongelmasta](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml).

*Tämän sivun on kääntänyt tekoäly. Äidinkielisten puhujien apu on tervetullutta.* Lisenssi: PolyForm Noncommercial 1.0.0.
