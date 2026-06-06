<div class="panel">
    <div class="panel-heading">
        <i class="icon-info"></i> {l s='Szczegóły wniosku o odstąpienie' mod='withdrawalform'}
    </div>
    <div class="form-horizontal">
        <div class="row">
            <label class="control-label col-lg-3">{l s='Zamówienie' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.order_reference}</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Klient' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.firstname} {$withdrawal.lastname} ({$withdrawal.customer_email})</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Data zgłoszenia' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.date_add}</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Powód' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.reason}</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Wiadomość' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.message|nl2br}</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Adres IP' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.ip_address}</p>
            </div>
        </div>
        {if $withdrawal.id_order_return}
            <div class="row">
                <label class="control-label col-lg-3">{l s='Zwrot PrestaShop' mod='withdrawalform'}:</label>
                <div class="col-lg-9">
                    <p class="form-control-static">
                        <a href="index.php?controller=AdminReturn&id_order_return={$withdrawal.id_order_return|intval}&updateorder_return=1&token={Tools::getAdminTokenLite('AdminReturn')}" class="btn btn-default" target="_blank">
                            <i class="icon-external-link"></i> {l s='Otwórz zwrot' mod='withdrawalform'} (#{$withdrawal.id_order_return|intval})
                        </a>
                    </p>
                </div>
            </div>
        {/if}
    </div>

    <hr>
    <h3>{l s='Zwracane produkty' mod='withdrawalform'}</h3>
    <table class="table">
        <thead>
            <tr>
                <th>{l s='Referencja' mod='withdrawalform'}</th>
                <th>{l s='Produkt' mod='withdrawalform'}</th>
                <th>{l s='Ilość' mod='withdrawalform'}</th>
            </tr>
        </thead>
        <tbody>
            {foreach from=$products item=product}
                <tr>
                    <td>{$product.reference}</td>
                    <td>{$product.name}</td>
                    <td>{$product.quantity}</td>
                </tr>
            {/foreach}
        </tbody>
    </table>

    <div class="panel-footer">
        <form action="{$form_action}" method="post" class="form-inline">
            <div class="form-group">
                <label for="new_status">{l s='Zmień status' mod='withdrawalform'}:</label>
                <select name="new_status" id="new_status" class="form-control">
                    {foreach from=$statuses key=k item=v}
                        <option value="{$k}" {if $current_status == $k}selected="selected"{/if}>{$v}</option>
                    {/foreach}
                </select>
            </div>
            <button type="submit" name="updatestatus" class="btn btn-primary">
                <i class="icon-save"></i> {l s='Zapisz' mod='withdrawalform'}
            </button>
        </form>
    </div>
</div>
