# Nestingator3000 - Status i Lista TODO

## ZROBIONE (COMPLETED FEATURES):
1. **Parser DXF:**
   - ObSluga `LINE`, `ARC` (z wyliczaniem kierunkow CCW/CW oraz kiewunkow G02/G03), `CIRCLE`, `LWPOLYLINE`, `POLYLINE`.
   - Poprawne przeksztalcanie lukow z parametru bulge (w tym luki $> 180^\circ$ i $< -180^\circ$).
   - Matematyczna ewaluacja B-Spline (`SPLINE`) algorytmem Cox-De Boora.
   - Aproksymacja elips (`ELLIPSE`).

2. **Silnik Rozkroju (NestingEngine):**
   - Prawdziwe pozycjonowanie w oparciu o wektory stykowe konturow (algorytm NFP / sliding-contact).
   - Obsluga nestingator wewnatrz otworow (`innerContours` / shape-in-shape).
   - Obliczanie powierzchni wzorem Shoelace.
   - Obsluga nieregularnych plyt resztkowych (`remnantOuterPolygon`).
   - Elastyczny obrot detali ze skokiem katowym (`angleStep` np. 15 deg lub 30 deg).
   - Zrównoleglenie 7 strategii sortowania na osobnych watkach za pomoca `std::async`.
   - Limity wydajnosciowe $O(N)$ poprzez `NFP_ANCHOR_WINDOW = 40`.

3. **Generator NC (NCGenerator):**
   - Bezwzgledne wycinanie otworow wewnetrznych przed obwodem zewnetrznym.
   - Kompensacja promienia frezu G41 / G42.
   - Mostki technologiczne (tabs/bridges) zapobiegajace przemieszczaniu detali.
   - Wielokrotne przejscia Z (multi-pass repeats).
   - Ciecilosc czesciowa na wybranych krawedziach (partial-depth cuts).
   - Plynny, ciagly ruch bez zbednych uniesien wrzeciona (`chainEntities`).

4. **Interfejs Graficzny (Win32 API):**
   - Czysty styl RETRO w standardzie ANSI (brak problemow z kodowaniem znakow).
   - Rysowanie prawdziwych konturow wielokatow i otworow wewnetrznych w Canvasie GDI.
   - Suwak animacji przejazdu narzedzia (Toolpath Simulator) w czasie rzeczywistym.
   - Przegladarka BAZA DETALI skanujaca katalog `BazaDXF/` z podgladem i podwojnym kliknieciem.
   - Ochrona przed nadpisywaniem modyfikowanych plikow uzytkownika przy starcie.
   - Paskowy eksport do prawdziwego pliku PDF (`PDFWriter`) oraz archiwum ZIP (`ZIPWriter`).

---

## PLANOWANE ROZWINIECIA PRODUKCYJNYCH (FUTURE ROADMAP):
1. Dalsza optymalizacja kompensacji promienia frezu w czasie rzeczywistym na podgladzie GDI.
2. Integracja bezposredniej komunikacji z kontrolerem Osai przez port szeregowy/RS232/Ethernet.
3. Import trojwymiarowych modeli STEP/IGES z automatycznym spłaszczaniem do 2D.
