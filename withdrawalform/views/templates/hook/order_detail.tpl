<div style="margin-top:20px; padding:15px; border:1px solid #5e4b3c;
            background:rgba(0,0,0,0.15); border-radius:6px;">
    <h4 style="color:#f79c4b; margin-top:0;">
        <i class="material-icons" style="vertical-align:middle; font-size:18px;">undo</i>
        {l s='Odstąpienie od umowy' mod='withdrawalform'}
    </h4>
    <p style="color:#f1e3d2; margin-bottom:10px; font-size:0.9rem;">
        {l s='Zgodnie z prawem konsumenta masz prawo odstąpić od umowy w terminie 14 dni bez podania przyczyny.' mod='withdrawalform'}
    </p>
    {if $is_expired && $mode == 'soft'}
        <p style="color:#f0ad4e; font-size:0.85rem;">
            <i class="material-icons" style="vertical-align:middle;">warning</i>
            {l s='Uwaga: Standardowy %d-dniowy termin upłynął.' sprintf=[$days_limit] mod='withdrawalform'}
        </p>
    {/if}
    <a href="{$withdrawal_url}" class="btn btn-primary"
       style="background-color:#f79c4b; border-color:#c97a2a; color:#1a0e00; font-weight:bold;">
        <i class="material-icons" style="vertical-align:middle; font-size:16px; margin-right:5px;">assignment_return</i>
        {l s='Odstąp od umowy tutaj' mod='withdrawalform'}
    </a>
</div>
