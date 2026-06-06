{extends file='customer/page.tpl'}

{block name='page_title'}
  {l s='Odstąp od umowy' mod='withdrawalform'}
{/block}

{block name='page_content'}
  <section id="withdrawal-form" style="background-color: #2c2c2c; padding: 20px; color: #f1e3d2; border-radius: 8px;">
    {if isset($success) && $success}
      <div class="alert alert-success">
        {l s='Twoje zgłoszenie odstąpienia od umowy zostało pomyślnie wysłane.' mod='withdrawalform'}
      </div>
      {if $customer.is_logged}
        <a href="{$link->getPageLink('history')}" class="btn btn-primary">{l s='Powrót do historii zamówień' mod='withdrawalform'}</a>
      {/if}
    {else}

      <div class="shop-regulation" style="margin-bottom: 30px; font-family: sans-serif; line-height: 1.6; max-height: 400px; overflow-y: auto; padding: 15px; background: rgba(0,0,0,0.1); border: 1px solid #5e4b3c;">
        <h1 style="color:#f79c4b;text-align:center;border-bottom:1px solid #5e4b3c;padding-bottom:10px;">regulamin sklepu internetowego orientica.pl</h1>

        <h2 style="color:#f79c4b;margin-top:25px;">§1 postanowienia ogólne</h2>
        <p>sklep internetowy <strong>orientica.pl</strong> prowadzony jest przez:<br>
        <strong style="color:#d4af37;">damian pańczak ph orientica</strong><br>
        ul. podgórze 15/4, 38-500 sanok<br>
        nip: 6912319929<br>
        e-mail: <a href="mailto:kontakt@orientica.pl" style="color:#f79c4b;text-decoration:none;">kontakt@orientica.pl</a><br>
        telefon: 517 211 301</p>
        <p>regulamin określa zasady korzystania ze sklepu, składania zamówień, zawierania umów sprzedaży, dostawy, płatności, odstąpienia od umowy oraz procedur reklamacyjnych. klient zobowiązany jest do zapoznania się z regulaminem przed złożeniem zamówienia.</p>

        <h2 style="color:#f79c4b;">§2 definicje</h2>
        <ul>
        <li><strong style="color:#d4af37;">sprzedawca</strong> – damian pańczak ph orientica</li>
        <li><strong style="color:#d4af37;">sklep</strong> – sklep internetowy dostępny pod adresem www.orientica.pl</li>
        <li><strong style="color:#d4af37;">klient</strong> – osoba fizyczna, prawna lub jednostka organizacyjna składająca zamówienie</li>
        <li><strong style="color:#d4af37;">konsument</strong> – osoba fizyczna dokonująca zakupu niezwiązanego bezpośrednio z działalnością gospodarczą</li>
        <li><strong style="color:#d4af37;">towar</strong> – produkty oferowane w sklepie</li>
        <li><strong style="color:#d4af37;">umowa sprzedaży</strong> – umowa zawarta pomiędzy sprzedawcą a klientem na odległość</li>
        <li><strong style="color:#d4af37;">dni robocze</strong> – od poniedziałku do piątku, z wyłączeniem dni ustawowo wolnych od pracy</li>
        </ul>

        <h2 style="color:#f79c4b;">§6 odstąpienie od umowy</h2>
        <h3 style="color:#d4af37;margin-top:18px;">6.1 prawo odstąpienia</h3>
        <p>konsument ma prawo odstąpić od umowy zawartej na odległość <strong>w całości lub w części (w odniesieniu do wybranych towarów)</strong> bez podania jakiejkolwiek przyczyny w terminie <strong>14 dni</strong> od dnia, w którym wszedł w posiadanie towaru lub w którym osoba trzecia inna niż przewoźnik i wskazana przez konsumenta weszła w posiadanie towaru.</p>

        <h3 style="color:#d4af37;margin-top:18px;">6.2 przycisk elektroniczny – odstąpienie jednym kliknięciem</h3>
        <p>zgodnie z dyrektywą ue 2019/2161 (omnibus) oraz obowiązującymi wytycznymi, sklep udostępnia <strong>elektroniczny przycisk „odstąp od umowy"</strong> umożliwiający złożenie oświadczenia o odstąpieniu od umowy bez konieczności drukowania ani ręcznego pisania formularzy.</p>

        <h3 style="color:#d4af37;margin-top:18px;">6.4 obowiązki konsumenta po odstąpieniu</h3>
        <p>konsument zobowiązany jest zwrócić towar sprzedawcy niezwłocznie, nie później niż w terminie <strong>14 dni</strong> od dnia, w którym odstąpił od umowy. towar należy odesłać na adres:</p>
        <p style="padding-left:20px;border-left:3px solid #f79c4b;"><strong style="color:#d4af37;">damian pańczak ph orientica</strong><br>
        ul. podgórze 15/4, 38-500 sanok</p>

        <h3 style="color:#d4af37;margin-top:18px;">6.5 zwrot środków</h3>
        <p>sprzedawca zwróci wszystkie otrzymane od konsumenta płatności niezwłocznie, nie później niż w terminie <strong>14 dni</strong> od dnia otrzymania oświadczenia o odstąpieniu od umowy.</p>
        <p style="color:#f79c4b; font-weight: bold; border-top: 1px solid #5e4b3c; padding-top: 10px;">
          <i class="material-icons" style="vertical-align: middle;">info</i>
          {l s='Gwarantujemy zwrot środków w ciągu 14 dni od otrzymania oświadczenia o odstąpieniu.' mod='withdrawalform'}
        </p>
      </div>

      <div class="withdrawal-action-form" style="border: 2px solid #f79c4b; padding: 20px; border-radius: 8px;">
          <h2 style="color:#f79c4b;">{l s='Formularz odstąpienia od umowy' mod='withdrawalform'}</h2>

          {if $is_expired && $mode == 'soft'}
            <div class="alert alert-warning">
              {l s='Uwaga: Standardowy 14-dniowy termin na odstąpienie od umowy upłynął. Możesz nadal złożyć formularz, ale może on wymagać dodatkowej weryfikacji.' mod='withdrawalform'}
            </div>
          {/if}

          <form action="{$smarty.server.REQUEST_URI}" method="post">
            <p style="font-size: 1.2rem;">{l s='Zamówienie nr:' mod='withdrawalform'} <strong>{$order->reference}</strong></p>
            <p>{l s='Data zamówienia:' mod='withdrawalform'} <strong>{$order->date_add}</strong></p>

            <h3 style="color:#d4af37; margin-top: 20px;">{l s='Wybierz produkty do zwrotu:' mod='withdrawalform'}</h3>
            <div class="product-selection" style="margin-bottom: 20px;">
                {foreach from=$products item=product}
                    <div class="product-item" style="display: flex; align-items: center; padding: 10px; border-bottom: 1px solid #5e4b3c;">
                        <div style="margin-right: 15px;">
                            <input type="checkbox" name="product_ids[]" value="{$product.id_order_detail}" id="prod_{$product.id_order_detail}" style="transform: scale(1.5);">
                        </div>
                        <label for="prod_{$product.id_order_detail}" style="margin: 0; flex-grow: 1;">
                            <strong>{$product.product_name}</strong><br>
                            <span style="opacity: 0.8;">{l s='Dostępna ilość:' mod='withdrawalform'} {$product.product_quantity}</span>
                        </label>
                        <div style="width: 100px;">
                            <input type="number" name="selected_products[{$product.id_order_detail}]" value="{$product.product_quantity}" min="1" max="{$product.product_quantity}" class="form-control" style="background: #3d3d3d; color: #fff; border: 1px solid #5e4b3c;">
                        </div>
                    </div>
                {/foreach}
            </div>

            <div class="form-group">
              <label class="form-control-label" style="color: #f79c4b;">{l s='Powód (opcjonalnie)' mod='withdrawalform'}</label>
              <input type="text" name="reason" class="form-control" style="background: #3d3d3d; color: #fff; border: 1px solid #5e4b3c;">
            </div>

            <div class="form-group">
              <label class="form-control-label" style="color: #f79c4b;">{l s='Wiadomość / Dodatkowe informacje' mod='withdrawalform'}</label>
              <textarea name="message" class="form-control" rows="5" style="background: #3d3d3d; color: #fff; border: 1px solid #5e4b3c;"></textarea>
            </div>

            <footer class="form-footer" style="margin-top: 20px; text-align: right;">
              <input type="hidden" name="token" value="{$token}">
              <button class="btn btn-primary" type="submit" name="submitWithdrawal" style="background-color: #f79c4b; border-color: #f79c4b; color: #000; font-weight: bold; padding: 10px 30px;">
                {l s='Zatwierdź odstąpienie' mod='withdrawalform'}
              </button>
            </footer>
          </form>
      </div>
    {/if}
  </section>
{/block}
