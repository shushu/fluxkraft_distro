<?php

/**
 * @file
 * Main extension file for the 'development' extension.
 */

/**
 * Implements hook_extension_EXTENSION_registry_alter().
 */
function omega_extension_development_theme_registry_alter(&$registry) {
  if (omega_theme_get_setting('omega_demo_regions', TRUE) || omega_theme_get_setting('omega_livereload', TRUE) || omega_theme_get_setting('omega_browser_width_indicator', FALSE)) {
    $file = drupal_get_path('theme', 'omega') . '/includes/development/development.inc';

    $registry['html']['includes'][] = $file;
    $registry['html']['preprocess functions'][] = 'omega_extension_development_preprocess_html';

    if (omega_theme_get_setting('omega_demo_regions', TRUE)) {
      $registry['region']['includes'][] = $file;
      $registry['region']['preprocess functions'][] = 'omega_extension_development_preprocess_region';
    }
  }
}

/**
 * Implements hook_extension_EXTENSION_preproces_html().
 */
function omega_extension_development_preprocess_html(&$variables) {
  $path = drupal_get_path('theme', 'omega');

  if (omega_theme_get_setting('omega_livereload', TRUE)) {
    $options = array('absolute' => TRUE);
    $port = omega_theme_get_setting('omega_livereload_port', '35729');
    $host = omega_theme_get_setting('omega_livereload_host', 'localhost');
    $script = omega_theme_get_setting('omega_livereload_script', "$host:$port/livereload.js");
    $parsed = parse_url($script);

    if ((!isset($parsed['port'])) || ($parsed['port'] != $port)) {
      // If no port is specified the livereload.js script will use the default
      // port (35729).
      $options['query']['port'] = $port;
    }

    if ((!isset($parsed['host'])) || ($parsed['host'] != $host)) {
      // If no host is specificed livereload.js will use the host that it is
      // served from (e.g. if livereload.js is served from example.com it will
      // try to connect to a livereload server on example.com.
      $options['query']['host'] = $host;
    }

    // We need to add livereload.js as external script with an absolute path
    // because otherwise Drupal core messes with the query string.
    drupal_add_js(url($script, $options), array(
      'preprocess' => FALSE,
      'type' => 'external',
      'browsers' => array(
        'IE' => FALSE,
        '!IE' => TRUE,
      ),
    ));
  }

  if (omega_theme_get_setting('omega_browser_width_indicator', FALSE) || omega_theme_get_setting('omega_demo_regions', TRUE)) {
    drupal_add_css($path . '/css/omega.development.css', array('group' => CSS_THEME, 'weight' => -10, 'every_page' => TRUE));

    if (omega_theme_get_setting('omega_browser_width_indicator', FALSE)) {
      if (!module_exists('overlay') || (!$mode = overlay_get_mode()) || $mode == 'parent') {
        $variables['attributes_array']['class'][] = 'omega-browser-width-indicator';
        drupal_add_js($path . '/js/omega.indicator.min.js', array('group' => JS_THEME, 'weight' => -10, 'every_page' => TRUE));
      }
    }
  }
}

/**
 * Implements hook_extension_EXTENSION_preproces_region().
 */
function omega_extension_development_preprocess_region(&$variables) {
  if ($variables['debug'] = !empty($variables['elements']['#debug'])) {
    $class = drupal_html_class('region--debug--' . $variables['region']);
    drupal_add_css(".$class:before { content: \"{$variables['elements']['#name']}\"; }", array(
      'type' => 'inline',
      'group' => CSS_THEME,
      'weight' => 1000,
    ));

    $variables['attributes_array']['class'][] = 'region--debug';
    $variables['attributes_array']['class'][] = $class;
  }
}
