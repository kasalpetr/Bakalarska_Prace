## Kontext
Při výuce a workshopech vzniká velké množství užitečných informací, které účastníci zaznamenávají na tabuli nebo flipcharty. Tyto informace je často potřeba zachovat pro další práci na projektu nebo pro navázání na workshop v následujících dnech. V současné praxi si uživatelé řešení obvykle pouze vyfotí, přičemž následně musí obsah ručně přepisovat nebo převádět do digitální podoby. Tento postup je časově náročný a často vede ke ztrátě informací nebo jejich významu.

Existující nástroje pro rozpoznávání textu se většinou zaměřují na blokově psaný text a nedokáží si dostatečně poradit se složitějším obsahem, jako jsou diagramy, strukturované poznámky, šipky nebo kombinace textu a kresby. Dalším problémem je nedostatečná integrace s nástroji, které by umožnily takto digitalizovaný, graficky orientovaný obsah dále zobrazovat, upravovat a rozvíjet.

## Cíl práce

Cílem práce je navrhnout a implementovat nástroj, který umožní uživateli jednoduše digitalizovat ručně psané a kreslené poznámky z tabule pomocí fotografie a převést je do editovatelné podoby ve vhodně zvolené cílové platformě.

## Výzkumné otázky

- Jaká architektura je vhodná pro nástroj umožňující digitalizaci ručně psaných a kreslených poznámek z fotografie?
- Jaká cílová platforma je nejvhodnější pro zobrazení a další práci s takto digitalizovaným obsahem?

## Rozsah práce (Scope)

Práce se zaměřuje na vytvoření proof-of-concept aplikace, která demonstruje funkční proces digitalizace ručně psaných a kreslených poznámek z tabule do digitálního prostředí zvolené cílové platformy. Rozsah podporovaných typů digitalizovaných objektů bude stanoven na základě úvodní analýzy uživatelského workflow, možností cílové platformy a dostupných nástrojů či knihoven pro digitalizaci obsahu.

## Metodika

- Průzkum existujících platforem pro digitální whiteboarding a výběr vhodné cílové platformy
- Průzkum existujících řešení pro digitalizaci ručně psaných poznámek do whiteboardových nástrojů
- Analýza uživatelského workflow při práci s fyzickou tabulí a následnou digitalizací obsahu
- Návrh architektury aplikace a uživatelského rozhraní
- Realizace prototypu
- Testování navrženého řešení
- Implementace a nasazení PoC s úpravami vycházejícími z výsledků testování
- Vypracování technické dokumentace umožňující další rozvoj projektu a zhodnocení aktuálních omezení navrženého řešení


## Sestavení aplikace

### Linux

Postup v adresáři App_W2B:

1. Spusť instalační skript:

```
cd App_W2B
chmod +x install.sh
./install.sh
```

Skript nainstaluje systémové závislosti, Python balíčky, aplikaci přeloží a vytvoří binárku.

2. Spusť aplikaci:

```
./build/DigitalBoard
```

### Windows (portable balíček)

1. Rozbal připravený ZIP balíček aplikace.

2. Nainstaluj Python 3 z python.org.

3. V rozbalené složce spusť run_windows.bat

Co se stane při prvním spuštění:
- vytvoří se lokální virtuální prostředí .venv
- doinstalují se Python závislosti
- spustí se DigitalBoard.exe

## Získání Api klíče do Mira
- Po příhlášení na https://miro.com/
- Vpravo nahoře pod ikonkou svého jméno - profil (nastavení profilu)
- Karta - Your apps 
- Tlačítko Create new app
- po vytvoření - přejít na část Permissions
    - zaškrtnout boards:read a boards:write -> Tlačítko Install app and get OAuth token
- Na závěr si už jen zkopírovat přístupový klíč

## Získání Api kliče v Google Vision
- Přihlásit na https://console.cloud.google.com/
- Z navigace vybrat APIs & Services -> Credentials
- Create Credentials -> API key
- Vybrat Cloud Vision API
- Na závěr vytvořit a uložit klíč