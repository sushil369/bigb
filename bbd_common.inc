<?php

/**
 * Page callback for admin_mobile_customer_service_details
 */
function bbd_common_price_discount_config_form($form, &$form_state) {

  $form['sp_max_discount'] = array(
    '#type' => 'textfield',
    '#title' => 'MAX Allowed Discount on Selling Price (In Percentage)',
    '#element_validate' => array('element_validate_integer_positive'),
    '#default_value' => variable_get('sp_max_discount'),
  );
  $form['fr_code_nid'] = array(
    '#type' => 'textfield',
    '#title' => 'What is Franchisee Code Nid', // what is fra code nid 
   // '#element_validate' => array('element_validate_integer_positive'),
    '#default_value' => variable_get('fr_code_nid'),
  );
  return system_settings_form($form);
}
/**
 * 
 * @param type $form
 * @param type $form_state
 * @return type variable set for load testing runtime
 */
function device_statistics_config_form($form, &$form_state) {

  $form['load_testing_runtime'] = array(
    '#type' => 'textfield',
    '#title' => 'Device Statistics run after (In Hr)',
    '#element_validate' => array('element_validate_integer_positive'),
    '#default_value' => variable_get('load_testing_runtime'),
  );
  return system_settings_form($form);
}

function show_catalogs(){
  drupal_set_message('Showing Last 3 catalogs generated');
  $directory_name = 'catalog_csv';
  $file_realpath = drupal_realpath(file_build_uri($directory_name));
  $files = glob("$file_realpath/*.csv");
  $output = '';
  array_multisort(
          array_map('filemtime', $files), SORT_NUMERIC, SORT_DESC, $files
      );
  foreach($files as $file){
    $filename = basename($file);
    $file_url = file_create_url(file_build_uri($directory_name.'/'.$filename));

    $creation_time = filemtime($file);
    $output .= l(t(date('Y-m-d H:i:s', $creation_time)),$file_url) . "<br/><br/>";
  }
  return $output;
}
