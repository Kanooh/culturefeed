<?php

/**
 * @file
 * Common functionality for the whole culturefeed framework.
 */
define('CULTUREFEED_GOOGLE_MAPS_JS_API', 'https://maps.googleapis.com/maps/api/js?sensor=true&v=3.5&region=BE&hl=nl&language=nl');

/**
 * Check if we are connected to acceptance API.
 */
function culturefeed_acceptance_mode() {

  // Get culturefeed_api_location
  $location = variable_get('culturefeed_api_location');
  if (strpos($location,'www') === FALSE) {
    return TRUE;
  }

  // Check for acc or dev environments that are connected with production
  else {
    global $base_root;
    $dev_ids = array('culturefeed', 'dev.', 'acc.');
    foreach($dev_ids as $id) {
      if (stripos($base_root, $id) !== false) {
        return TRUE;
      }
    }
    return FALSE;
  }

}

/**
 * Set the "noindex, follow" meta tag when needed.
 */
function culturefeed_set_noindex_metatag() {

  $element = array(
    '#tag' => 'meta',
    '#attributes' => array(
      'name' => 'robots',
      'content' => 'noindex, follow',
    ),
  );

  drupal_add_html_head($element, 'robots');

}


/**
 * Store the page image in the drupal files temporary.
 * @return file object or false if failed.
 */
function culturefeed_create_temporary_image($image, $destination) {

  if (strpos($image, 'http') === false) {
    if (isset($_SERVER['HTTPS'])) {
      $image = 'https:' . $image;
    }
    else {
      $image = 'http:' . $image;
    }
  }

  $image_data = file_get_contents($image);
  file_prepare_directory($destination, FILE_CREATE_DIRECTORY);
  $uri = file_unmanaged_save_data($image_data, $destination . '/' . drupal_basename($image));

  if (empty($uri)) {
    return FALSE;
  }

  global $user;
  $file = new stdClass();
  $file->uri = $uri;
  $file->filename = drupal_basename($uri);
  $file->filemime = file_get_mimetype($file->uri);
  $file->uid = $user->uid;
  $file->status = FALSE;

  return file_save($file);
}

/**
 * Load a block through ajax.
 */
function culturefeed_ui_block_ajaxload($config) {
  $block['subject'] = $config['title'];
  $block['content'] = array(
    '#markup' => '<div id="' . $config['id'] . '">' . t('Loading') . '</div>',
    '#id' => $config['id'],
    '#ajax' => array(
      'event' => 'ajaxload',
      'path' => $config['path'],
    ),
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'culturefeed') . '/js/culturefeed_ajaxload.js',
        array('data' => array(), 'type' => 'setting'),
      ),
    ),
  );

  $block['content'] = ajax_pre_render_element($block['content']);

  return $block;
}

/**
 * Generate an image style url for an external image.
 */
function culturefeed_external_image_style($image_url, $image_style) {

  global $base_url;
  $curr_url = str_replace('http://', '', $base_url);
  $image_url_parts = explode('/', str_replace('http://', '', $image_url));
  $curr_url_parts = explode('/', $curr_url);

  $file_directory = variable_get('file_public_path', 'sites/default/files');

  // If this file is original of current host, just use the original path.
  if ($image_url_parts[0] == $curr_url_parts[0]) {
    $local_uri = trim(str_replace($base_url . '/', '', $image_url), '/');
    $local_uri = str_replace($file_directory . '/', 'public://', $local_uri);
  }
  // If the file is of another site, download it locally. This is needed to use imagecache.
  else {

    $file = new stdClass();
    $file->filename = $image_url;
    $validation_errors = file_validate_extensions($file, 'jpg jpeg png gif');
    if ($validation_errors) {
      return;
    }

    $name_parts = explode('/', $image_url);
    $file_name = $name_parts[count($name_parts) - 1];

    $local_uri = 'public://external/' . $file_name;
    $directory = $file_directory . '/external';
    // Not needed if the file already exists.
    if (!file_exists($local_uri)) {
      file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
      $image_content = @file_get_contents($image_url);
      if ($image_content) {
        file_put_contents($directory . '/' . $file_name, $image_content);
      }
      else {
        return;
      }
    }

  }

  return image_style_url($image_style, $local_uri);

}

/**
 * Ajax command for a modal.
 * This will check if culturefeed_bootstrap's modal command is available.
 * If not, it will do a html insert.
 */
function culturefeed_ajax_command_modal($selector, $html) {
  return array(
    'command' => 'culturefeedModal',
    'selector' => $selector,
    'data' => $html,
  );
}

/**
 * Ajax command to reload current page.
 */
function culturefeed_ajax_command_goto($url) {
  return array(
    'command' => 'culturefeedGoto',
    'url' => $url,
  );
}

/**
 * Getter function to retrieve a Mobile_Detect() singleton.
 * @return object
 * Instance of Mobile_Detect(), NULL if failure.
 */
function culturefeed_mobile_detect_get_object() {
  $detect = &drupal_static(__FUNCTION__);

  if (!isset($detect)) {
    if (class_exists('Mobile_Detect')) {
      try {
        $detect = new Mobile_Detect();
      } catch (Exception $e) {
        $detect = NULL;
        watchdog('mobile_detect', 'Could not instantiate Mobile_Detect(): %message',
          array('%message' => $e->getMessage()), WATCHDOG_ALERT);
      }
    } else {
      $detect = NULL;
      watchdog('mobile_detect', 'Mobile_Detect() class does not exist', array(), WATCHDOG_ALERT);
    }
  }

  return $detect;
}

/**
 * Known versions of cdb xml.
 *
 * @return array
 */
function culturefeed_cdb_xml_versions() {
  $cdb_xml_versions = array(
    '3.0',
    '3.1',
    '3.2',
    '3.3',
  );

  return $cdb_xml_versions;
}

/*
f
*/
function culturefeed_share_link($share_url, $platform, $title, $device) {

  // Set prefix for share URL's.
  $facebook['prefix'] = 'https://www.facebook.com/sharer/sharer.php?u=';
  $googleplus['prefix'] = 'https://plus.google.com/share?url=';
  $twitter['prefix'] = 'https://twitter.com/intent/tweet?text=';
  $whatsapp['prefix'] = 'whatsapp://send?text=';

  // Set UTM parameters for share URL's.
  $utm['campaign'] = 'share';
  $utm['medium'] = 'social';

  $facebook['utm'] = array (
    'utm_campaign' => $utm['campaign'],
    'utm_medium' => $utm['medium'],
    'utm_source' => 'facebook.com',
  );

  $googleplus['utm'] = array (
    'utm_campaign' => $utm['campaign'],
    'utm_medium' => $utm['medium'],
    'utm_source' => 'plus.google.com',
  );

  $twitter['utm'] = array (
    'utm_campaign' => $utm['campaign'],
    'utm_medium' => $utm['medium'],
    'utm_source' => 'twitter.com',
  );

  $whatsapp['utm'] = array (
    'utm_campaign' => $utm['campaign'],
    'utm_medium' => $utm['medium'],
    'utm_source' => 'whatsapp',
  );

  $mail['utm'] = array (
    'utm_campaign' => $utm['campaign'],
    'utm_medium' => 'email',
    'utm_source' => 'forward-mail',
  );

  switch($platform)
  {
    case 'facebook' :
      return $facebook['prefix'] . urlencode(url($share_url, array('query' => $facebook['utm'])));
      break;
    case 'googleplus' :
      return $googleplus['prefix'] . urlencode(url($share_url, array('query' => $googleplus['utm'])));
      break;
    case 'twitter' :
      return $twitter['prefix'] . urlencode(url($share_url, array('query' => $twitter['utm'])));
      break;
    case 'whatsapp' :
      return $whatsapp['prefix'] . urlencode(url($share_url, array('query' => $whatsapp['utm'])));
      break;
    case 'mail' :
      if (module_exists('forward') && $device == 'computer') {
        // We can't forward external URL's
        $share_url = current_path();
        return url('forward', array('query' => array('path' => $share_url)));
      }
      else {
        return 'mailto:?subject=' . $title . '&body=' . t('I thought this might interest you:') . '%0D%0A' .  urlencode(url($share_url, array('query' => $mail['utm'])));
      }
      break;
  }

}