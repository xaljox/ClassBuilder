# Plan: `constexpr` voor methodes en members

Status: **UITGEVOERD 2026-10-29** (voorstel uit de Dammen64-sessie). Zie [Uitvoering](#uitvoering-2026-10-29) onderaan voor
wat er anders is gedaan dan hieronder voorgesteld, vooral het formaat.

## Aanleiding

In Dammen64 (`~/Projects/Dammen64`) stonden tabelbouwers die de compiler uitrekent (`constexpr`) in een naamloze
namespace in een USER-sectie. Jimmy wil zo weinig mogelijk code in USER-secties: functies horen methodes te zijn,
tabellen statische members. ClassBuilder kent `constexpr` niet, dus nu werkt het met een omweg:

- een Other type met de naam `constexpr X` als returntype, bijv. `constexpr std::uint64_t`, `constexpr TriangleCones`;
- de methode `static` + `inline`, zodat de definitie in de header staat:
  `static constexpr std::uint64_t MakeCone(int bit, bool white);` en na de klasse
  `inline constexpr std::uint64_t Position::MakeCone(int bit, bool white) { ... }`;
- de tabel een statisch member van het type `const TriangleCones` met als initialisatie `MakeTriangleCones()`. In de
  .cpp wordt dat `const TriangleCones Position::_triangleCones = MakeTriangleCones();`. Omdat de bouwer constexpr is
  en al gedefinieerd (header), doet de compiler constante initialisatie: de tabel staat in `__TEXT,__const`,
  gecontroleerd met `nm -m`.

Een member van het type `constexpr X` werkt **niet**. De header krijgt dan `static constexpr X _x;` zonder waarde, en
een `static constexpr` data member moet zijn waarde in de klasse krijgen.

## Formaat

- `kCurrentFormatVersion` (src/model/ClassBuilderDoc.cpp) van 3 naar **4**.
- `Method::_constexpr` (bool), in `Method::Serialize` achteraan schrijven, lezen achter `if (4 <= _objectVersion)`.
- `Member::_constexpr` (bool) op dezelfde manier (Member.cpp, read gate 4), alleen zinvol voor statische members.
- Kopieerconstructors/`operator=`/`Method(Method* pOld)` en `CbImport` (zoals `_dllExport`) bijwerken.
- Oude bestanden (versie <= 3): `_constexpr = false`, dus geen gedragsverandering.

## Code genereren

Methode met `_constexpr`:
- declaratie en definitie krijgen `constexpr` (voor het returntype, na `static`/`inline`);
- `constexpr` maakt een functie impliciet inline: de definitie hoort dan in de header, net als bij `inline`. Voorstel:
  constexpr genereert als inline (of de dialoog zet inline vast aan). Een constexpr-methode die in de .cpp staat
  werkt alleen in die ene vertaaleenheid;
- `constexpr` samen met `virtual` mag sinds C++20; `pure` niet.

Statisch member met `_constexpr`:
- de initialisatie gaat **in de klasse**: `static constexpr T _x = <initialisatie>;`, zonder definitie in de .cpp (sinds
  C++17 impliciet inline);
- niet-statische constexpr members bestaan niet in C++: weigeren of het vinkje uitschakelen.
- Let op: een initialisatie in de klasse die een constexpr-methode van **dezelfde** klasse aanroept compileert niet
  (de klasse is daar nog niet compleet). Voor dat geval blijft de huidige vorm nodig: `const T` met de initialisatie
  in de .cpp, waarbij de compiler constant initialiseert omdat de bouwer constexpr is. Misschien een melding als
  de initialisatie een methode van de eigen klasse noemt.

## GUI, API en terug inlezen

- Vinkje `constexpr` in de methode- en de memberdialoog. Het membervinkje alleen bij static.
- API: `add_method` en `add_member` met `"constexpr": true`, en `set_method_constexpr` / `set_member_constexpr`.
  `ToJson` van methode en member geeft `"constexpr"` terug.
- `read_source`: `constexpr` in de declaratie herkennen, zoals `static` en `inline`, anders volgt "interface of method
  ... has changed".

## Daarna in Dammen64

De Other types `constexpr X` vervangen door het vinkje. Dat gaat om de bouwers van Position: `MakeCone`,
`MakeTriangleCones`, `MakeBreakthroughCones`, `PairMask`, `ColumnIndex`, `MakeColumnBlocks`, `CheckColumnBlocks`,
`MakeTriangleValues`, `MakeBreakthroughValues`, en straks die van Endgame. Daarna moet `nodes` dezelfde knopen geven,
en de kegels moeten in `__TEXT,__const` staan.

## Andere waarnemingen uit de Dammen64-sessie (2026-10-29)

- `add_method` met een body die zelf `{ ... }` bevat: de accolades komen er dubbel in. De body moet zonder de buitenste
  accolades. Misschien de buitenste accolades er afhalen, of het in de beschrijving van de API zetten.
- Eenmaal verdween een handmatige wijziging in USER1 van het master include (Checkers.h) bij `write_source`, terwijl
  `read_source {"all":true}` er wel na de wijziging was gedaan. Bij herhalen bleef hij wel staan; de oorzaak is
  onbekend. Misschien werd er tussendoor vanuit de GUI opgeslagen.
- Het vinkje "Suppress forward declaration" (extern class) is alleen aan als de klasse niet erft en uitsluitend als
  pointer/referentie gebruikt wordt (ExternClassDialog.cpp:48-63). Dat is logisch, maar staat nergens in de GUI. Een
  tooltip zou helpen.
- `read_source` meldt "Warning interface of method ... has changed" bij elke **inline** methode waarvan ClassBuilder
  de signatuur in de header over twee regels heeft afgebroken, terwijl header en model gelijk zijn. In Dammen64:
  TransTable::Probe/Store, Position::BreakthroughKey/StopSearch/Position(white, black, king), MoveStack::Find/Push,
  MoveGenerator::AddMove. Afgebroken methodes in de .cpp (bijv. Endgame::Decode, Endgame::TableSize) geven de
  melding niet. Vermoedelijk leest de parser van de inline-definities in de header alleen de eerste regel van de
  signatuur.
- `add_constructor` met `seed_args: false` op een klasse met members: de gegenereerde initialisatielijst gebruikt
  argumenten die er niet zijn (`: _slots(slots), _pContext(pContext)` bij EndgameBlockCache in Dammen64). Verwacht:
  zonder argumenten geen initialisatie per member, of de standaardinitialisatie van het member.

## Uitvoering (2026-10-29)

### Formaat: een formaatkop, niet alleen versie 4

Het voorstel "`kCurrentFormatVersion` naar 4, lezen achter `if (4 <= _objectVersion)`" zou bestaande bestanden breken.
`_objectVersion` was bij het lezen de **projectversie** (`DataModelDoc::_version`), en die groeit bij elke keer dat de
bron wordt geschreven: Dammen64/Checkers.cbz staat op 138. De nieuwe CB zou daar een `constexpr`-veld verwachten dat
de oude CB nooit schreef, en het bestand als corrupt zien.

Daarom gaan formaat en projectversie nu apart ([src/model/CbCbzFile.h](../src/model/CbCbzFile.h)):

- opslaan: kop `0xCB2F0000`, daarna de echte projectversie; op de plek van `_version` in de stroom staat de
  **formaatversie** (`kCbzFormatVersion` = 4). De gates zien zo het formaat;
- laden met kop: na het lezen wordt de projectversie teruggezet. Een formaat boven 4 wordt geweigerd ("saved by a
  newer ClassBuilder");
- laden zonder kop (alle bestaande bestanden): gelezen als formaat `min(projectversie, 3)`, precies zoals vroeger;
- alle vijf lezers/schrijvers (openen, opslaan, crash-opslag, `load_cb`, `save_cb`) gaan via `Cb_ReadCbz` / `Cb_WriteCbz`.

Getest: Checkers (138), Matrix (6) en het ClassBuilder-model laden; na opslaan in formaat 4 is de stroom achter de kop
byte voor byte gelijk aan het oude bestand, en opnieuw laden geeft hetzelfde aantal objecten.

**Gevolg:** een bestand dat de nieuwe CB opslaat, kan een oudere CB niet openen ("file is corrupt"). Op Windows dus
eerst pullen en bouwen, en dan pas bestanden openen die op de Mac zijn opgeslagen.

Meegenomen (zelfde oorzaak):
- `OnOpenDocument` testte voor de CRLF-regel (`< 3`) en de reset `DataModelDoc::_objectVersion`, een **andere** static
  dan die van de gates (`DataModelDocObject::_objectVersion`) en die nergens gezet werd. Het eerste bestand van elke
  sessie verloor daardoor een LF-keuze. Nu op `info.format`;
- undo van een documentinstelling (`SerializeMembersOnly`) liet de gate-static op de projectversie staan; nu weer
  `INT_MAX`.

### Versienummers in het ClassBuilder-model

Het model is eerst teruggezet naar versie 3 (Compact Version), zodat de nieuwe velden `InitialVersion` 4 kregen. Let
op bij een volgend veld: de leescode van een nieuw veld ontstaat pas bij de **tweede** `write_source` (de eerste wordt
nog met de oude modelversie gegenereerd). Daarna moet `kCbzFormatVersion` mee omhoog. De API heeft daarvoor nu
`get_version` en `compact_version`.

### Wat er is gebouwd

- Model: `Method::_constexpr` en `Member::_constexpr` (bool, gate 4), met getter/setter `GetConstexpr()` /
  `SetConstexpr(bool val)`, in alle constructors, kopieerconstructors en `CopyValuesFrom`. Een nieuwe methode neemt
  constexpr over van de vorige, net als inline.
- Codegen methode: `constexpr` in declaratie en definitie; de definitie gaat naar `CB_INLINES` in de header
  (`GetInline() || GetConstexpr()`). Het Inline-vinkje zelf blijft los. constexpr-constructors/-destructors worden
  nog niet ondersteund (hun declaratie wordt elders opgebouwd), de API weigert ze.
- Codegen member: alleen bij static: `static constexpr T _x = <initialisatie>;` in de klasse, niets in de .cpp.
- `read_source`: de vergelijking van de signatuur negeert nu witruimte (dat loste ook de valse melding bij afgebroken
  inline-methodes op), en het blok met statische members in de .cpp slaat constexpr-members over.
- GUI: vinkje Constexpr in de methodedialoog (zelfde regels als Inline; geen sneltoets, alle letters waren bezet) en
  "Conste&xpr" in de memberdialoog (alleen bij Static).
- API: `"constexpr"` in de records; `add_method` / `add_member` met `constexpr`; `set_method_constexpr`,
  `set_member_constexpr`; `set_member_static false` wist constexpr. Waarschuwing bij een constexpr-member zonder
  initialisatie of met een aanroep van een methode van dezelfde klasse.

### De andere waarnemingen

- Dubbele accolades bij `add_method`: een body die als geheel `{ ... }` is, verliest die buitenste accolades (ook bij
  `set_method_body`); het antwoord zegt dat. **Opgelost.**
- `seed_args: false` met members: de initialisatielijst gebruikt alleen nog argumenten die de constructor heeft; een
  member zonder argument wordt `_x()`. **Opgelost** (`Constructor::InitInit`).
- Tooltip bij "Suppress forward declaration" met de regel wanneer het vinkje aan kan. **Opgelost.**
- USER1 van het master include verdwenen bij `write_source`: **niet onderzocht** (niet te reproduceren). Openstaand.
