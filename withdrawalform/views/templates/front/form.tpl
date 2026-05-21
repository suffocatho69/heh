{extends file='customer/page.tpl'}

{block name='page_title'}
  {l s='Withdraw from contract' mod='withdrawalform'}
{/block}

{block name='page_content'}
  <section id="withdrawal-form">
    {if isset($success) && $success}
      <div class="alert alert-success">
        {l s='Your withdrawal request has been successfully submitted.' mod='withdrawalform'}
      </div>
      <a href="{$link->getPageLink('history')}" class="btn btn-primary">{l s='Back to order history' mod='withdrawalform'}</a>
    {else}
      {if $is_expired && $mode == 'soft'}
        <div class="alert alert-warning">
          {l s='Note: The standard 14-day withdrawal period has expired. You can still submit the form, but it may be subject to additional verification.' mod='withdrawalform'}
        </div>
      {/if}

      <form action="{$smarty.server.REQUEST_URI}" method="post">
        <p>{l s='Order reference:' mod='withdrawalform'} <strong>{$order->reference}</strong></p>

        <div class="form-group">
          <label class="form-control-label">{l s='Reason (optional)' mod='withdrawalform'}</label>
          <input type="text" name="reason" class="form-control">
        </div>

        <div class="form-group">
          <label class="form-control-label">{l s='Message / Details' mod='withdrawalform'}</label>
          <textarea name="message" class="form-control" rows="5"></textarea>
        </div>

        <footer class="form-footer">
          <input type="hidden" name="token" value="{$token}">
          <button class="btn btn-primary" type="submit" name="submitWithdrawal">
            {l s='Submit withdrawal' mod='withdrawalform'}
          </button>
        </footer>
      </form>
    {/if}
  </section>
{/block}
