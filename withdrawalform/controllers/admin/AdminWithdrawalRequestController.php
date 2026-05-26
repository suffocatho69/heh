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

require_once _PS_MODULE_DIR_ . 'withdrawalform/classes/WithdrawalRequest.php';

class AdminWithdrawalRequestController extends ModuleAdminController
{
    public function __construct()
    {
        $this->table = 'withdrawal_request';
        $this->className = 'WithdrawalRequest';
        $this->identifier = 'id_withdrawal_request';
        $this->bootstrap = true;
        $this->lang = false;
        $this->explicitSelect = true;

        $this->_select = 'o.reference as order_reference, c.email as customer_email';
        $this->_join = '
            LEFT JOIN `' . _DB_PREFIX_ . 'orders` o ON (a.`id_order` = o.`id_order`)
            LEFT JOIN `' . _DB_PREFIX_ . 'customer` c ON (a.`id_customer` = c.`id_customer`)';

        parent::__construct();

        $this->fields_list = array(
            'id_withdrawal_request' => array(
                'title' => $this->l('ID'),
                'align' => 'center',
                'width' => 25
            ),
            'id_order' => array(
                'title' => $this->l('Order ID'),
                'width' => 25,
                'filter_key' => 'a!id_order'
            ),
            'order_reference' => array(
                'title' => $this->l('Order reference'),
                'width' => 100,
                'filter_key' => 'o!reference'
            ),
            'customer_email' => array(
                'title' => $this->l('Customer email'),
                'width' => 150,
                'filter_key' => 'c!email'
            ),
            'reason' => array(
                'title' => $this->l('Reason'),
                'width' => 200
            ),
            'date_add' => array(
                'title' => $this->l('Submission date'),
                'type' => 'datetime',
                'width' => 150,
                'filter_key' => 'a!date_add'
            ),
            'ip_address' => array(
                'title' => $this->l('IP address'),
                'width' => 100
            ),
            'id_order_return' => array(
                'title' => $this->l('Native Return ID'),
                'width' => 50,
                'callback' => 'displayNativeReturnLink'
            ),
            'status' => array(
                'title' => $this->l('Status'),
                'width' => 100,
                'type' => 'select',
                'list' => array(
                    'pending' => $this->l('Pending'),
                    'completed' => $this->l('Completed'),
                    'rejected' => $this->l('Rejected'),
                ),
                'filter_key' => 'a!status',
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
            'desc' => $this->l('Export to CSV')
        );
    }

    public function displayNativeReturnLink($id_order_return)
    {
        if (!$id_order_return) {
            return '---';
        }

        $link = $this->context->link->getAdminLink('AdminReturn') . '&id_order_return=' . (int) $id_order_return . '&vieworder_return';
        return '<a href="' . $link . '" class="btn btn-default" target="_blank"><i class="icon-external-link"></i> #' . (int) $id_order_return . '</a>';
    }

    public function renderList()
    {
        $listHtml = parent::renderList();

        // === NATYWNE ZWROTY PS ===
        $returns = Db::getInstance()->executeS('
            SELECT r.id_order_return, r.id_order, r.question, r.date_add,
                   o.reference AS order_reference,
                   c.firstname, c.lastname, c.email AS customer_email,
                   rs.name AS state_name, rs.color AS state_color,
                   wr.id_withdrawal_request
            FROM `' . _DB_PREFIX_ . 'order_return` r
            LEFT JOIN `' . _DB_PREFIX_ . 'orders` o
                ON o.id_order = r.id_order
            LEFT JOIN `' . _DB_PREFIX_ . 'customer` c
                ON c.id_customer = r.id_customer
            LEFT JOIN `' . _DB_PREFIX_ . 'order_return_state_lang` rs
                ON rs.id_order_return_state = r.state
                AND rs.id_lang = ' . (int)$this->context->language->id . '
            LEFT JOIN `' . _DB_PREFIX_ . 'withdrawal_request` wr
                ON wr.id_order_return = r.id_order_return
            ORDER BY r.date_add DESC
            LIMIT 50
        ') ?: [];

        $nativeReturnsHtml = '<div class="panel col-lg-12">
            <div class="panel-heading">
                <i class="icon-exchange"></i> ' . $this->l('Native PrestaShop Returns (Recent 50)') . '
            </div>
            <div class="table-responsive-row clearfix">
                <table class="table order_return">
                    <thead>
                        <tr class="nodrag nodrop">
                            <th><span class="title_box">' . $this->l('ID') . '</span></th>
                            <th><span class="title_box">' . $this->l('Date') . '</span></th>
                            <th><span class="title_box">' . $this->l('Order') . '</span></th>
                            <th><span class="title_box">' . $this->l('Customer') . '</span></th>
                            <th><span class="title_box">' . $this->l('PS Status') . '</span></th>
                            <th><span class="title_box">' . $this->l('Associated Request') . '</span></th>
                            <th></th>
                        </tr>
                    </thead>
                    <tbody>';

        if (empty($returns)) {
            $nativeReturnsHtml .= '<tr><td class="list-empty" colspan="7"><div class="list-empty-msg"><i class="icon-warning-sign list-empty-icon"></i>' . $this->l('No native returns found.') . '</div></td></tr>';
        } else {
            foreach ($returns as $r) {
                $link = $this->context->link->getAdminLink('AdminReturn') . '&id_order_return=' . (int) $r['id_order_return'] . '&vieworder_return';
                $assoc = $r['id_withdrawal_request'] ? '<span class="label label-success">#' . (int)$r['id_withdrawal_request'] . '</span>' : '<span class="label label-warning">' . $this->l('None') . '</span>';

                $nativeReturnsHtml .= '<tr>
                    <td>' . (int)$r['id_order_return'] . '</td>
                    <td>' . $r['date_add'] . '</td>
                    <td>' . $r['order_reference'] . '</td>
                    <td>' . $r['firstname'] . ' ' . $r['lastname'] . ' (' . $r['customer_email'] . ')</td>
                    <td><span class="label" style="background-color:' . $r['state_color'] . ';color:white;">' . $r['state_name'] . '</span></td>
                    <td>' . $assoc . '</td>
                    <td class="text-right">
                        <a href="' . $link . '" class="btn btn-default"><i class="icon-search-plus"></i> ' . $this->l('View') . '</a>
                    </td>
                </tr>';
            }
        }

        $nativeReturnsHtml .= '</tbody></table></div></div>';

        return $listHtml . $nativeReturnsHtml;
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
            if (Validate::isLoadedObject($order)) {
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
        }

        $this->context->smarty->assign(array(
            'withdrawal' => $res,
            'products' => $products_data
        ));

        return parent::renderView();
    }
}
