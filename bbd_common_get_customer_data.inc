<?php

//Setting batch for getting franchisee's and their respective customer data.
function get_customer_from_franchisee() {
  $batch = array(
    'operations' => array(
      array('batch_example_process', array()),
    ),
    'finished' => 'batch_example_finished',
    'title' => t('Processing Example Batch'),
    'init_message' => t('Customer Data  Batch is starting.'),
    // 'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Customer Data Batch has encountered an error.'),
  );
  batch_set($batch);
  batch_process('franchisee/download/excel');
}

// Excecuting Batch Operation 
function batch_example_process(&$context) {
  if (!isset($context['sandbox']['progress'])) {

    //if temp.csv file not exist then create it.
    if (!file_exists($_SERVER['DOCUMENT_ROOT'] . '/sites/default/files/franchisee')) {
      mkdir($_SERVER['DOCUMENT_ROOT'] . '/sites/default/files/franchisee');
    }

    $fp = fopen($_SERVER['DOCUMENT_ROOT'] . '/sites/default/files/franchisee/temp.csv', 'w+') or die('Cant open file!');
    $myHeaders = array("Franchisee Name", "Franchisee Code", "Franchisee City", "Franchisee State","Customer Name", "Customer Email", "Customer Phone Number", " Customer City", "Customer State");
    fputcsv($fp, $myHeaders);
    $context['sandbox']['progress'] = 0;

    //calculating Total No of Franchisee and getting their data.
 
    $user_count = COUNT(get_franchisee_data_from_batch());
    $context['sandbox']['max'] = $user_count;
  }

  $progress = $context['sandbox']['progress'];
  $limit = 50;

  //Calculating All Franchisee  Data According To Range.
  $user_data = get_franchisee_data_from_batch($progress, $limit);

  //Calculating All Franchisee Data  With Respective Their Customer's Data.
  $data = bbd_common_get_data_from_franchisee($context['sandbox']['progress'], $limit, $user_data);

  $file_path = $_SERVER['DOCUMENT_ROOT'] . '/sites/default/files/franchisee/temp.csv';
  $handle = fopen($file_path, 'a');

  //writing all data in csv file.
  foreach ($data as $key => $value) {
    fputcsv($handle, $value);
  }
  fclose($handle);
  
  $context['sandbox']['progress'] += $limit;
  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  $customer_remaining = $context['sandbox']['max'] - $context['sandbox']['progress'];
  $customer_remaining = $customer_remaining < 0 ? 0 : $customer_remaining;
  $context['message'] = t($customer_remaining . ' Customer are remaining.');

  if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {

    $context['finished'] = 1;
  }
}

/**
 * 
 * @param type $success
 * @param type $results
 * @param type $operations
 * 
 * Batch Process is finished then show success or error message.
 */
function batch_example_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(check_plain("successfully created Excel File."));
  }
  else {
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

// Giving Download Template Option after Batch process Finished.
function get_thankyou_msg() {
  $csv_template = 'sites/default/files/franchisee/temp.csv';
  $click = l(t('Click Here'), $csv_template, array());
  $anchor = $click . ' To download Template';
  return $anchor;
}

/**
 * 
 * @param type $progress(integer) = Progress no which is coming from Batch Process
 * @param type $limit(integer) = Batch Limit in this case it is 50.
 * @return type
 * 
 * Calculating All Franchisee  Data.
 */
function get_franchisee_data_from_batch($progress = NULL, $limit = NULL) {
  $query = db_select('users', 'u');
  $query->leftJoin('users_roles', 'ur', 'ur.uid = u.uid');
  $query->Join('role', 'rl', 'ur.rid = rl.rid');
  $query->Join('field_data_field_franchisee_code', 'fc', 'fc.entity_id = u.uid');
  $query->condition("rl.rid ", 5);
  $query->condition('fc.field_franchisee_code_value', db_like(142).'%', 'LIKE');
  if (!empty($progress) && !empty($limit)) {
    $query->range($context['sandbox']['progress'], $limit);
  }
  $query->distinct();
  $query->fields('u', array('uid'));
  $user = $query->execute()->fetchAll();
  return $user;
}

/**
 * 
 * @param type $start
 * @param type $limit
 * @param type $users(array of franchisee data)
 * @return type
 * 
 * 
 * calculating All Franchisee  with their respective customer's Data.
 */
function bbd_common_get_data_from_franchisee($start, $limit, $users) {

  foreach ($users as $key => $val) {
    $uid = $val->uid;

    //Get Franchisee's  Data.
    $data = get_franchisee_data_from_uid($uid, $start, $limit);

    // Get Franchisee's Customers Data.
    $query = get_customer_directory_query_new($uid);
    $result = $query->execute()->fetchAll(PDO::FETCH_ASSOC);

    if (!empty($result)) {
      foreach ($result as $key => $value) {
        $array[] = array_merge($data, $value);
      }
    }
  }
  return $array;
}

/**
 * 
 * @param type $userid (Franchisee's User Id)
 * @return type
 * 
 *  Get Franchisee's Customers Data.
 */
function get_customer_directory_query_new($userid) {

  //Start 1st Query
  $query1 = db_select('users', 'u');
  $query1->join('field_data_field_franchisee_user', 'fu', 'fu.entity_id = u.uid');
  $query1->leftJoin('commerce_order', 'o', 'o.uid = u.uid');
  $query1->join('field_data_field_user_billing_address', 'addr', 'addr.entity_id = u.uid');
  $query1->join('field_data_field_phone_number', 'pn', 'pn.entity_id = u.uid');


  $query1->addField('u', 'uid', 'user_id');
  $query1->addField('addr', 'field_user_billing_address_name_line', 'name_line');
  $query1->addField('u', 'mail', 'email');
  $query1->addField('pn', 'field_phone_number_value', 'phone_no');
  $query1->addField('addr', 'field_user_billing_address_locality', 'city');
  $query1->addField('addr', 'field_user_billing_address_administrative_area', 'state');
  $query1->isNull('o.order_id');
  $query1->condition('fu.field_franchisee_user_uid', $userid, '=');


  //Start 2nd Query
  $query2 = db_select('commerce_order', 'co');
  $query2->leftJoin('field_data_field_franchisee_uid', 'fuid', 'fuid.entity_id = co.order_id');
  $query2->leftJoin('field_data_commerce_customer_billing', 'fcb', 'fuid.entity_id = fcb.entity_id');
  $query2->leftjoin('field_data_field_phone_number', 'pn', 'fcb.commerce_customer_billing_profile_id = pn.entity_id');
  $query2->join('field_data_field_neft_customer_email', 'cemail', 'cemail.entity_id = fcb.commerce_customer_billing_profile_id');
  $query2->join('field_data_commerce_customer_address', 'cadd', 'cadd.entity_id = fcb.commerce_customer_billing_profile_id');
  $query2->addField('co', 'uid', 'user_id');
  $query2->addField('cadd', 'commerce_customer_address_name_line', 'name_line');
  $query2->addField('cemail', 'field_neft_customer_email_value', 'email');
  $query2->addField('pn', 'field_phone_number_value', 'phone_no');
  $query2->addField('cadd', 'commerce_customer_address_locality', 'city');
  $query2->addField('cadd', 'commerce_customer_address_administrative_area', 'state');
  $query2->condition('fuid.field_franchisee_uid_uid', $userid, '=');
  $status_array = array('canceled', 'payment_refunded', 'replacement_in_process', 'partial_shipped', 'partial_returned', 'partial_processed', 'replacement_processed', 'replace_completed', 'shipped', 'return_to_seller_in_process', 'partial_delivered', 'order_placed', 'delivered', 'delivery_attempted', 'in_process', 'order_confirmed', 'jcaps_success', 'jcaps_fail', 'processing', 'delivery', 'completed');
  $query2->condition('co.status', $status_array, 'IN');


  $query2->groupBy('pn.field_phone_number_value');

  $query = Database::getConnection()
      ->select($query1->union($query2))
      ->fields(NULL, array('user_id', 'name_line', 'email', 'phone_no', 'city', 'state'));

  return $query;
}

/**
 * 
 * @param type $uid(Franchisee's user Id)
 * @param type $start
 * @param type $length
 * @return type
 * 
 * Get Franchisees data from his user id. 
 */
function get_franchisee_data_from_uid($uid, $start, $length) {
  $query = db_select('users', 'u');
  $query->Join('field_data_field_franchisee_code', 'fc', 'fc.entity_id = u.uid');
  $query->join('field_data_field_user_billing_address', 'addr', 'addr.entity_id = u.uid');
  $query->condition("u.uid ", $uid);
  $query->condition('fc.field_franchisee_code_value', db_like(142).'%', 'LIKE');
  $query->addField('addr', 'field_user_billing_address_name_line', 'FRname_line');
  $query->addField('fc', 'field_franchisee_code_value', 'FRCODE');
  $query->addField('addr', 'field_user_billing_address_locality', 'FRcity');
  $query->addField('addr', 'field_user_billing_address_administrative_area', 'FRstate');
  $data = $query->execute()->fetchAssoc();
  return $data;
}
