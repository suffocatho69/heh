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

class AdminWithdrawalRequestController extends ModuleAdminController
{
    public function __construct()
    {
        $this->table = 'withdrawal_request';
        $this->className = 'WithdrawalRequest'; // We don't have a model class, but HelperList needs it or we can bypass
        $this->identifier = 'id_withdrawal_request';
        $this->bootstrap = true;
        $this->lang = false;
        $this->explicitSelect = true;

        parent::__construct();

        $this->_select = 'o.reference as order_reference, c.email as customer_email';
        $this->_join = '
            LEFT JOIN `' . _DB_PREFIX_ . 'orders` o ON (a.`id_order` = o.`id_order`)
            LEFT JOIN `' . _DB_PREFIX_ . 'customer` c ON (a.`id_customer` = c.`id_customer`)';

        $this->fields_list = array(
            'id_withdrawal_request' => array(
                'title' => $this->l('ID'),
                'align' => 'center',
                'width' => 25
            ),
            'id_order' => array(
                'title' => $this->l('Order ID'),
                'width' => 25
            ),
            'order_reference' => array(
                'title' => $this->l('Order Reference'),
                'width' => 100
            ),
            'customer_email' => array(
                'title' => $this->l('Customer Email'),
                'width' => 150
            ),
            'reason' => array(
                'title' => $this->l('Reason'),
                'width' => 200
            ),
            'date_add' => array(
                'title' => $this->l('Date'),
                'type' => 'datetime',
                'width' => 150
            ),
            'ip_address' => array(
                'title' => $this->l('IP Address'),
                'width' => 100
            ),
        );

        $this->actions = array('view', 'delete');
        $this->bulk_actions = array(
            'delete' => array(
                'text' => $this->l('Delete selected'),
                'confirm' => $this->l('Delete selected items?'),
                'icon' => 'icon-trash'
            )
        );
    }

    public function initToolbar()
    {
        parent::initToolbar();
        unset($this->toolbar_btn['new']);

        $this->toolbar_btn['export'] = array(
            'href' => self::$currentIndex . '&export' . $this->table . '&token=' . $this->token,
            'desc' => $this->l('Export CSV')
        );
    }

    public function renderView()
    {
        $id = (int) Tools::getValue('id_withdrawal_request');
        $res = Db::getInstance()->getRow('
            SELECT a.*, o.reference as order_reference, c.email as customer_email, c.firstname, c.lastname
            FROM `' . _DB_PREFIX_ . 'withdrawal_request` a
            LEFT JOIN `' . _DB_PREFIX_ . 'orders` o ON (a.`id_order` = o.`id_order`)
            LEFT JOIN `' . _DB_PREFIX_ . 'customer` c ON (a.`id_customer` = c.`id_customer`)
            WHERE a.id_withdrawal_request = ' . $id
        );

        $products_data = [];
        if ($res && $res['selected_products']) {
            $selected_products = json_decode($res['selected_products'], true);
            $order = new Order((int) $res['id_order']);
            $order_products = $order->getProducts();
            foreach ($order_products as $op) {
                if (isset($selected_products[$op['id_order_detail']])) {
                    $products_data[] = [
                        'name' => $op['product_name'],
                        'reference' => $op['product_reference'],
                        'quantity' => $selected_products[$op['id_order_detail']]
                    ];
                }
            }
        }

        $this->context->smarty->assign(array(
            'withdrawal' => $res,
            'products' => $products_data
        ));

        return parent::renderView();
    }
}
