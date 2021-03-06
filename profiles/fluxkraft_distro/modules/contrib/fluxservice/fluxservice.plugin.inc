<?php

/**
 * @file
 * Contains the fluxservice plugin system.
 */

/**
 * Retrieves all available plugin definitions for a given plugin type.
 *
 * @param $type
 *   The plugin type.
 * @param $plugin
 *   (optional) The machine-readable name of a plugin definition.
 *
 * @return array|null
 *   An array of plugin definitions keyed by their machine-readable name or a
 *   single plugin definition array if $plugin was given and a matching record
 *   could be found. Returns NULL if $plugin was given and there was no matching
 *   plugin definition.
 */
function fluxservice_get_plugin_info($type, $plugin = NULL) {
  // Try to retrieve the plugin definitions from the static cache.
  $info = &drupal_static("fluxservice:plugins:$type");

  // Try to retrieve the plugin definitions from the persistent cache if they
  // could not be retrieved from the static cache.
  if (!isset($info) && ($cache = cache_get("fluxservice:plugins:$type")) !== FALSE) {
    $info = $cache->data;
  }

  // If the plugin definitions could not be retrieved from the database cache
  // either we need to (re-)discover them from the info hook implementations.
  if (!isset($info)) {
    $info = fluxservice_discover_plugins($type);

    // Cache the plugin definitions in the database for future requests.
    cache_set("fluxservice:plugins:$type", $info);
  }

  // Either return the full array of plugin definitions or just a single record.
  if (!isset($plugin)) {
    return $info;
  }
  elseif (isset($info[$plugin])) {
    return $info[$plugin];
  }
}

/**
 * Discovers all available plugin definitions for a given plugin type.
 *
 * @param $type
 *   The plugin type.
 *
 * @return array
 *   An array of plugin definitions keyed by the machine-readable name.
 */
function fluxservice_discover_plugins($type) {
  $types = module_invoke_all('fluxservice_plugin_type_info');
  $type_info = $types[$type];
  $interface = $type_info['interface'];

  // Make sure all files possibly holding plugins are included.
  $files = array();
  foreach (fluxservice_get_plugin_directories() as $module => $directory) {
    foreach (glob(drupal_get_path('module', $module) . "/$directory/{$type_info['directory']}/*.{inc,php}", GLOB_BRACE) as $filename) {
      $files[$filename] = $filename;
      include_once $filename;
    };
  }

  $items = array();
  foreach (get_declared_classes() as $plugin_class) {
    if (is_subclass_of($plugin_class, $interface) && method_exists($plugin_class, 'getInfo')) {
      $reflection = new ReflectionClass($plugin_class);
      $file_path = str_replace(DRUPAL_ROOT . '/', '', $reflection->getFileName());
      if (array_key_exists($file_path, $files)) {
        $info = call_user_func(array($plugin_class, 'getInfo'));
        $info['class'] = $plugin_class;
        $info['module'] = _fluxservice_discover_module($file_path);
        $items[$info['name']] = $info;
      }
    }
  }
  // Give modules a chance to alter the array of plugin definitions;
  $type_info += array('alter_hook' => $type . '_plugin_info');
  drupal_alter($type_info['alter_hook'], $items);

  return $items;
}

/**
 * Determines the module providing the given class.
 */
function _fluxservice_discover_module($file_path) {
  $paths = &drupal_static(__FUNCTION__);

  if (!isset($paths)) {
    // Build up a map of modules keyed by their directory.
    foreach (system_list('module_enabled') as $name => $module_info) {
      $paths[dirname($module_info->filename)] = $name;
    }
  }
  // Go up the path until we match a module up.
  while (!isset($paths[$file_path]) && strpos($file_path, DIRECTORY_SEPARATOR) !== FALSE) {
    $file_path = dirname($file_path);
  }
  return isset($paths[$file_path]) ? $paths[$file_path] : FALSE;
}

/**
 * Implements hook_fluxservice_plugin_type_info().
 */
function fluxservice_fluxservice_plugin_type_info() {
  return array(
    'fluxservice_account' => array(
      'interface' => 'Drupal\fluxservice\Plugin\Entity\AccountInterface',
      'directory' => 'Service',
    ),
    'fluxservice_service' => array(
      'interface' => 'Drupal\fluxservice\Plugin\Entity\ServiceInterface',
      'directory' => 'Service',
    ),
    // Allow exposing entity types via entity classes.
    'fluxservice_entity' => array(
      'interface' => 'Drupal\fluxservice\Entity\EntityInterface',
      'directory' => 'Entity',
    ),
  );
}

/**
 * Retrieves all available service plugin definitions.
 *
 * @param $plugin
 *   (optional) The machine-readable name of a plugin definition.
 *
 * @return array|null
 *   An array of service plugin definitions keyed by their machine-readable name
 *   or a single plugin definition array if $plugin was given and a matching
 *   record could be found. Returns NULL if $plugin was given and there was no
 *   matching plugin definition.
 *
 * @see fluxservice_get_plugin_info()
 */
function fluxservice_get_service_plugin_info($plugin = NULL) {
  return fluxservice_get_plugin_info('fluxservice_service', $plugin);
}

/**
 * Retrieves all available account plugin definitions.
 *
 * @param $plugin
 *   (optional) The machine-readable name of a plugin definition.
 *
 * @return array|null
 *   An array of account plugin definitions keyed by their machine-readable name
 *   or a single plugin definition array if $plugin was given and a matching
 *   record could be found. Returns NULL if $plugin was given and there was no
 *   matching plugin definition.
 *
 * @see fluxservice_get_plugin_info()
 */
function fluxservice_get_account_plugin_info($plugin = NULL) {
  return fluxservice_get_plugin_info('fluxservice_account', $plugin);
}
