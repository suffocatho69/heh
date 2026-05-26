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
    public $auth = false;
    public $guestAllowed = true;

    public function initContent()
    {
        parent::initContent();

        $id_order = (int) Tools::getValue('id_order');
        $order = new Order($id_order);
        $secure_key = Tools::getValue('secure_key');

        $is_owner = ($this->context->customer->isLogged() && $order->id_customer == $this->context->customer->id);
        $is_guest_allowed = ($secure_key && $order->secure_key == $secure_key);

        if (!Validate::isLoadedObject($order) || (!$is_owner && !$is_guest_allowed)) {
            Tools::redirect('index.php?controller=history');
        }

        $days_limit = (int) Configuration::get('WITHDRAWAL_FORM_DAYS_LIMIT');
        $mode = Configuration::get('WITHDRAWAL_FORM_MODE');
        $one_per_order = (int) Configuration::get('WITHDRAWAL_FORM_ONE_PER_ORDER');

        $reference_date = $order->delivery_date && $order->delivery_date != '0000-00-00 00:00:00' ? $order->delivery_date : $order->date_add;
        $order_date = new DateTime($reference_date);
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
            $product_ids = Tools::getValue('product_ids');
            $quantities = Tools::getValue('selected_products');
            $selected_products = [];

            if (empty($product_ids) || !is_array($product_ids)) {
                $this->errors[] = $this->module->l('Please select at least one product.');
            } else {
                $order_products = $order->getProducts();
                $op_indexed = [];
                foreach ($order_products as $op) {
                    $op_indexed[$op['id_order_detail']] = $op;
                }

                foreach ($product_ids as $id_order_detail) {
                    $id_order_detail = (int) $id_order_detail;
                    if (!isset($op_indexed[$id_order_detail])) {
                        continue;
                    }

                    $requested_qty = (int) $quantities[$id_order_detail];
                    $available_qty = (int) $op_indexed[$id_order_detail]['product_quantity'];

                    if ($requested_qty <= 0 || $requested_qty > $available_qty) {
                        $this->errors[] = sprintf(
                            $this->module->l('Invalid quantity for product %s.'),
                            $op_indexed[$id_order_detail]['product_name']
                        );
                    } else {
                        $selected_products[$id_order_detail] = $requested_qty;
                    }
                }
            }

            if (empty($this->errors)) {
                $withdrawal = new WithdrawalRequest();
                $withdrawal->id_order = (int) $order->id;
                $withdrawal->id_customer = (int) $order->id_customer;
                $withdrawal->reason = $reason;
                $withdrawal->message = $message;
                $withdrawal->selected_products = json_encode($selected_products);
                $withdrawal->ip_address = Tools::getRemoteAddr();
                $withdrawal->date_add = date('Y-m-d H:i:s');

                // Create PrestaShop native OrderReturn
                if (Configuration::get('PS_ORDER_RETURN')) {
                    $order_return = new OrderReturn();
                    $order_return->id_order = (int) $order->id;
                    $order_return->id_customer = (int) $order->id_customer;
                    $order_return->question = $reason . "\n" . $message;
                    $order_return->state = 1; // Waiting for confirmation
                    if ($order_return->add()) {
                        $withdrawal->id_order_return = (int) $order_return->id;
                        foreach ($selected_products as $id_order_detail => $qty) {
                            $order_return->setReturnDetail($id_order_detail, $qty);
                        }
                    }
                }

                $res = $withdrawal->add();

                if ($res) {
                    $this->sendEmails($order, $reason, $message, $selected_products);

                    // Add message to order
                    $customer_msg = new CustomerMessage();
                    $customer_msg->id_customer_thread = $this->getOrCreateCustomerThread($order);
                    $customer_msg->id_employee = 0;
                    $customer_msg->message = $this->module->l('Withdrawal request submitted for products:') . ' ' . $this->getProductsString($order, $selected_products);
                    $customer_msg->private = 1;
                    $customer_msg->add();

                    $this->context->smarty->assign('success', true);
                } else {
                    $this->errors[] = $this->module->l('An error occurred while saving your request.');
                }
            }
        }

        $this->context->smarty->assign(array(
            'order' => $order,
            'products' => $order->getProducts(),
            'is_expired' => $is_expired,
            'mode' => $mode,
            'days_limit' => $days_limit,
            'token' => Tools::getToken(false),
        ));

        $this->setTemplate('module:withdrawalform/views/templates/front/form.tpl');
    }

    protected function getOrCreateCustomerThread($order)
    {
        $id_customer_thread = (int) Db::getInstance()->getValue('
            SELECT id_customer_thread
            FROM ' . _DB_PREFIX_ . 'customer_thread
            WHERE id_order = ' . (int) $order->id . '
            AND id_customer = ' . (int) $order->id_customer . '
            ORDER BY date_add DESC'
        );

        if (!$id_customer_thread) {
            $customer_thread = new CustomerThread();
            $customer_thread->id_order = (int) $order->id;
            $customer_thread->id_customer = (int) $order->id_customer;
            $customer_thread->id_shop = (int) $order->id_shop;
            $customer_thread->id_lang = (int) $order->id_lang;
            $customer_thread->id_contact = 0;
            $customer_thread->email = $this->context->customer->email ?: (new Customer($order->id_customer))->email;
            $customer_thread->status = 'open';
            $customer_thread->token = Tools::passwdGen(12);
            $customer_thread->add();
            $id_customer_thread = (int) $customer_thread->id;
        }

        return $id_customer_thread;
    }

    protected function getProductsString($order, $selected_products)
    {
        $names = [];
        $order_products = $order->getProducts();
        foreach ($order_products as $op) {
            if (isset($selected_products[$op['id_order_detail']])) {
                $qty = (int) $selected_products[$op['id_order_detail']];
                $names[] = $op['product_name'] . ' (x' . $qty . ')';
            }
        }
        return implode(', ', $names);
    }

    protected function sendEmails($order, $reason, $message, $selected_products = [])
    {
        $customer = new Customer((int) $order->id_customer);
        $products_list = '';
        $order_products = $order->getProducts();
        foreach ($order_products as $op) {
            if (isset($selected_products[$op['id_order_detail']])) {
                $qty = (int) $selected_products[$op['id_order_detail']];
                $products_list .= '- ' . $op['product_name'] . ' x ' . $qty . "\n";
            }
        }

        $template_vars = array(
            '{order_reference}' => $order->reference,
            '{firstname}' => $customer->firstname,
            '{lastname}' => $customer->lastname,
            '{reason}' => $reason,
            '{message}' => $message,
            '{products}' => nl2br($products_list),
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
