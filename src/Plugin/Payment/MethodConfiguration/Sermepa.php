<?php

/**
 * Contains \Drupal\sermepa_payment\Plugin\Payment\MethodConfiguration\Sermepa.
 */

namespace Drupal\sermepa_payment\Plugin\Payment\MethodConfiguration;

use Drupal\Component\Utility\NestedArray;
use Drupal\Core\Form\FormStateInterface;
use Drupal\payment\Plugin\Payment\MethodConfiguration\Basic;
use CommerceRedsys\Payment\Sermepa as SermepaApi;

/**
 * Provides the configuration for the Sermepa payment method plugin.
 *
 * @PaymentMethodConfiguration(
 *   description = @Translation("Sermepa payment method type."),
 *   id = "sermepa_payment",
 *   label = @Translation("Sermepa")
 * )
 */
class Sermepa extends Basic {

  /**
   * Gets the setting for the production server.
   *
   * @return string
   */
  public function getEnvironment() {
    return !empty($this->configuration['environment']) ? $this->configuration['environment'] : '';
  }

  /**
   * Gets the setting for the merchant name.
   *
   * @return string
   */
  public function getMerchantName() {
    return !empty($this->configuration['merchant_name']) ? $this->configuration['merchant_name'] : '';
  }

  /**
   * Gets the setting for the merchant code.
   *
   * @return string
   */
  public function getMerchantCode() {
    return !empty($this->configuration['merchant_code']) ? $this->configuration['merchant_code'] : '';
  }

  /**
   * Gets the setting for the merchant terminal.
   *
   * @return string
   */
  public function getMerchantTerminal() {
    return !empty($this->configuration['merchant_terminal']) ? $this->configuration['merchant_terminal'] : '';
  }

  /**
   * Gets the setting for the merchant currency.
   *
   * @return string
   */
  public function getMerchantCurrency() {
    return !empty($this->configuration['merchant_currency']) ? $this->configuration['merchant_currency'] : '';
  }

  /**
   * Gets the setting for the merchant payment method.
   *
   * @return string
   */
  public function getMerchantPaymentMethod() {
    return !empty($this->configuration['merchant_payment_method']) ? $this->configuration['merchant_payment_method'] : '';
  }

  /**
   * Gets the setting for the encryption key.
   *
   * @return string
   */
  public function getEncryptionKey() {
    return !empty($this->configuration['encryption_key']) ? $this->configuration['encryption_key'] : '';
  }

  /**
   * Implements a form API #process callback.
   */
  public function processBuildConfigurationForm(array &$element, FormStateInterface $form_state, array &$form) {
    parent::processBuildConfigurationForm($element, $form_state, $form);

    $element['sermepa'] = [
      '#type' => 'fieldset',
      '#title' => $this->t("SERMEPA configuration")
    ];
    $element['sermepa']['environment'] = [
      '#type' => 'select',
      '#title' => $this->t('Environment'),
      '#options' => array(
        "live" => $this->t("Live"),
        "test" => $this->t("Test")
      ),
      '#required' => TRUE,
      '#default_value' => $this->getEnvironment(),
    ];
    $element['sermepa']['merchant_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Merchant Name'),
      '#maxlength' => SermepaApi::getMerchantNameMaxLength(),
      '#required' => TRUE,
      '#default_value' => $this->getMerchantName()
    ];
    $element['sermepa']['merchant_code'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Merchant Code'),
      '#maxlength' => SermepaApi::getMerchantCodeMaxLength(),
      '#required' => TRUE,
      '#default_value' => $this->getMerchantCode()
    ];
    $element['sermepa']['merchant_terminal'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Merchant Terminal'),
      '#maxlength' => SermepaApi::getMerchantTerminalMaxLength(),
      '#required' => TRUE,
      '#default_value' => $this->getMerchantTerminal()
    ];
    $element['sermepa']['merchant_currency'] = [
      '#type' => 'select',
      '#title' => $this->t('Merchant Currency'),
      '#options' => SermepaApi::getAvailableCurrencies(),
      '#required' => TRUE,
      '#default_value' => $this->getMerchantCurrency()
    ];
    $element['sermepa']['merchant_payment_method'] = [
      '#type' => 'select',
      '#title' => $this->t('Merchant Payment Method'),
      '#options' => SermepaApi::getAvailablePaymentMethods(),
      '#required' => TRUE,
      '#default_value' => $this->getMerchantPaymentMethod()
    ];
    $element['sermepa']['encryption_key'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Encryption Key'),
      '#maxlength' => SermepaApi::getMerchantPasswordMaxLength(),
      '#required' => TRUE,
      '#default_value' => $this->getEncryptionKey()
    ];

    return $element;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    parent::submitConfigurationForm($form, $form_state);

    $parents = $form['plugin_form']['sermepa']['#parents'];
    array_pop($parents);
    $values = $form_state->getValues();
    $values = NestedArray::getValue($values, $parents);
    $this->configuration['environment'] = $values['sermepa']['environment'];
    $this->configuration['merchant_name'] = $values['sermepa']['merchant_name'];
    $this->configuration['merchant_code'] = $values['sermepa']['merchant_code'];
    $this->configuration['merchant_terminal'] = $values['sermepa']['merchant_terminal'];
    $this->configuration['merchant_currency'] = $values['sermepa']['merchant_currency'];
    $this->configuration['merchant_payment_method'] = $values['sermepa']['merchant_payment_method'];
    $this->configuration['encryption_key'] = $values['sermepa']['encryption_key'];
  }

  /**
   * @return array
   */
  public function getDerivativeConfiguration() {
    return [
      'environment' => $this->getEnvironment(),
      'merchant_name' => $this->getMerchantName(),
      'merchant_code' => $this->getMerchantCode(),
      'merchant_terminal' => $this->getMerchantTerminal(),
      'merchant_currency' => $this->getMerchantCurrency(),
      'merchant_payment_method' => $this->getMerchantPaymentMethod(),
      'encryption_key' => $this->getEncryptionKey()
    ];
  }

}