<?php

/**
 * Preprocess and Process Functions SEE: http://drupal.org/node/254940#variables-processor
 * 1. Rename each function and instance of "adaptivetheme_subtheme" to match
 *    your subthemes name, e.g. if your theme name is "footheme" then the function
 *    name will be "footheme_preprocess_hook". Tip - you can search/replace
 *    on "adaptivetheme_subtheme".
 * 2. Uncomment the required function to use.
 * 3. Read carefully, especially within adaptivetheme_subtheme_preprocess_html(), there
 *    are extra goodies you might want to leverage such as a very simple way of adding
 *    stylesheets for Internet Explorer and a browser detection script to add body classes.
 */
 
 
 /*
  * Check whether the rendered page is the public home page (with a different design)
  */
 function UCDmanagertheme_isFrontDesign() {
	// global $user;
	// return (drupal_is_front_page() && $user->uid == 0) || current_path()=="home/public";
	return drupal_is_front_page();
 }
 
 function UCDmanagertheme_downloadFormat() {
	if (array_key_exists('format', $_GET) && !is_blank($format = $_GET['format']) ) {
		return $format;
	} else {
		return null;
	}
 }

/**
 * Override or insert variables into the html templates.
 */
function UCDmanagertheme_preprocess_html(&$vars) {
  /*
  print('preprocess_html() called: ');
  $calls = debug_backtrace();
   foreach ($calls as $call) {
	$call['args'] = null;
	print_r($call);
  }
  */
  // Load the media queries styles
  // Remember to rename these files to match the names used here - they are
  // in the CSS directory of your subtheme.
  $media_queries_css = array(
    'ucdmanagertheme.responsive.style.css',
    'ucdmanagertheme.responsive.gpanels.css'
  );
  load_subtheme_media_queries($media_queries_css, 'UCDmanagertheme');

	// show global message?
	$global_message_type = variable_get('global_message_type', '');
	if ( !is_blank($global_message_type) && isset($_SESSION) ) {
		drupal_set_message(variable_get('global_message_text', ''), $global_message_type);
	}

 /**
  * Load IE Stylesheets
  *
  * AT automates adding IE stylesheets, simply add to the array using
  * the conditional comment as the key and the stylesheet name as the value.
  *
  * See our online help: http://adaptivethemes.com/documentation/working-with-internet-explorer
  *
  * For example to add a stylesheet for IE8 only use:
  *
  *  'IE 8' => 'ie-8.css',
  *
  * Your IE CSS file must be in the /css/ directory in your subtheme.
  */
  /* -- Delete this line to add a conditional stylesheet for IE 7 or less.
  $ie_files = array(
    'lte IE 7' => 'ie-lte-7.css',
  );
  load_subtheme_ie_styles($ie_files, 'adaptivetheme_subtheme');
  // */
  
  // Add class for the active theme name
  /* -- Delete this line to add a class for the active theme name.
  $vars['classes_array'][] = drupal_html_class($theme_key);
  // */

  // Browser/platform sniff - adds body classes such as ipad, webkit, chrome etc.
  /* -- Delete this line to add a classes for the browser and platform.
  $vars['classes_array'][] = css_browser_selector();
  // */
  
   	// add body classes
	if ( function_exists('ucdmanager_isSuperadmin') && ucdmanager_isSuperadmin() ) {
		$vars['classes_array'][] = 'superadmin';
	}
	if ( function_exists('ucdmanager_getGroupIdFromURL') ) {
		$gid = ucdmanager_getGroupIdFromURL();
	} else {
		$gid = null;
	}
	if ( $gid || current_path()=="node/add/project") {
		$vars['classes_array'][] = 'groupcontent';
		if ( $gid && ! ucdmanager_canCreateInGroup($gid) ) {
			$vars['classes_array'][] = 'readonly'; 
		}
	} else {
		$vars['classes_array'][] = 'notgroupcontent';
	}

	// design for front page?
	if ( UCDmanagertheme_isFrontDesign() ||  current_path() == 'my' ) {
		$vars['classes_array'][] = 'frontdesign';
	} else {
		$vars['classes_array'][] = 'notfrontdesign';
	}
	
	// unpublished?
	if ($node = menu_get_object()) {
		if ( $node->status == '0' ) {
			$vars['classes_array'][] = 'unpublished';
		}
	}
	
	// class for techniques applied
	$type = '';
	if ($node) {
		$type = $node->type;
	} else if ( arg(0) == 'node' && arg(1) == 'add' ) {
		$type = str_replace('-', '_', arg(2));
	}
	if ( ucdmanager_isAppTechnique($type) ) {
		$vars['classes_array'][] = 'tech-app';
	}
	
	// error page?
	$status = drupal_get_http_header("status");
	if ( !is_blank($status) ) {
		$statuscode = substr($status, 0, 3);
		$statustype = substr($status, 0, 1);
		if ( $statustype == '4' || $status == '5' ) {
			$vars['classes_array'][] = 'page-error';
			$vars['classes_array'][] = 'page-error' . $statuscode;
		}
	}
	
	// download?
	$format = UCDmanagertheme_downloadFormat();
	if ( !is_blank($format) ) {
		$mime = null;
		switch ($format) {
			case 'doc':
				$mime = 'application/msword';
				break;
			case 'xls':
				$mime = 'application/vnd.ms-excel';
				break;
			case 'html':
				$mime = 'application/octet-stream';
				break;
		}
		if ( $mime ) {
			//$vars['classes_array'] = null;
			$vars['scripts'] = null;
			drupal_add_http_header('Content-Type', $mime . '; utf-8');
			drupal_add_http_header('Content-Disposition', 'attachment; filename=ucdmanager_' . end(arg()) . '.' . $format);
		}
	}

}

/* -- Delete this line if you want to use this function
function adaptivetheme_subtheme_process_html(&$vars) {
}
// */

/**
 * Override or insert variables into the page templates.
 */
function UCDmanagertheme_preprocess_page(&$vars) {
	$node = null;
	if ( array_key_exists('node', $vars) && !is_blank($vars['node']) ) {
		$node = $vars['node'];
	}
	if ( $node && !$vars['is_front'] ) {
		$vars['title'] = drupal_get_title() . ' <span class="nodetype">' . node_type_get_name($vars['node']) . '</span>';
	}
	$theme_path = drupal_get_path('theme', variable_get('theme_default', NULL)); 
	// theme logo
	$vars['linked_site_logo'] = '<a href="/" title="UCDmanager home"><img alt="UCD" src="/' . $theme_path . '/UCD.png" /></a>';
	// add variable to use it in template
	$vars['front_design'] = UCDmanagertheme_isFrontDesign();
	// demo?
	$vars['is_demo'] = function_exists('ucdmanager_getGroupIdFromURL') && ucdmanager_getGroupIdFromURL() == '1';
	if ($vars['is_demo']) {
		drupal_add_css($theme_path . '/joyride/joyride-ucdmanager.css', 'file');
		drupal_add_js($theme_path . '/joyride/jquery.cookie.js', 'file');
		drupal_add_js($theme_path . '/joyride/jquery.joyride-ucdmanager.js', 'file');
		drupal_add_js('jQuery(document).ready(function () {
			if ( jQuery.cookie("joyridedemoshowtips") != "false" || window.location.href.indexOf("showtips") > 0 ) {
				jQuery.cookie("joyridedemoshowtips", "true", { path: "/"} );
				jQuery("#block-views-help-helpblock").joyride({
					"tipLocation": "top",
					"tipClass": ' . ($node ? '"type-' . $node->type . '"': 'null') . '
				});
			}
		});', 'inline');
	}


	// to filter out own visits in Google Analytics
	if ( function_exists('ucdmanager_isSuperadmin') && ucdmanager_isSuperadmin() ) {
		drupal_add_js("
		if (typeof _gaq!='undefined') {
			_gaq.push(['_setCustomVar', 5, 'wasSuperadmin', 'true', 1]); // still doesn't work? http://www.lunametrics.com/blog/2012/02/27/track-moving-target/comment-page-1/#comment-651788
			_gaq.push(['_setVar', 'wasSuperadmin']); 
		}
		", array('type'=>'inline', 'weight'=>99));
	}

	// additional help text
	$vars['help_text'] = '';
	
	// previous (referrer) node
	if ( $node ) {
		$nodeid = ucdmanager_session("nodeid");
		if ( $nodeid != $node->nid ) {
			$ref_nodeid = ucdmanager_session("ref_nodeid");
			ucdmanager_session("ref_nodeid", $nodeid);
			ucdmanager_session("nodeid", $node->nid);
		}
	} else {
		ucdmanager_session("nodeid", null);
	}
}

function UCDmanagertheme_process_page(&$vars) {
}
// */

/**
 * Override or insert variables into the node templates.
 */
function UCDmanagertheme_preprocess_node(&$vars) {
	// add variable with last-modified info
	// http://11heavens.com/adding-last-edited-by-name-some-time-ago-info-to-node
	$vars['last_edit'] = null;
	if ($vars['page'] && ($vars['created'] != $vars['changed'])) {
		$edited_time = format_date($vars['changed']);
		$time_ago = format_interval(time() - $vars['changed'], 1);
		/*
		* If the last person who edited the node
		* is NOT the author who created the node.
		*/
		if ($vars['uid'] != $vars['revision_uid']) {
			// Let's get the THEMED name of the last editor.
			$user = user_load($vars['revision_uid']);
			$uservars['account'] = $user;
			$uservars['name'] = $user->name;
			// $uservars['link_path'] = 'user/' . $user->uid; //user_uri($user);
			$edited_by = theme('username', $uservars);
		}
		/*
		* If the last person who edited the node
		* is also the author who created the node,
		* we already have the THEMED name of that person.
		*/
		else {
			$edited_by = $vars['name'];
		}
		/* Adding the variable. */
		$vars['last_edit'] = t('Last edited by !name on @edited<br />(about @time ago)',
		array('!name' => $edited_by, '@edited' => $edited_time, '@time' => $time_ago));
	}
}

function UCDmanagertheme_process_node(&$vars) {
}
// */

/**
 * Override or insert variables into the comment templates.
 */
/* -- Delete this line if you want to use these functions
function adaptivetheme_subtheme_preprocess_comment(&$vars) {
}

function adaptivetheme_subtheme_process_comment(&$vars) {
}
// */

/**
 * Override or insert variables into the block templates.
 */
/* -- Delete this line if you want to use these functions
function adaptivetheme_subtheme_preprocess_block(&$vars) {
}

function adaptivetheme_subtheme_process_block(&$vars) {
}
// */

/**
 * Add the Style Schemes if enabled.
 * NOTE: You MUST make changes in your subthemes theme-settings.php file
 * also to enable Style Schemes.
 */
/* -- Delete this line if you want to enable style schemes.
// DONT TOUCH THIS STUFF...
function get_at_styles() {
  $scheme = theme_get_setting('style_schemes');
  if (!$scheme) {
    $scheme = 'style-default.css';
  }
  if (isset($_COOKIE["atstyles"])) {
    $scheme = $_COOKIE["atstyles"];
  }
  return $scheme;
}
if (theme_get_setting('style_enable_schemes') == 'on') {
  $style = get_at_styles();
  if ($style != 'none') {
    drupal_add_css(path_to_theme() . '/css/schemes/' . $style, array(
      'group' => CSS_THEME,
      'preprocess' => TRUE,
      )
    );
  }
}
// */

// http://drupal.org/node/1295794#comment-5415094
function UCDmanagertheme_breadcrumb($variables) {
  $args = arg();
  $breadcrumb = $variables['breadcrumb'];
  if (!is_blank($breadcrumb)) {
    // Provide a navigational heading to give context for breadcrumb links to
    // screen-reader users. Make the heading invisible with .element-invisible.
    $output = '<h2 class="element-invisible">' . t('You are here') . '</h2>';
	$output = '<ol class="breadcrumbs">';

    // show home?
	$show_breadcrumb_home = theme_get_setting('breadcrumb_home');
    if ($show_breadcrumb_home) {
      $output .= '<li class="active crumb home">' . $breadcrumb[0] . '</li>';
    }
	array_shift($breadcrumb);
	
	$node = null;
    // Load the node and if it is part of a group, add the group label into the breadcrumb
	if ( $args['0'] == 'node' && is_numeric($args['1']) ) {
		$node = node_load($args['1']);
	}
	if ( $node && $node->type == 'project' ) {
		// project nodes don't need breadcrumb
		return;
	}
	$group = null;
	$gid = ucdmanager_getGroupIdFromURL();
	if ( !is_blank($gid) ) {
		$group = og_get_group('group', $gid);
	} 
	if ( $group ) {
		$group_link = l($group->label, 'node/' . $group->etid);
		$output .= '<li class="active crumb group">'. $group_link .'</li>';
		// technique applied
		$tech_app = ucdmanager_getEntityValues($node, 'field_technique_app');
		if ( is_blank($tech_app) ) {
			$tech_app = ucdmanager_getEntityValues($node, 'field_technique_app_single');
		}
		if ( !is_blank($tech_app) ) {
			if ( count($tech_app)>1 ) {
				// item belongs to 2 or more techniques => check referrer
				$ref_nodeid = ucdmanager_session('ref_nodeid');
				if ( !is_blank($ref_nodeid) && in_array($ref_nodeid, $tech_app) ) {
					$tech_app = $ref_nodeid;
				} else {
					$tech_app = null;
				}
			} else {
				$tech_app = $tech_app[0];
			}
			if ( !is_blank($tech_app) ) {
				$tech_link = l(ucdmanager_getTitle($tech_app), 'node/' . $tech_app);
				$output .= '<li class="active crumb techapp">'. $tech_link .'</li>';
			}
		}
	} elseif ( $node ) { // other node types
		switch ($node->type) {
			case 'heuristics':
				$tech_nid = ucdmanager_getFirstValue($node, 'field_techniques');
				$output .= '<li class="active crumb technique">' . l(ucdmanager_getTitle($tech_nid), 'node/' . $tech_nid) . '</li>';
				break;
			case 'single_heuristic':
				$heuristics_nid = ucdmanager_getFirstValue($node, 'field_heuristics');
				$output .= '<li class="active crumb heuristics">' . l(ucdmanager_getTitle($heuristics_nid), 'node/' . $heuristics_nid) . '</li>';
				if ( !is_blank(ucdmanager_getFirstValue($node, 'field_heuristic_subnumber')) ) {
					// get node for parent single heuristic
					if ( $parent_h = ucdmanager_getParentSingleHeuristic($node) ) {
						$output .= '<li class="active crumb heuristics">' . l($parent_h->title, 'node/' . $parent_h->nid) . '</li>';
					}
				}
				break;
			case 'help_page':
				$output .= '<li class="active crumb help">' . l(t('Help'), 'help') . '</li>';
				break;
		}
	}
	
	$output .= '</ol>';
	
    return $output;
  }
}

// http://drupal.org/node/982034
function UCDmanagertheme_html_head_alter(&$head_elements) {
  unset($head_elements['system_meta_generator']);
}


// date fields
function UCDmanagertheme_date_combo($variables) {
  return theme('form_element', $variables);
}

?>
