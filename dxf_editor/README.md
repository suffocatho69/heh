# Retro Win32 CAD/DXF Editor

Lekki i wysoce funkcjonalny edytor CAD / rysunków DXF 2D, stylizowany na interfejs z ery Windows 95/98 i Windows 2000.

Aplikacja opiera się w 100% na czystym Win32 API i standardowej bibliotece GDI, bez zależności od zewnętrznych silników czy bibliotek GUI takich jak Qt czy MFC.

Zaprojektowany z myślą o bezproblemowej kompilacji w środowiskach **Dev-C++ 5.11 (32-bit)**, MinGW oraz MSVC.

## Funkcje i możliwości
* **Szablon mechaniczny (Fusion 360 style):** Na starcie ładowany jest kompletny techniczny szkic szablonowy krawędzi (kapsuła z otworami o wymiarach 50.00 x 20.00, linie osiowe).
* **Narzędzia rysowania:** Linia, Polilinia, Prostokąt, Okrąg, Łuk, Elipsa, Tekst, Wymiarowanie liniowe, Punkt techniczny, Pięciokąt (Wielokąt).
* **Interaktywne modyfikacje krawędzi:**
  - **Gumka (Trim/Delete):** Usuwanie lub przycinanie sąsiednich krawędzi bezpośrednio kliknięciem myszy.
  - **Mirror (Odbicie lustrzane):** Odbicie wybranego elementu symetrycznie względem osi pionowej.
  - **Offset (Odsunięcie krawędzi):** Dodawanie równoległych odsunięć (np. dla okręgu / linii o zadany dystans).
  - **Przesuń / Skaluj:** Swobodne przenoszenie obiektów na rysownicy.
* **Import i eksport DXF:** Zapis i odczyt standardowych plików tekstowych ASCII DXF kompatybilnych z AutoCAD (LINE, CIRCLE, ARC, TEXT, VERTEX, POLYLINE).
* **Obszar roboczy:**
  - Podwójne buforowanie GDI zapobiegające migotaniu rysownicy.
  - Dynamiczny podgląd rysowania na żywo (rubber banding).
  - Pomocnicza siatka (Grid) z funkcją przyciągania krawędzi (Snap-to-Grid).
  - Skalowanie (Zoom) kółkiem myszy względem położenia kursora oraz przesuwanie (Pan) środkowym przyciskiem myszy.
  - Podział dolnych zakładek na `[Model]` i `[Arkusz1]`.
* **Klasyczny styl retro:** Szara stylistyka COLOR_BTNFACE `#C0C0C0`, obramowania 3D, retro przyciski, czcionka Tahoma 8pt.

## Instrukcja Kompilacji

### 1. Kompilacja w Dev-C++ 5.11:
1. Otwórz plik `dxf_editor.dev` w środowisku Dev-C++.
2. Wybierz profil kompilatora **TDM-GCC 4.9.2 Release (32-bit)**.
3. Kliknij **Execute -> Compile & Run** (F11).

### 2. Kompilacja z wiersza poleceń MinGW:
Uruchom poniższą komendę w terminalu:
```bash
g++ -static -o editor.exe main.cpp -lgdi32 -lcomctl32 -lcomdlg32
```

## Pobieranie i instalacja
Pełny kod źródłowy wraz z plikami projektu znajduje się w spakowanym archiwum ZIP.
Skonstruowana aplikacja jest gotowa do uruchomienia bezpośrednio na systemach Windows (lub za pomocą Wine na systemach macOS/Linux).
