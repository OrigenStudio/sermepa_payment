<?php

use CommerceRedsys\Payment\Sermepa;
use CommerceRedsys\Payment\SermepaException;

/**
 * Payment Controller for integration with Sermepa (RedSys) gateway.
 */
class SermepaPaymentMethodController extends PaymentMethodController {

  public $controller_data_defaults = array(
    'currency'          => '',
    'titular'           => '',
    'merchant_name'     => '',
    'merchant_code'     => '',
    'terminal'          => '',
    'signature'         => '',
    'environment'       => '',
    'encryption_method' => '',
  );

  public $payment_method_configuration_form_elements_callback = 'sermepa_payment_method_configuration';


  function __construct() {
    $this->title = 'Sermepa (RedSys)';
    $this->name = 'sermepa_payment';
    $this->description = 'Sermepa payment gateway integration';
  }


  /**
   * Implements PaymentMethodController::validate().
   */
  function validate(Payment $payment, PaymentMethod $payment_method, $strict) {
    // Empty on purpose.  For the moment there is no validation before the payment
    // execution.
  }


  /**
   * Implements PaymentMethodController::execute().
   */
  function execute(Payment $payment) {
    entity_save('payment', $payment);
    // The payment PID is stored in the session in order to check it when
    // the user is redirected to the payment submission form.  This allows us
    // to disable access to users who have not made a payment.
    $_SESSION['sermepa_payment_pid'] = $payment->pid;
    drupal_goto('sermepa/redirect/' . $payment->pid);
  }


  /**
   * Create a Sermepa object that acts as a gateway with the Sermepa TPV.
   */
  static function createGateway(Payment $payment) {
    libraries_load('sermepa');
    $settings = $payment->method->controller_data;
    return new Sermepa(
      $settings['merchant_name'],
      $settings['merchant_code'],
      $settings['terminal'],
      $settings['signature'],
      $settings['environment'],
      array(
        'currency' => $settings['currency'],
      ));
  }

  /**
   * Handles Sermepa response and change the payment status accordingly.
   *
   * Performs checks for response (feedback) sanity, validation of signature and
   * updates the status based on the response code.
   *
   * No operation will be performed by default if no feedback is received, but
   * this behavior can be changed by passing the 'force' option as TRUE. This
   * should be used in those cases where a feedback is truly expected so
   * status is updated accordingly.
   *
   * @param \Payment $payment
   * @param bool     $force
   */
  static function handleResponse(Payment $payment, $force = FALSE) {
    $sermepa = static::createGateway($payment);
    $feedback = $sermepa->getFeedback();
    if ($feedback || $force) {
      try {
        if (
          !$feedback ||
          !isset($feedback['Ds_MerchantParameters']) ||
          !isset($feedback['Ds_SignatureVersion']) ||
          !isset($feedback['Ds_Signature'])
        ) {
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_INVALID_FEEDBACK));
          throw new SermepaException('Invalid response');
        }
        
        if (!$sermepa->validSignatures($feedback)) {
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_INVALID_SIGNATURES));
          throw new SermepaException('Invalid signatures');
        }
        
        $decoded_parameters = $sermepa->decodeMerchantParameters($feedback['Ds_MerchantParameters']);
        static::setStatusFromResponseCode($payment, $decoded_parameters['Ds_Response']);
        watchdog('sermepa_payment', "Feedback for payment @pid was valid.\nResponse was: @response", array(
          '@pid' => $payment->pid,
          '@response' => $sermepa::handleResponse($decoded_parameters['Ds_Response']),
        ));
        
      } catch (SermepaException $e) {
        watchdog('sermepa_payment', "Feedback for payment @pid was NOT valid.\nError was: @error", array(
          '@pid' => $payment->pid,
          '@error' => $e->getMessage(),
        ));
        if (payment_status_is_or_has_ancestor($payment->getStatus(), PAYMENT_STATUS_PENDING)) {
          $payment->setStatus(new PaymentStatusItem(PAYMENT_STATUS_UNKNOWN));
        }
      }
    }
  }
  
  /**
   * Sets the corresponding status to the given payment for the received response code.
   *
   * @param \Payment $payment
   * @param int $response_code
   */
  static function setStatusFromResponseCode(Payment $payment, $response_code) {
    if ((int) $response_code <= 99) {
      $payment->setStatus(new PaymentStatusItem(PAYMENT_STATUS_SUCCESS));
    }
    else {
      $payment->setStatus(new PaymentStatusItem(PAYMENT_STATUS_FAILED));
      switch ((int) $response_code) {
        case 900:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_RETURNS_CONFIRMATIONS));
          break;
      
        case 101:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_EXPIRED));
          break;
      
        case 102:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_EXCEPTION_CARD));
          break;
      
        case 104:
        case 9104:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_NOT_ALLOWED));
          break;
      
        case 116:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_INSUFFICIENT));
          break;
      
        case 118:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_UNREGISTERED));
          break;
      
        case 129:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_SECURITY_CODE));
          break;
      
        case 180:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_OUT_OF_SERVICE));
          break;
      
        case 184:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_OWNER_AUTHENTICATION));
          break;
      
        case 190:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_NO_REASONS));
          break;
      
        case 191:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_WRONG_EXPIRATION));
          break;
      
        case 202:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_FRAUD));
          break;
      
        case 912:
        case 9912:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_ISSUER_UNAVAILABLE));
          break;
      
        case 913:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_ORDER_DUPLICATED));
          break;
      
        default:
          $payment->setStatus(new PaymentStatusItem(SERMEPA_PAYMENT_STATUS_TRANSACTION_REFUSED));
          break;
      }
    }
  }
  
}