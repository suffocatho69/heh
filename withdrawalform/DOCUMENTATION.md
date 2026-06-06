# Dokumentacja Modułu: Odstąp od umowy tutaj (withdrawalform)

## 1. Wstęp
Moduł „Odstąp od umowy tutaj” został stworzony dla platformy PrestaShop 8.1.1 (kompatybilny z 1.7.x - 9.x) w celu zapewnienia pełnej zgodności z dyrektywą unijną z dnia 19 czerwca 2026 r. oraz specyficznym regulaminem sklepu orientica.pl. Moduł umożliwia klientom (zarówno zarejestrowanym, jak i gościom) cyfrowe złożenie oświadczenia o odstąpieniu od umowy bez konieczności drukowania dokumentów.

## 2. Kluczowe Funkcjonalności

### 2.1. Formularz Elektroniczny
- Dostępny pod przyjaznym adresem: `TwojaDomena.pl/zwroty`.
- Pełna integracja tekstu regulaminu sklepu bezpośrednio w formularzu.
- Design dopasowany do stylistyki orientica.pl (ciemne tło, akcenty kolorystyczne #f79c4b i #d4af37).

### 2.2. Obsługa Zwrotów Częściowych
- Klient może wybrać konkretne produkty z zamówienia, które chce zwrócić.
- Możliwość określenia ilości zwracanych sztuk dla każdego produktu.
- Walidacja po stronie serwera: system uniemożliwia wpisanie ilości ujemnej lub większej niż zakupiona.

### 2.3. Dostęp dla Gości (Guest Access)
- Specjalny mechanizm autoryzacji oparty na `secure_key` zamówienia.
- Goście nie muszą zakładać konta, aby skorzystać z formularza – otrzymują unikalny, bezpieczny link w wiadomościach e-mail.

### 2.4. Automatyzacja Powiadomień
- **Przy Dostawie:** Po zmianie statusu zamówienia na „Dostarczone” (lub dowolny status z flagą dostawy), system automatycznie wysyła do klienta dedykowaną wiadomość e-mail z informacją o prawie do zwrotu i bezpośrednim linkiem do formularza.
- **Potwierdzenie Zgłoszenia:** Po wysłaniu formularza, zarówno klient, jak i administrator sklepu otrzymują szczegółowe potwierdzenie (HTML oraz TXT) zawierające listę zwracanych towarów.

### 2.5. Inteligentny Termin Ważności (14 dni)
- Formularz automatycznie blokuje się po upływie 14 dni.
- Czas liczony jest od **daty dostarczenia** zamówienia (jeśli jest uzupełniona w systemie) lub od daty złożenia zamówienia.

### 2.6. Integracja z Panelem Administracyjnym (Back Office)
- **Lista Zgłoszeń:** Nowa sekcja w menu Klienci -> Withdrawal Requests.
- **Podgląd Szczegółowy:** Możliwość podejrzenia każdego zgłoszenia, danych klienta, powodu odstąpienia oraz przejrzystej tabeli z listą i ilością zwracanych produktów.
- **Powiadomienia w Zamówieniu:** Każde zgłoszenie automatycznie dodaje prywatną wiadomość (notatkę) do danego zamówienia w panelu admina, co ułatwia pracę obsłudze sklepu.

## 3. Instalacja i Konfiguracja
1. Wgraj plik `withdrawalform.zip` przez Panel Administracyjny (Moduły -> Menedżer modułów).
2. Kliknij „Instaluj”.
3. W konfiguracji modułu możesz ustawić:
   - Limit dni na odstąpienie (domyślnie 14).
   - Tryb limitu (Soft – tylko ostrzeżenie, Hard – całkowita blokada formularza).
   - Ograniczenie do jednego zgłoszenia na zamówienie.

## 4. Techniczne aspekty
- Wykorzystuje standardowe mechanizmy `ObjectModel` PrestaShop.
- Bezpieczeństwo: Formularz chroniony tokenem CSRF (`Tools::getToken`).
- Kompatybilność z URL Rewrite (trasa `/zwroty`).
- Obsługa wielu języków (pełne tłumaczenie na j. polski).

---
*Autor: Jules*
*Data: 2026-05-26*
