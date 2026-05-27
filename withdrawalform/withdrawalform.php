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

if (!defined('_PS_VERSION_')) {
    exit;
}

require_once _PS_MODULE_DIR_ . 'withdrawalform/classes/WithdrawalRequest.php';

class WithdrawalForm extends Module
{
    protected $config_prefix = 'WITHDRAWAL_FORM_';

    public function __construct()
    {
        $this->name = 'withdrawalform';
        $this->tab = 'front_office_features';
        $this->version = '1.1.0';
        $this->author = 'Orientica.pl';
        $this->need_instance = 0;

        $this->bootstrap = true;

        parent::__construct();

        $this->displayName = $this->l('Withdraw from the contract here');
        $this->description = $this->l('Allows customers to easily withdraw from the contract directly from the order history, in accordance with new regulations.');

        $this->ps_versions_compliancy = array('min' => '1.7.5', 'max' => '9.9.9');
    }

    public function install()
    {
        include_once($this->local_path . 'sql/install.php');

        return parent::install() &&
            $this->registerHook('header') &&
            $this->registerHook('displayOrderDetail') &&
            $this->registerHook('actionGetExtraMailTemplateVars') &&
            $this->registerHook('actionOrderStatusPostUpdate') &&
            $this->registerHook('moduleRoutes') &&
            $this->registerHook('displayCheckoutSubtotalDetails') &&
            $this->installTab() &&
            Configuration::updateValue($this->config_prefix . 'DAYS_LIMIT', 14) &&
            Configuration::updateValue($this->config_prefix . 'MODE', 'soft') &&
            Configuration::updateValue($this->config_prefix . 'ONE_PER_ORDER', 1);
    }

    public function uninstall()
    {
        include_once($this->local_path . 'sql/uninstall.php');

        return parent::uninstall() &&
            $this->uninstallTab() &&
            Configuration::deleteByName($this->config_prefix . 'DAYS_LIMIT') &&
            Configuration::deleteByName($this->config_prefix . 'MODE') &&
            Configuration::deleteByName($this->config_prefix . 'ONE_PER_ORDER');
    }

    public function installTab()
    {
        $tab = new Tab();
        $tab->class_name = 'AdminWithdrawalRequest';
        $tab->module = $this->name;
        $tab->id_parent = (int) Tab::getIdFromClassName('AdminParentOrders');
        $tab->active = 1;

        $languages = Language::getLanguages();
        foreach ($languages as $lang) {
            $tab->name[$lang['id_lang']] = $this->l('Withdrawal requests');
        }

        return $tab->add();
    }

    public function uninstallTab()
    {
        $id_tab = (int) Tab::getIdFromClassName('AdminWithdrawalRequest');
        if ($id_tab) {
            $tab = new Tab($id_tab);
            return $tab->delete();
        }
        return true;
    }

    public function getContent()
    {
        $output = '';

        if (Tools::isSubmit('submit' . $this->name)) {
            $days_limit = (int) Tools::getValue('WITHDRAWAL_FORM_DAYS_LIMIT');
            $mode = pSQL(Tools::getValue('WITHDRAWAL_FORM_MODE'));
            $one_per_order = (int) Tools::getValue('WITHDRAWAL_FORM_ONE_PER_ORDER');

            Configuration::updateValue($this->config_prefix . 'DAYS_LIMIT', $days_limit);
            Configuration::updateValue($this->config_prefix . 'MODE', $mode);
            Configuration::updateValue($this->config_prefix . 'ONE_PER_ORDER', $one_per_order);

            $output .= $this->displayConfirmation($this->l('Settings updated.'));
        }

        return $output . $this->renderForm();
    }

    public function renderForm()
    {
        $fields_form = array(
            'form' => array(
                'legend' => array(
                    'title' => $this->l('Settings'),
                    'icon' => 'icon-cogs',
                ),
                'input' => array(
                    array(
                        'type' => 'text',
                        'label' => $this->l('Withdrawal days limit'),
                        'name' => 'WITHDRAWAL_FORM_DAYS_LIMIT',
                        'size' => 20,
                        'required' => true,
                        'desc' => $this->l('Number of days from delivery date (or order date) during which withdrawal is possible.'),
                    ),
                    array(
                        'type' => 'select',
                        'label' => $this->l('Limit mode'),
                        'name' => 'WITHDRAWAL_FORM_MODE',
                        'options' => array(
                            'query' => array(
                                array('id' => 'off', 'name' => $this->l('Disabled - no limit')),
                                array('id' => 'soft', 'name' => $this->l('Soft - warning only')),
                                array('id' => 'hard', 'name' => $this->l('Hard - form blocked after deadline')),
                            ),
                            'id' => 'id',
                            'name' => 'name',
                        ),
                    ),
                    array(
                        'type' => 'switch',
                        'label' => $this->l('Limit to one request per order'),
                        'name' => 'WITHDRAWAL_FORM_ONE_PER_ORDER',
                        'is_bool' => true,
                        'values' => array(
                            array(
                                'id' => 'active_on',
                                'value' => 1,
                                'label' => $this->l('Yes'),
                            ),
                            array(
                                'id' => 'active_off',
                                'value' => 0,
                                'label' => $this->l('No'),
                            ),
                        ),
                    ),
                ),
                'submit' => array(
                    'title' => $this->l('Save'),
                ),
            ),
        );

        $helper = new HelperForm();
        $helper->show_toolbar = false;
        $helper->table = $this->table;
        $lang = new Language((int) Configuration::get('PS_LANG_DEFAULT'));
        $helper->default_form_language = $lang->id;
        $helper->allow_employee_form_lang = Configuration::get('PS_BO_ALLOW_EMPLOYEE_FORM_LANG') ? Configuration::get('PS_BO_ALLOW_EMPLOYEE_FORM_LANG') : 0;
        $helper->identifier = $this->identifier;
        $helper->submit_action = 'submit' . $this->name;
        $helper->currentIndex = $this->context->link->getAdminLink('AdminModules', false) . '&configure=' . $this->name . '&tab_module=' . $this->tab . '&module_name=' . $this->name;
        $helper->token = Tools::getAdminTokenLite('AdminModules');
        $helper->tpl_vars = array(
            'fields_value' => $this->getConfigFieldsValues(),
            'languages' => $this->context->controller->getLanguages(),
            'id_language' => $this->context->language->id,
        );

        return $helper->generateForm(array($fields_form));
    }

    public function getConfigFieldsValues()
    {
        return array(
            'WITHDRAWAL_FORM_DAYS_LIMIT' => Configuration::get($this->config_prefix . 'DAYS_LIMIT'),
            'WITHDRAWAL_FORM_MODE' => Configuration::get($this->config_prefix . 'MODE'),
            'WITHDRAWAL_FORM_ONE_PER_ORDER' => Configuration::get($this->config_prefix . 'ONE_PER_ORDER'),
        );
    }

    public function hookHeader()
    {
        $this->context->controller->addCSS($this->_path . 'views/css/withdrawalform.css');
    }

    public function hookDisplayOrderDetail($params)
    {
        $order = $params['order'];
        if (!Validate::isLoadedObject($order)) {
            return;
        }

        $days_limit = (int) Configuration::get($this->config_prefix . 'DAYS_LIMIT');
        $mode = Configuration::get($this->config_prefix . 'MODE');
        $one_per_order = (int) Configuration::get($this->config_prefix . 'ONE_PER_ORDER');

        $reference_date = $order->delivery_date && $order->delivery_date != '0000-00-00 00:00:00' ? $order->delivery_date : $order->date_add;
        $order_date = new DateTime($reference_date);
        $now = new DateTime();
        $diff = $now->diff($order_date)->days;

        $is_expired = ($diff > $days_limit);
        $already_submitted = false;

        if ($one_per_order) {
            $already_submitted = (bool) Db::getInstance()->getValue('
                SELECT id_withdrawal_request
                FROM ' . _DB_PREFIX_ . 'withdrawal_request
                WHERE id_order = ' . (int) $order->id
            );
        }

        if ($mode === 'hard' && $is_expired) {
            return;
        }

        if ($already_submitted) {
            return;
        }

        $this->context->smarty->assign(array(
            'withdrawal_url' => $this->context->link->getModuleLink($this->name, 'form', array('id_order' => $order->id)),
            'is_expired' => $is_expired,
            'mode' => $mode,
            'days_limit' => $days_limit
        ));

        return $this->display(__FILE__, 'views/templates/hook/order_detail.tpl');
    }

    public function hookActionGetExtraMailTemplateVars($params)
    {
        if ($params['template'] === 'order_conf') {
            if (isset($params['extraContext']['order'])) {
                $order = $params['extraContext']['order'];
            } elseif (isset($params['id_order'])) {
                $order = new Order((int) $params['id_order']);
            }

            if (isset($order) && Validate::isLoadedObject($order)) {
                $params['extra_template_vars']['{withdrawal_url}'] = $this->context->link->getModuleLink(
                    $this->name,
                    'form',
                    array(
                        'id_order' => $order->id,
                        'secure_key' => $order->secure_key
                    )
                );
            }
        }
    }

    public function hookModuleRoutes($params)
    {
        return [
            'module-withdrawalform-form' => [
                'controller' => 'form',
                'rule' => 'zwroty',
                'keywords' => [],
                'params' => [
                    'fc' => 'module',
                    'module' => $this->name,
                ],
            ],
        ];
    }

    public function hookDisplayCheckoutSubtotalDetails($params)
    {
        return $this->display(__FILE__, 'views/templates/hook/checkout_info.tpl');
    }

    public function hookActionOrderStatusPostUpdate($params)
    {
        $new_order_status = $params['newOrderStatus'];
        $id_order = (int) $params['id_order'];
        $order = new Order($id_order);

        // Standard PrestaShop status for "Delivered" is often 5
        // But it's better to check if it has the 'delivery' flag or by name
        if ($new_order_status->delivery || $new_order_status->id == (int) Configuration::get('PS_OS_DELIVERED')) {
            $customer = new Customer((int) $order->id_customer);

            $withdrawal_url = $this->context->link->getModuleLink(
                $this->name,
                'form',
                array(
                    'id_order' => $order->id,
                    'secure_key' => $order->secure_key
                ),
                true,
                (int) $order->id_lang
            );

            $template_vars = array(
                '{firstname}' => $customer->firstname,
                '{lastname}' => $customer->lastname,
                '{order_reference}' => $order->reference,
                '{withdrawal_url}' => $withdrawal_url,
            );

            Mail::Send(
                (int) $order->id_lang,
                'withdrawal_link',
                $this->l('Information about the right to return goods'),
                $template_vars,
                $customer->email,
                $customer->firstname . ' ' . $customer->lastname,
                null,
                null,
                null,
                null,
                $this->local_path . 'mails/'
            );
        }
    }
}
