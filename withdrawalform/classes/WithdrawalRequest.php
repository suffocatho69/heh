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

class WithdrawalRequest extends ObjectModel
{
    public $id_order;
    public $id_customer;
    public $reason;
    public $message;
    public $selected_products;
    public $ip_address;
    public $id_order_return;
    public $date_add;

    public static $definition = array(
        'table' => 'withdrawal_request',
        'primary' => 'id_withdrawal_request',
        'fields' => array(
            'id_order' => array('type' => self::TYPE_INT, 'validate' => 'isUnsignedId', 'required' => true),
            'id_customer' => array('type' => self::TYPE_INT, 'validate' => 'isUnsignedId', 'required' => true),
            'reason' => array('type' => self::TYPE_STRING, 'validate' => 'isString', 'size' => 255),
            'message' => array('type' => self::TYPE_HTML, 'validate' => 'isCleanHtml'),
            'selected_products' => array('type' => self::TYPE_STRING, 'validate' => 'isString'),
            'ip_address' => array('type' => self::TYPE_STRING, 'validate' => 'isAnything', 'size' => 45),
            'id_order_return' => array('type' => self::TYPE_INT, 'validate' => 'isUnsignedId'),
            'date_add' => array('type' => self::TYPE_DATE, 'validate' => 'isDate'),
        ),
    );
}
