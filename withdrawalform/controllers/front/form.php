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
 * DISCLAIMER
 *
 * Do not edit or add to this file if you wish to upgrade PrestaShop to newer
 * versions in the future. If you wish to customize PrestaShop for your
 * needs please refer to http://www.prestashop.com for more information.
 *
 *  @author    PrestaShop SA <contact@prestashop.com>
 *  @copyright 2007-2024 PrestaShop SA
 *  @license   http://opensource.org/licenses/afl-3.0.php  Academic Free License (AFL 3.0)
 *  International Registered Trademark & Property of PrestaShop SA
 */

class WithdrawalFormFormModuleFrontController extends ModuleFrontController
{
    public $auth = true;
    public $guestAllowed = false;

    public function initContent()
    {
        parent::initContent();

        $id_order = (int) Tools::getValue('id_order');
        $order = new Order($id_order);

        if (!Validate::isLoadedObject($order) || $order->id_customer != $this->context->customer->id) {
            Tools::redirect('index.php?controller=history');
        }

        $days_limit = (int) Configuration::get('WITHDRAWAL_FORM_DAYS_LIMIT');
        $mode = Configuration::get('WITHDRAWAL_FORM_MODE');
        $one_per_order = (int) Configuration::get('WITHDRAWAL_FORM_ONE_PER_ORDER');

        $order_date = new DateTime($order->date_add);
        $now = new DateTime();
        $diff = $now->diff($order_date)->days;

        $is_expired = ($diff > $days_limit);

        if ($mode === 'hard' && $is_expired) {
            $this->errors[] = $this->module->l('The withdrawal period for this order has expired.');
        }

        if ($one_per_order) {
            $already_submitted = (bool) Db::getInstance()->getValue('
                SELECT id_withdrawal_request
                FROM ' . _DB_PREFIX_ . 'withdrawal_request
                WHERE id_order = ' . (int) $order->id
            );
            if ($already_submitted) {
                $this->errors[] = $this->module->l('A withdrawal request has already been submitted for this order.');
            }
        }

        if (Tools::isSubmit('submitWithdrawal')) {
            if (!Tools::getValue('token') || Tools::getValue('token') != Tools::getToken(false)) {
                $this->errors[] = $this->module->l('Invalid security token.');
            }

            $reason = Tools::getValue('reason');
            $message = Tools::getValue('message');

            if (empty($this->errors)) {
                $withdrawal = new WithdrawalRequest();
                $withdrawal->id_order = (int) $order->id;
                $withdrawal->id_customer = (int) $this->context->customer->id;
                $withdrawal->reason = $reason;
                $withdrawal->message = $message;
                $withdrawal->ip_address = Tools::getRemoteAddr();
                $withdrawal->date_add = date('Y-m-d H:i:s');
                $res = $withdrawal->add();

                if ($res) {
                    $this->sendEmails($order, $reason, $message);
                    $this->context->smarty->assign('success', true);
                } else {
                    $this->errors[] = $this->module->l('An error occurred while saving your request.');
                }
            }
        }

        $this->context->smarty->assign(array(
            'order' => $order,
            'is_expired' => $is_expired,
            'mode' => $mode,
            'days_limit' => $days_limit,
            'token' => Tools::getToken(false),
        ));

        $this->setTemplate('module:withdrawalform/views/templates/front/form.tpl');
    }

    protected function sendEmails($order, $reason, $message)
    {
        $customer = $this->context->customer;
        $template_vars = array(
            '{order_reference}' => $order->reference,
            '{firstname}' => $customer->firstname,
            '{lastname}' => $customer->lastname,
            '{reason}' => $reason,
            '{message}' => $message,
            '{date}' => date('Y-m-d H:i:s'),
        );

        // To customer
        Mail::Send(
            $this->context->language->id,
            'withdrawal_conf',
            $this->module->l('Withdrawal request confirmation'),
            $template_vars,
            $customer->email,
            $customer->firstname . ' ' . $customer->lastname,
            null,
            null,
            null,
            null,
            $this->module->getLocalPath() . 'mails/'
        );

        // To admin
        Mail::Send(
            $this->context->language->id,
            'withdrawal_admin',
            $this->module->l('New withdrawal request'),
            $template_vars,
            Configuration::get('PS_SHOP_EMAIL'),
            null,
            null,
            null,
            null,
            null,
            $this->module->getLocalPath() . 'mails/'
        );
    }
}
