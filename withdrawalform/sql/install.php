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

$sql = array();

$sql[] = 'CREATE TABLE IF NOT EXISTS `' . _DB_PREFIX_ . 'withdrawal_request` (
    `id_withdrawal_request` int(11) NOT NULL AUTO_INCREMENT,
    `id_order` int(11) NOT NULL,
    `id_customer` int(11) NOT NULL,
    `reason` varchar(255) DEFAULT NULL,
    `message` text DEFAULT NULL,
    `selected_products` text DEFAULT NULL,
    `ip_address` varchar(45) DEFAULT NULL,
    `id_order_return` int(11) DEFAULT NULL,
    `status` varchar(20) NOT NULL DEFAULT \'pending\',
    `date_add` datetime NOT NULL,
    PRIMARY KEY (`id_withdrawal_request`),
    KEY `id_order` (`id_order`),
    KEY `id_customer` (`id_customer`)
) ENGINE=' . _MYSQL_ENGINE_ . ' DEFAULT CHARSET=utf8;';

foreach ($sql as $query) {
    if (Db::getInstance()->execute($query) == false) {
        return false;
    }
}

// Add missing columns if the table already existed
$columns = Db::getInstance()->executeS(
    'SHOW COLUMNS FROM `' . _DB_PREFIX_ . 'withdrawal_request`'
);
$existing = array_column($columns ?: [], 'Field');

if (!in_array('status', $existing)) {
    Db::getInstance()->execute(
        'ALTER TABLE `' . _DB_PREFIX_ . 'withdrawal_request`
         ADD COLUMN `status` varchar(20) NOT NULL DEFAULT \'pending\'
         AFTER `id_order_return`'
    );
}
if (!in_array('selected_products', $existing)) {
    Db::getInstance()->execute(
        'ALTER TABLE `' . _DB_PREFIX_ . 'withdrawal_request`
         ADD COLUMN `selected_products` text DEFAULT NULL'
    );
}
if (!in_array('id_order_return', $existing)) {
    Db::getInstance()->execute(
        'ALTER TABLE `' . _DB_PREFIX_ . 'withdrawal_request`
         ADD COLUMN `id_order_return` int(11) DEFAULT NULL'
    );
}
