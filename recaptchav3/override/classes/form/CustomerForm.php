<?php
/**
 * 2007-2024 PrestaShop
 *
 * NOTICE OF LICENSE
 *
 * This source file is subject to the Academic Free License (AFL 3.0)
 * that is bundled with this package in the file LICENSE.txt.
 * It is also available through the world-wide-web at this URL:
 * http://opensource.org/licenses/afl-3.0.php
 * If you did not receive a copy of the license and are unable to
 * obtain it through the world-wide-web, please send an email
 * to license@prestashop.com so we can send you a copy immediately.
 *
 * @author    Orientica.pl
 * @copyright 2007-2024 Orientica.pl
 * @license   http://opensource.org/licenses/afl-3.0.php  Academic Free License (AFL 3.0)
 */

class CustomerForm extends CustomerFormCore
{
    public function submit()
    {
        if (Module::isEnabled('recaptchav3') && Configuration::get('RECAPTCHAV3_ENABLE_REGISTER')) {
            $recaptcha = Module::getInstanceByName('recaptchav3');
            if (!$recaptcha->validateToken('register')) {
                $this->errors[''][] = $recaptcha->l('reCAPTCHA verification failed. Please try again.');
                return false;
            }
        }

        return parent::submit();
    }
}
