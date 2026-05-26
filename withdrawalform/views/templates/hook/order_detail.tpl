<div class="withdrawal-button-container" style="margin-top: 20px; padding: 15px; border: 1px solid #ddd; background: #f9f9f9;">
    <h4>{l s='Withdraw from contract' mod='withdrawalform'}</h4>
    <p>{l s='According to consumer law, you have the right to withdraw from the contract.' mod='withdrawalform'}</p>

    {if $is_expired && $mode == 'soft'}
        <p class="text-warning"><strong>{l s='Warning:' mod='withdrawalform'}</strong> {l s='The standard %d-day period has passed.' sprintf=[$days_limit] mod='withdrawalform'}</p>
    {/if}

    <a href="{$withdrawal_url}" class="btn btn-primary">
        {l s='Withdraw from contract here' mod='withdrawalform'}
    </a>
</div>
