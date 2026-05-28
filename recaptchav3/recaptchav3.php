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

if (!defined('_PS_VERSION_')) {
    exit;
}

class Recaptchav3 extends Module
{
    protected $config_prefix = 'RECAPTCHAV3_';

    public function __construct()
    {
        $this->name = 'recaptchav3';
        $this->tab = 'front_office_features';
        $this->version = '1.1.0';
        $this->author = 'Orientica.pl';
        $this->need_instance = 0;

        $this->bootstrap = true;

        parent::__construct();

        $this->displayName = $this->l('Google reCAPTCHA V3');
        $this->description = $this->l('Chroń swój sklep przed spamem i botami używając Google reCAPTCHA V3.');

        $this->ps_versions_compliancy = array('min' => '1.7.0', 'max' => _PS_VERSION_);
    }

    public function install()
    {
        return parent::install() &&
            $this->registerHook('header') &&
            $this->registerHook('actionContactFormSubmitBefore') &&
            Configuration::updateValue($this->config_prefix . 'SITE_KEY', '') &&
            Configuration::updateValue($this->config_prefix . 'SECRET_KEY', '') &&
            Configuration::updateValue($this->config_prefix . 'THRESHOLD', 0.5) &&
            Configuration::updateValue($this->config_prefix . 'ENABLE_CONTACT', 1) &&
            Configuration::updateValue($this->config_prefix . 'ENABLE_REGISTER', 1) &&
            Configuration::updateValue($this->config_prefix . 'ENABLE_LOGIN', 1);
    }

    public function uninstall()
    {
        return parent::uninstall() &&
            Configuration::deleteByName($this->config_prefix . 'SITE_KEY') &&
            Configuration::deleteByName($this->config_prefix . 'SECRET_KEY') &&
            Configuration::deleteByName($this->config_prefix . 'THRESHOLD') &&
            Configuration::deleteByName($this->config_prefix . 'ENABLE_CONTACT') &&
            Configuration::deleteByName($this->config_prefix . 'ENABLE_REGISTER') &&
            Configuration::deleteByName($this->config_prefix . 'ENABLE_LOGIN');
    }

    public function getContent()
    {
        $output = '';

        if (Tools::isSubmit('submit' . $this->name)) {
            $site_key = Tools::getValue('RECAPTCHAV3_SITE_KEY');
            $secret_key = Tools::getValue('RECAPTCHAV3_SECRET_KEY');
            $threshold = (float) Tools::getValue('RECAPTCHAV3_THRESHOLD');

            $errors = array();
            if (empty($site_key)) {
                $errors[] = $this->l('Site Key is required.');
            }

            // Only require Secret Key if it's not already set in Configuration
            if (empty($secret_key) && !Configuration::get($this->config_prefix . 'SECRET_KEY')) {
                $errors[] = $this->l('Secret Key is required.');
            }

            if (!empty($errors)) {
                foreach ($errors as $err) {
                    $output .= $this->displayError($err);
                }
            } else {
                Configuration::updateValue($this->config_prefix . 'SITE_KEY', $site_key);
                if (!empty($secret_key)) {
                    Configuration::updateValue($this->config_prefix . 'SECRET_KEY', $secret_key);
                }
                Configuration::updateValue($this->config_prefix . 'THRESHOLD', $threshold);
                Configuration::updateValue($this->config_prefix . 'ENABLE_CONTACT', (int) Tools::getValue('RECAPTCHAV3_ENABLE_CONTACT'));
                Configuration::updateValue($this->config_prefix . 'ENABLE_REGISTER', (int) Tools::getValue('RECAPTCHAV3_ENABLE_REGISTER'));
                Configuration::updateValue($this->config_prefix . 'ENABLE_LOGIN', (int) Tools::getValue('RECAPTCHAV3_ENABLE_LOGIN'));

                $output .= $this->displayConfirmation($this->l('Settings updated.'));
            }
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
                        'label' => $this->l('Site Key'),
                        'name' => 'RECAPTCHAV3_SITE_KEY',
                        'required' => true,
                    ),
                    array(
                        'type' => 'password',
                        'label' => $this->l('Secret Key'),
                        'name' => 'RECAPTCHAV3_SECRET_KEY',
                        'required' => (Configuration::get($this->config_prefix . 'SECRET_KEY') ? false : true),
                        'desc' => Configuration::get($this->config_prefix . 'SECRET_KEY')
                            ? $this->l('Key is already saved. Leave empty to keep it.')
                            : $this->l('Please enter your Secret Key.'),
                    ),
                    array(
                        'type' => 'text',
                        'label' => $this->l('Threshold'),
                        'name' => 'RECAPTCHAV3_THRESHOLD',
                        'desc' => $this->l('Minimum score (0.0 to 1.0) to consider the user human. Default 0.5.'),
                    ),
                    array(
                        'type' => 'switch',
                        'label' => $this->l('Protect Contact Form'),
                        'name' => 'RECAPTCHAV3_ENABLE_CONTACT',
                        'is_bool' => true,
                        'values' => array(
                            array('id' => 'active_on', 'value' => 1, 'label' => $this->l('Enabled')),
                            array('id' => 'active_off', 'value' => 0, 'label' => $this->l('Disabled')),
                        ),
                    ),
                    array(
                        'type' => 'switch',
                        'label' => $this->l('Protect Registration Form'),
                        'name' => 'RECAPTCHAV3_ENABLE_REGISTER',
                        'is_bool' => true,
                        'values' => array(
                            array('id' => 'active_on', 'value' => 1, 'label' => $this->l('Enabled')),
                            array('id' => 'active_off', 'value' => 0, 'label' => $this->l('Disabled')),
                        ),
                    ),
                    array(
                        'type' => 'switch',
                        'label' => $this->l('Protect Login Form'),
                        'name' => 'RECAPTCHAV3_ENABLE_LOGIN',
                        'is_bool' => true,
                        'values' => array(
                            array('id' => 'active_on', 'value' => 1, 'label' => $this->l('Enabled')),
                            array('id' => 'active_off', 'value' => 0, 'label' => $this->l('Disabled')),
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
            'RECAPTCHAV3_SITE_KEY' => Configuration::get($this->config_prefix . 'SITE_KEY'),
            'RECAPTCHAV3_SECRET_KEY' => '', // Never prefill Secret Key for security
            'RECAPTCHAV3_THRESHOLD' => Configuration::get($this->config_prefix . 'THRESHOLD'),
            'RECAPTCHAV3_ENABLE_CONTACT' => Configuration::get($this->config_prefix . 'ENABLE_CONTACT'),
            'RECAPTCHAV3_ENABLE_REGISTER' => Configuration::get($this->config_prefix . 'ENABLE_REGISTER'),
            'RECAPTCHAV3_ENABLE_LOGIN' => Configuration::get($this->config_prefix . 'ENABLE_LOGIN'),
        );
    }

    public function hookHeader()
    {
        $page_name = $this->context->controller->php_self;
        $allowed_pages = array('contact', 'authentication', 'registration');

        if (!in_array($page_name, $allowed_pages)) {
            return;
        }

        $site_key = Configuration::get($this->config_prefix . 'SITE_KEY');
        if (empty($site_key)) {
            return;
        }

        $this->context->controller->registerJavascript(
            'remote-recaptcha',
            'https://www.google.com/recaptcha/api.js?render=' . $site_key,
            array('server' => 'remote', 'position' => 'head', 'priority' => 10)
        );

        Media::addJsDef(array(
            'recaptchav3_site_key' => $site_key,
            'recaptchav3_enable_contact' => (bool) Configuration::get($this->config_prefix . 'ENABLE_CONTACT'),
            'recaptchav3_enable_register' => (bool) Configuration::get($this->config_prefix . 'ENABLE_REGISTER'),
            'recaptchav3_enable_login' => (bool) Configuration::get($this->config_prefix . 'ENABLE_LOGIN'),
        ));

        $this->context->controller->registerJavascript(
            'module-recaptchav3-js',
            'modules/' . $this->name . '/views/js/recaptchav3.js',
            array('position' => 'bottom', 'priority' => 100)
        );
    }

    public function hookActionContactFormSubmitBefore($params)
    {
        if (!Configuration::get($this->config_prefix . 'ENABLE_CONTACT')) {
            return;
        }

        if (!$this->validateToken('contact')) {
            $this->context->controller->errors[] = $this->l('reCAPTCHA verification failed. Please try again.');
        }
    }

    public function validateToken($action)
    {
        $token = Tools::getValue('recaptcha_token');
        $secret_key = Configuration::get($this->config_prefix . 'SECRET_KEY');
        $threshold = (float) Configuration::get($this->config_prefix . 'THRESHOLD');

        if (empty($token) || empty($secret_key)) {
            return false;
        }

        $response = $this->getVerifyResponse($token, $secret_key);

        if ($response && isset($response['success']) && $response['success'] === true) {
            if (isset($response['score']) && $response['score'] >= $threshold && $response['action'] === $action) {
                return true;
            }
        }

        return false;
    }

    protected function getVerifyResponse($token, $secret_key)
    {
        $url = 'https://www.google.com/recaptcha/api/siteverify';
        $data = array(
            'secret' => $secret_key,
            'response' => $token,
            'remoteip' => Tools::getRemoteAddr(),
        );

        if (function_exists('curl_init')) {
            $ch = curl_init();
            curl_setopt($ch, CURLOPT_URL, $url);
            curl_setopt($ch, CURLOPT_POST, 1);
            curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            $result = curl_exec($ch);
            curl_close($ch);
        } else {
            $options = array(
                'http' => array(
                    'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
                    'method'  => 'POST',
                    'content' => http_build_query($data),
                ),
            );
            $context  = stream_context_create($options);
            $result = file_get_contents($url, false, $context);
        }

        if ($result === false) {
            PrestaShopLogger::addLog('reCAPTCHA v3: Failed to connect to Google API', 3);
            return null;
        }

        $decoded_response = json_decode($result, true);
        if (!$decoded_response || (isset($decoded_response['success']) && $decoded_response['success'] === false)) {
            $error_codes = isset($decoded_response['error-codes']) ? implode(', ', $decoded_response['error-codes']) : 'no error codes';
            PrestaShopLogger::addLog('reCAPTCHA v3 verification failed: ' . $error_codes, 3);
        }

        return $decoded_response;
    }
}
