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
        $this->export = true;

        $this->ensureColumnsExist();

        $this->_select = 'o.reference as order_reference, c.email as customer_email';
        $this->_join = '
            LEFT JOIN `' . _DB_PREFIX_ . 'orders` o ON (a.`id_order` = o.`id_order`)
            LEFT JOIN `' . _DB_PREFIX_ . 'customer` c ON (a.`id_customer` = c.`id_customer`)';

        parent::__construct();

        $this->fields_list = array(
            'id_withdrawal_request' => array(
                'title' => $this->l('ID'),
                'align' => 'center',
                'width' => 25,
                'filter_key' => 'a!id_withdrawal_request'
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
                'width' => 200,
                'filter_key' => 'a!reason'
            ),
            'date_add' => array(
                'title' => $this->l('Submission date'),
                'type' => 'datetime',
                'width' => 150,
                'filter_key' => 'a!date_add'
            ),
            'ip_address' => array(
                'title' => $this->l('IP address'),
                'width' => 100,
                'filter_key' => 'a!ip_address'
            ),
            'id_order_return' => array(
                'title' => $this->l('Native Return ID'),
                'width' => 50,
                'callback' => 'displayNativeReturnLink',
                'filter_key' => 'a!id_order_return'
            ),
            'status' => array(
                'title' => $this->l('Status'),
                'width' => 100,
                'type' => 'select',
                'list' => array(
                    'pending' => $this->l('Pending'),
                    'accepted' => $this->l('Accepted'),
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
    }

    protected function ensureColumnsExist()
    {
        try {
            $columns = Db::getInstance()->executeS('SHOW COLUMNS FROM `' . _DB_PREFIX_ . 'withdrawal_request`');
            if (!$columns) {
                return;
            }
            $existing = array_column($columns, 'Field');

            if (!in_array('selected_products', $existing)) {
                Db::getInstance()->execute('ALTER TABLE `' . _DB_PREFIX_ . 'withdrawal_request` ADD COLUMN `selected_products` text DEFAULT NULL');
            }
            if (!in_array('id_order_return', $existing)) {
                Db::getInstance()->execute('ALTER TABLE `' . _DB_PREFIX_ . 'withdrawal_request` ADD COLUMN `id_order_return` int(11) DEFAULT NULL');
            }
            if (!in_array('status', $existing)) {
                Db::getInstance()->execute('ALTER TABLE `' . _DB_PREFIX_ . 'withdrawal_request` ADD COLUMN `status` varchar(20) NOT NULL DEFAULT \'pending\'');
            }
        } catch (Exception $e) {
            // Silently fail to avoid 500 error if table doesn't exist yet
        }
    }

    public function displayPSStatus($state_name, $row)
    {
        return '<span class="label" style="background-color:' . $row['state_color'] . ';color:white;">' . $state_name . '</span>';
    }

    public function displayAssociatedRequest($id_withdrawal_request)
    {
        if ($id_withdrawal_request) {
            return '<span class="label label-success">#' . (int)$id_withdrawal_request . '</span>';
        }
        return '<span class="label label-warning">' . $this->l('Brak') . '</span>';
    }

    public function displayNativeReturnLink($id_order_return)
    {
        if (!$id_order_return) {
            return '';
        }

        // Return plain ID for export
        if (Tools::isSubmit('submitExport' . $this->table) || Tools::isSubmit('export' . $this->table)) {
            return (int)$id_order_return;
        }

        $link = $this->context->link->getAdminLink('AdminReturn', true, [], [
            'id_order_return' => (int)$id_order_return,
            'vieworder_return' => 1
        ]);
        return '<a href="' . $link . '" class="btn btn-default" target="_blank"><i class="icon-external-link"></i> #' . (int) $id_order_return . '</a>';
    }

    public function initContent()
    {
        if (Tools::isSubmit('export' . $this->table) || Tools::isSubmit('submitExport' . $this->table)) {
            $this->processExport();
            die();
        }
        parent::initContent();
    }

    public function renderList()
    {
        $db = Db::getInstance();
        $p  = '`' . _DB_PREFIX_ . 'withdrawal_request`';

        $stats = [
            'total'    => (int)$db->getValue("SELECT COUNT(*) FROM {$p}"),
            'pending'  => (int)$db->getValue("SELECT COUNT(*) FROM {$p} WHERE status='pending'"),
            'accepted' => (int)$db->getValue("SELECT COUNT(*) FROM {$p} WHERE status='accepted'"),
            'rejected' => (int)$db->getValue("SELECT COUNT(*) FROM {$p} WHERE status='rejected'"),
            'today'    => (int)$db->getValue("SELECT COUNT(*) FROM {$p} WHERE DATE(date_add)=CURDATE()"),
            'native'   => (int)$db->getValue('SELECT COUNT(*) FROM `' . _DB_PREFIX_ . 'order_return`'),
        ];

        $statsHtml = '<div class="panel" style="margin-bottom:15px;">
            <div class="panel-heading"><i class="icon-bar-chart"></i> ' . $this->l('Statystyki wniosków o odstąpienie') . '</div>
            <div class="row" style="padding:10px 15px;">
                <div class="col-lg-2 col-xs-6"><div class="alert alert-info text-center" style="background-color:#e1f5fe; border:1px solid #b3e5fc; color:#01579b;"><h3 style="margin:5px 0;">' . $stats['total'] . '</h3><p style="margin:0; font-weight:bold;">' . $this->l('Łącznie') . '</p></div></div>
                <div class="col-lg-2 col-xs-6"><div class="alert alert-warning text-center" style="background-color:#fffde7; border:1px solid #fff9c4; color:#f57f17;"><h3 style="margin:5px 0;">' . $stats['pending'] . '</h3><p style="margin:0; font-weight:bold;">⏳ ' . $this->l('Oczekujące') . '</p></div></div>
                <div class="col-lg-2 col-xs-6"><div class="alert alert-success text-center" style="background-color:#e8f5e9; border:1px solid #c8e6c9; color:#1b5e20;"><h3 style="margin:5px 0;">' . $stats['accepted'] . '</h3><p style="margin:0; font-weight:bold;">✅ ' . $this->l('Przyjęte') . '</p></div></div>
                <div class="col-lg-2 col-xs-6"><div class="alert alert-danger text-center" style="background-color:#ffebee; border:1px solid #ffcdd2; color:#b71c1c;"><h3 style="margin:5px 0;">' . $stats['rejected'] . '</h3><p style="margin:0; font-weight:bold;">❌ ' . $this->l('Odrzucone') . '</p></div></div>
                <div class="col-lg-2 col-xs-6"><div class="alert alert-info text-center" style="background-color:#f3e5f5; border:1px solid #e1bee7; color:#4a148c;"><h3 style="margin:5px 0;">' . $stats['today'] . '</h3><p style="margin:0; font-weight:bold;">' . $this->l('Dzisiaj') . '</p></div></div>
                <div class="col-lg-2 col-xs-6"><div class="alert text-center" style="background:#7b68ee; color:#fff; border:1px solid #5e4bcf;"><h3 style="margin:5px 0; color:#fff;">' . $stats['native'] . '</h3><p style="margin:0; font-weight:bold;">' . $this->l('Zwroty PS') . '</p></div></div>
            </div>
        </div>';

        $listHtml = parent::renderList();

        // === NATYWNE ZWROTY PS ===
        $returns = Db::getInstance()->executeS('
            SELECT r.id_order_return, r.id_order, r.question, r.date_add,
                   o.reference AS order_reference,
                   c.firstname, c.lastname, c.email AS customer_email,
                   rs.name AS state_name, ors.color AS state_color,
                   wr.id_withdrawal_request
            FROM `' . _DB_PREFIX_ . 'order_return` r
            LEFT JOIN `' . _DB_PREFIX_ . 'orders` o
                ON o.id_order = r.id_order
            LEFT JOIN `' . _DB_PREFIX_ . 'customer` c
                ON c.id_customer = r.id_customer
            LEFT JOIN `' . _DB_PREFIX_ . 'order_return_state` ors
                ON ors.id_order_return_state = r.state
            LEFT JOIN `' . _DB_PREFIX_ . 'order_return_state_lang` rs
                ON rs.id_order_return_state = r.state
                AND rs.id_lang = ' . (int)$this->context->language->id . '
            LEFT JOIN `' . _DB_PREFIX_ . 'withdrawal_request` wr
                ON wr.id_order_return = r.id_order_return
            ORDER BY r.date_add DESC
            LIMIT 50
        ') ?: [];

        $nativeReturnsHtml = '<div class="panel col-lg-12" id="native_returns_panel">
            <style>
                @media (max-width: 991px) {
                    #native_returns_panel .table-responsive table,
                    #native_returns_panel .table-responsive thead,
                    #native_returns_panel .table-responsive tbody,
                    #native_returns_panel .table-responsive th,
                    #native_returns_panel .table-responsive td,
                    #native_returns_panel .table-responsive tr { display: block; }
                    #native_returns_panel .table-responsive thead tr { position: absolute; top: -9999px; left: -9999px; }
                    #native_returns_panel .table-responsive tr { border: 1px solid #ccc; margin-bottom: 10px; }
                    #native_returns_panel .table-responsive td { border: none; border-bottom: 1px solid #eee; position: relative; padding-left: 50% !important; text-align: right; }
                    #native_returns_panel .table-responsive td:before { position: absolute; left: 10px; width: 45%; padding-right: 10px; white-space: nowrap; text-align: left; font-weight: bold; }
                    #native_returns_panel .table-responsive td:nth-child(1):before { content: "ID"; }
                    #native_returns_panel .table-responsive td:nth-child(2):before { content: "' . $this->l('Data') . '"; }
                    #native_returns_panel .table-responsive td:nth-child(3):before { content: "' . $this->l('Zamówienie') . '"; }
                    #native_returns_panel .table-responsive td:nth-child(4):before { content: "' . $this->l('Klient') . '"; }
                    #native_returns_panel .table-responsive td:nth-child(5):before { content: "' . $this->l('Status PS') . '"; }
                    #native_returns_panel .table-responsive td:nth-child(6):before { content: "' . $this->l('Powiązany wniosek') . '"; }
                    #native_returns_panel .table-responsive td:nth-child(7):before { content: "' . $this->l('Akcje') . '"; }
                }
            </style>
            <div class="panel-heading">
                <i class="icon-exchange"></i> ' . $this->l('Zwroty produktów — natywny system PrestaShop (Recent 50)') . '
            </div>
            <div class="table-responsive">
                <table class="table">
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>' . $this->l('Data') . '</th>
                            <th>' . $this->l('Zamówienie') . '</th>
                            <th>' . $this->l('Klient') . '</th>
                            <th>' . $this->l('Status PS') . '</th>
                            <th>' . $this->l('Powiązany wniosek') . '</th>
                            <th class="text-right">' . $this->l('Akcje') . '</th>
                        </tr>
                    </thead>
                    <tbody>';

        if (empty($returns)) {
            $nativeReturnsHtml .= '<tr><td colspan="7" class="text-center">' . $this->l('Brak zwrotów w systemie PrestaShop.') . '</td></tr>';
        } else {
            foreach ($returns as $r) {
                $link = $this->context->link->getAdminLink('AdminReturn', true, [], [
                    'id_order_return' => (int)$r['id_order_return'],
                    'vieworder_return' => 1
                ]);
                $assoc = $r['id_withdrawal_request'] ? '<span class="label label-success">#' . (int)$r['id_withdrawal_request'] . '</span>' : '<span class="label label-warning">' . $this->l('Brak') . '</span>';

                $nativeReturnsHtml .= '<tr>
                    <td>' . (int)$r['id_order_return'] . '</td>
                    <td>' . $r['date_add'] . '</td>
                    <td>' . $r['order_reference'] . '</td>
                    <td>' . $r['firstname'] . ' ' . $r['lastname'] . ' (' . $r['customer_email'] . ')</td>
                    <td><span class="label" style="background-color:' . $r['state_color'] . '; color:white;">' . $r['state_name'] . '</span></td>
                    <td>' . $assoc . '</td>
                    <td class="text-right">
                        <a href="' . $link . '" class="btn btn-default" title="' . $this->l('Otwórz') . '">
                            <i class="icon-search-plus"></i> ' . $this->l('Otwórz') . '
                        </a>
                    </td>
                </tr>';
            }
        }
        $nativeReturnsHtml .= '</tbody></table></div></div>';

        return $statsHtml . $listHtml . $nativeReturnsHtml;
    }

    public function postProcess()
    {
        if (Tools::isSubmit('updatestatus')) {
            $id  = (int)Tools::getValue('id_withdrawal_request');
            $new = Tools::getValue('new_status');
            if ($id && in_array($new, ['pending', 'accepted', 'rejected'])) {
                Db::getInstance()->execute(
                    'UPDATE `' . _DB_PREFIX_ . 'withdrawal_request`
                     SET status = \'' . pSQL($new) . '\'
                     WHERE id_withdrawal_request = ' . $id
                );
                $this->confirmations[] = $this->l('Status zaktualizowany.');
            }
        }
        return parent::postProcess();
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
            'products' => $products_data,
            'current_status' => $res['status'],
            'statuses' => [
                'pending' => $this->l('Pending'),
                'accepted' => $this->l('Accepted'),
                'rejected' => $this->l('Rejected')
            ],
            'form_action' => self::$currentIndex . '&id_withdrawal_request=' . $id . '&viewwithdrawal_request&token=' . $this->token
        ));

        return parent::renderView();
    }
}
