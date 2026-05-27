<div class="panel">
    <div class="panel-heading">
        <i class="icon-info"></i> {l s='Withdrawal Request Details' mod='withdrawalform'}
    </div>
    <div class="form-horizontal">
        <div class="row">
            <label class="control-label col-lg-3">{l s='Order Reference' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.order_reference}</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Customer' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.firstname} {$withdrawal.lastname} ({$withdrawal.customer_email})</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Date' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.date_add}</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Reason' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.reason}</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='Message' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.message|nl2br}</p>
            </div>
        </div>
        <div class="row">
            <label class="control-label col-lg-3">{l s='IP Address' mod='withdrawalform'}:</label>
            <div class="col-lg-9">
                <p class="form-control-static">{$withdrawal.ip_address}</p>
            </div>
        </div>
        {if $withdrawal.id_order_return}
            <div class="row">
                <label class="control-label col-lg-3">{l s='PrestaShop Return' mod='withdrawalform'}:</label>
                <div class="col-lg-9">
                    <p class="form-control-static">
                        <a href="{$link->getAdminLink('AdminReturn')|escape:'html':'UTF-8'}&id_order_return={$withdrawal.id_order_return|intval}&vieworder_return" class="btn btn-default" target="_blank">
                            <i class="icon-external-link"></i> #{l s='View Return' mod='withdrawalform'} ({$withdrawal.id_order_return|intval})
                        </a>
                    </p>
                </div>
            </div>
        {/if}
    </div>

    <hr>
    <h3>{l s='Returned Products' mod='withdrawalform'}</h3>
    <table class="table">
        <thead>
            <tr>
                <th>{l s='Reference' mod='withdrawalform'}</th>
                <th>{l s='Product' mod='withdrawalform'}</th>
                <th>{l s='Quantity' mod='withdrawalform'}</th>
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
                <label for="new_status">{l s='Update Status' mod='withdrawalform'}:</label>
                <select name="new_status" id="new_status" class="form-control">
                    {foreach from=$statuses key=k item=v}
                        <option value="{$k}" {if $current_status == $k}selected="selected"{/if}>{$v}</option>
                    {/foreach}
                </select>
            </div>
            <button type="submit" name="updatestatus" class="btn btn-primary">
                <i class="icon-save"></i> {l s='Save' mod='withdrawalform'}
            </button>
        </form>
    </div>
</div>
