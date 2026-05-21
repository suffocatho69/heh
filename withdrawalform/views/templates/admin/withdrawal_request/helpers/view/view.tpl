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
    </div>
</div>
