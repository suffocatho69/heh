# Retro 2D Nesting System - Instrukcja Obsługi i Kompilacji

Witaj w profesjonalnym systemie do **nestingu 2D** (rozkroju detali z plików DXF na płytach bazowych) z czytelnym i schludnym interfejsem użytkownika stylizowanym na retro Win32 API.

Aplikacja została napisana w czystym standardzie **C++14**, z zachowaniem pełnej modularności, obiektowości i dbałości o bezbłędne operacje matematyczne.

---

## 🛠️ Architektura Klas i Modułów

Kod źródłowy został podzielony na logiczne, czytelne komponenty:
1. **`Component`**: Reprezentuje wczytany detal rozkroju, jego parametry geometryczne (wymiary X/Y, ilość sztuk, wektor geometrii linii, okręgów i łuków) oraz helpery do rotacji.
2. **`DXFReader`**: Szybki, bezbłędny parser formatu plików DXF, wyciągający jednostki geometryczne (`LINE`, `ARC`, `CIRCLE`) i automatycznie normalizujący ich współrzędne do układu lokalnego `(0,0)` oraz kalkulujący optymalną obwiednię (bounding box).
3. **`NestingEngine`**: Główny silnik rozkroju. Implementuje pakowanie 2D strategią **Best-Fit Descending** (sortowanie po polu powierzchni malejąco) oraz algorytm bottom-left scanning z obsługą rotacji o 90 stopni, marginesów arkusza i odstępów part-to-part.
4. **`NCGenerator`**: Tłumaczy ułożone detale na standardowy, przemysłowy **G-kod (ISO RS-274)**, sterując obrotami wrzeciona, posuwem roboczym, posuwem wejścia w materiał, wysokością przejazdu bezpiecznego (Z) oraz głębokością frezowania.
5. **`ComponentManager`**: Odpowiada za zarządzanie listą detali, dodawanie, usuwanie i dokładne wyliczanie statystyk wykorzystania materiału.
6. **`MainWindow`**: Czyste okienkowe Win32 API o logicznym układzie trzech paneli (Lista detali, parametry wejściowe płyty i maszynowe, retro podgląd graficzny GDI oraz statystyki).

---

## 📦 Struktura Projektu

```text
Nesting2D/
├── Nesting2D.dev       <- Plik projektu dla środowiska Dev-C++ 5.11
├── Makefile            <- Uniwersalny plik Makefile (Linux i MinGW GCC)
├── README.md           <- Niniejsza instrukcja użytkownika (Polish)
├── test_main.cpp       <- Testy jednostkowe CLI uruchamiane na Linuksie / konsoli
└── src/
    ├── Component.h / .cpp
    ├── ComponentManager.h / .cpp
    ├── DXFReader.h / .cpp
    ├── NestingEngine.h / .cpp
    ├── NCGenerator.h / .cpp
    ├── MainWindow.h / .cpp
    ├── main.cpp
    ├── resource.h
    └── resource.rc
```

---

## 🚀 Jak skompilować aplikację w Dev-C++ 5.11

Środowisko: **Dev-C++ 5.11**, kompilator **TDM-GCC 10.3.0** (lub standardowy TDM-GCC 4.9.2 dołączony domyślnie).

### Krok po kroku:
1. Skopiuj rozpakowany folder `Nesting2D` na swój komputer z systemem Windows.
2. Uruchom środowisko **Dev-C++**.
3. Kliknij `File -> Open Project or File...` (`Plik -> Otwórz Projekt lub Plik...`) i wybierz plik **`Nesting2D.dev`**.
4. Upewnij się, że w opcjach projektu wybrano standard języka **C++14**:
   - Wejdź w `Project -> Project Options` (`Projekt -> Opcje Projektu`).
   - Przejdź do zakładki `Compiler` (`Kompilator`).
   - W sekcji *Code Generation* upewnij się, że opcja `Language standard` jest ustawiona na `ISO C++11` lub **`ISO C++14`** (lub dopisz `-std=c++14` w polu *Compiler Options / Command line*).
5. Linkowanie bibliotek Win32 GDI i Common Controls jest już skonfigurowane w pliku `.dev` za pomocą flag:
   `-mwindows -lcomctl32 -lgdi32`
6. Naciśnij klawisz **`F9`** (Kompiluj) lub **`F11`** (Kompiluj i Uruchom).
7. Program skompiluje się bez ostrzeżeń i uruchomi ładne retro okienko.

---

## 🎮 Instrukcja Obsługi Interfejsu Retro

1. **Lista Komponentów (Lewy Panel)**:
   - Wyświetla nazwę, gabaryty X, Y oraz żądaną ilość każdego elementu.
   - Kliknięcie **Dodaj Ręcznie** dodaje nowy domyślny prostokątny detal na listę.
   - Kliknięcie **Usuń Zaznaczony** usuwa podświetloną pozycję.
   - Przycisk **Importuj plik DXF...** otwiera standardowe systemowe okno dialogowe wyboru pliku. Po wczytaniu prawidłowego pliku `.dxf`, program automatycznie obliczy obwiednię i doda go na listę z domyślną ilością sztuk.

2. **Parametry płyty i CNC (Środkowy Panel)**:
   - Pozwala zdefiniować szerokość i wysokość bazowego arkusza (płyty) w milimetrach.
   - **Margines krawędzi** określa bezpieczną odległość od krańca arkusza.
   - **Odstęp detali** to minimalna odległość pomiędzy sąsiadującymi elementami (uwzględniająca np. szerokość frezu).
   - Opcja **Zezwalaj na obrót o 90°** pozwala silnikowi na rotowanie elementów w celu ciaśniejszego upakowania.
   - Pola maszynowe (obroty, posuwy, wysokości bezpieczne i głębokość frezowania) posłużą do precyzyjnego wygenerowania końcowego programu G-code.

3. **Podgląd i Statystyki (Prawy Panel)**:
   - Kliknij duży przycisk **`URUCHOM NESTING`**.
   - Silnik natychmiast ułoży detale na minimalnej liczbie płyt, uwzględniając wszystkie parametry i narysuje efekt na ciemnoszarym, wektorowym retro-ekranie.
   - Po prawej stronie u góry pojawi się panel statystyk, pokazujący:
     - **Średnie zużycie materiału** (w procentach).
     - **Liczbę płyt ogółem** potrzebnych do rozkroju.
     - **Liczbę unikalnych układów** płyt.
     - Liczbę pomyślnie ułożonych elementów.
   - Z listy rozwijanej **Wybierz układ płyty** możesz przełączać podgląd graficzny między kolejnymi wygenerowanymi arkuszami.
   - Przycisk **Eksportuj plik .NC (G-kod)** zapisuje gotowy program sterujący na dysku. Program zawiera pełne ścieżki przejazdów szybkich G00, cięcia G01 oraz kołowych G02/G03 dla okręgów i łuków.
