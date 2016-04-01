<?php

/**
 * Get the value of a field in a node being edited
 * returns an array
 */
function ucdmanager_getFieldValues($form, $form_state, $field_name) {
	$values = array();
	if ( isset($form_state['values'][$field_name]['und']) ) {
		$values = array_values($form_state['values'][$field_name]['und']);
	} else {
		// if not set, get the default values
		$default = $form[$field_name]['und']['#default_value'];
		if (! is_array($default) ) {
			$values[] = $default;
		} else foreach ($default as $d) {
			$values[] = $d;
		}
		// get the value from URL argument
		//if ( array_key_exists($field_name, $_GET) ) {
		//	$value = $_GET[$field_name];
		//}
	}
	// error_log($field_name . ': ' . print_r($values, true));
	return $values;
}
/**
 * Get the value of a field in a node being edited
 * returns a single value
 */
function ucdmanager_getFieldValue($form, $form_state, $field_name) {
	$values = ucdmanager_getFieldValues($form, $form_state, $field_name);
	if ( count($values)>0 ) {
		return $values[0];
	} else {
		return null;
	}
}


// FIELD FUNCTIONS

function _set_field_hidden(&$field, $value = null) {
	if ( $value ) {
		$field['#value'] = $value;
	} else if ( array_key_exists('#default_value', $field) ) {
		$field['#value'] = $field['#default_value'];
	}
	$field['#type'] = 'hidden';
}

function _set_field_disabled(&$field, $value = null) {
	if ( $value ) {
		$field['#value'] = $value;
	} else {
		$field['#value'] = $field['#default_value'];
	}
	$field['#disabled'] = True;
}

/*
 * Set a default value for a field
 */
function _set_default_value(&$field, $value = null, $value_id = null) {
	if ( array_key_exists('und', $field) ) {
		$f = &$field['und']; 
	} else {
		$f = &$field;
	}
	
	$default_value = $value;
	if ( array_key_exists('#type', $f) ) {
		// multiple default value?
		if ( $f['#type'] == 'checkboxes' && !is_array($default_value) ) {
			$default_value = array($default_value);
		}
	}
	$f['#default_value'] = $default_value;
	$f[0]['value']['#default_value'] = $default_value; // http://drupal.stackexchange.com/questions/21627/programmatically-set-the-default-value-for-a-field

	/*
	if ( $value_id && array_key_exists(0, $field) && is_array($field[0]) ) {
		// works for multiple value fields with autocomplete?
		// TODO: make this more general/robust
		if ( array_key_exists('target_id', $field[0]) ) {
			$field[0]['target_id']['#default_value'] = $value_id;
		}
	}
	*/
}


/**
 * ALTER (NODE) FORMS
 */


/*
 * Administration > configuration > system
 */ 
function ucdmanager_form_system_site_information_settings_alter(&$form, $form_state, $form_id) {
	// add a 'global message' option
	$form['global_message_type'] = array(
		'#type' => 'radios',
		'#title' => t('Global message: type'),
		'#description' => t('Text to be shown in every page of this site'),
		'#options' => array(''=>'(none)', 'status'=>'Status', 'warning'=>'Warning', 'error'=>'Error'),
		'#default_value' => variable_get('global_message_type', ''),
	);
	$form['global_message_text'] = array(
		'#type' => 'textfield',
		'#title' => t('Global message: text'),
		'#default_value' => variable_get('global_message_text', ''),
	);
}
 
/*
 * Add member to group (project) form
 */ 
function ucdmanager_form_og_ui_add_users_alter(&$form, $form_state, $form_id) {
	// hide additional fields (make no sense to users)
	_set_field_hidden($form['og_user']['og_add_user_membership_type'], 'og_membership_type_default');
	if (  ucdmanager_isProjectManager(ucdmanager_getGroupIdFromURL()) ) {
		_set_field_hidden($form['membership_fields']['og_membership_request']);
	}
	// show message for default role
	$suffix = '';
	if ( array_key_exists('#suffix', $form['og_user']['og_add_user']) ) {
		$suffix = $form['og_user']['og_add_user']['#suffix'];
	}
	$form['og_user']['og_add_user']['#suffix'] = $suffix . '<div class="form-item"><label>Role</label><p>Default role is <strong>read-only</strong></p></div>';
}

/*
 * Manage group members
 */ 
function ucdmanager_form_og_ui_user_admin_account_alter(&$form, $form_state, $form_id) {
	$form['options']['#weight'] = 99;
	$form['link_new'] = array(
		'#markup' => '<div><a class="new new-node fixed-nav" href="/group/node/' . arg(2) . '/admin/people/add-user">new member</a></div>',
		'#weight' => 100
	);
}

/*
 * Comment form
 */
function ucdmanager_form_comment_form_alter(&$form, &$form_state) {
	$form['actions']['submit']['#value'] = t('save comment');
}

/*
 * Connector buttons
 */
function ucdmanager_form_connector_button_form_alter(&$form, &$form_state) {
	foreach ( $form as $key => $value) {
		if ( is_array($value) && array_key_exists('#type', $value) && $value['#type'] == 'submit' ) {
			$form[$key]['#value'] = t('Log in with ' . $form[$key]['connector']['#value']['title']);
		}
	}
}


/*
 * User register form (avoid spam)
 */
function ucdmanager_form_user_register_form_alter(&$form, &$form_state, $form_id) {
  $form['#validate'][] = 'ucdmanager_validate_register_form';
  $form['account']['mail']['#description'] = t('Just for notifications. We WON\'T made it public or use it to spam you.');
  
}
function ucdmanager_validate_register_form(&$form, &$form_state) {
  if ( ! is_blank($form_state['values']['field_url']['und'][0]['value']) ||
       $form_state['values']['field_comments']['und'][0]['value'] == 'field_comments[und][0][value]' ) {
    http_response_code(404);
    die('SPAM detected');
    form_set_error('', t('Unauthorized'));
    watchdog('SPAM (ucdmanager module)', 'Spam catched<br/><br/>username = %user<br/>field_comments = %1<br/>field_url = %2',
             array('%user' => $form_state['input']['name'],
                   '%1' => $form_state['values']['field_comments']['und'][0]['value'], 
                   '%2' => $form_state['values']['field_url']['und'][0]['value']),
             WATCHDOG_NOTICE, NULL);
  }
}



/*
 * GENERAL FORM PROCESSING
 */
function ucdmanager_form_alter(&$form, $form_state, $form_id) {
	// code below is just for node forms (for the moment)
	if ( substr($form_id, -1 * strlen('_node_form')) != '_node_form' ) {
		return;
	}
		
	// hide promotion and revision options
	if ( ! ucdmanager_isSuperadmin() ) {
		// http://www.brightec.co.uk/blog/hiding-sticky-and-promoted-front-page-options-drupal
		$form['revision_information']['#access'] = FALSE;
		$form['options']['#access'] = FALSE;
		$form['additional_settings']['#access'] = FALSE;
	}
	
	// rename buttons
	$node = $form['#node'];
	if ( empty($node) || $node->status == '1') {
		$form['actions']['submit']['#value'] = t('save');
		$form['actions']['draft']['#value'] = t('save as draft (unpublished)');
	} else {
		$form['actions']['submit']['#value'] = t('save and publish');
		$form['actions']['draft']['#value'] = t('save (keep as draft)');
	}
	// hide extra buttons
	$form['actions']['preview']['#access'] = FALSE;
	// delete button
	// $form['actions']['delete']['#access'] = FALSE;
	
	
	// default values from URL
	foreach ($_GET as $argument => $value) {
		if ( array_key_exists($argument, $form) ) {
			_set_default_value($form[$argument], $value);
			//$form[$argument]['und']['#default_value'] = $value;
		}
	}
	
	// select authors-field values only from current project members
	// default value = current user
	global $user;
	if ( array_key_exists('field_authors', $form) ) {
		$gid = ucdmanager_getFieldValue($form, $form_state, 'group_audience');
		if ( $gid ) {
			$form['field_authors']['und']['#options'] = ucdmanager_getGroupMembers($gid, true);
		}
		_set_default_value($form['field_authors']['und'], $user->uid); //['und'][0]['target_id']['#default_value'] = $user->name;
	}

	// any document inside a project (group)
	global $is_template;
	$is_template = false;
	if ( array_key_exists('group_audience', $form) ) {
		if ( !is_blank($form['group_audience']['und']['#default_value']) ) {
			// don't allow editing group (project) in any form
			_set_field_disabled($form['group_audience']['und']);
		} else {
			// no default value for project => TEMPLATE
			$is_template = true;
			$form['actions']['submit']['#access'] = FALSE;
			$form['actions']['draft']['#access'] = FALSE;
		}
	}
	// group content always inherits group's (project's) visibility
	if ( array_key_exists('group_content_access', $form) ) {
		_set_field_hidden($form['group_content_access']['und'], '0');
	}

	// technique-applied forms
	if ( ucdmanager_isAppTechnique($form_id) ) {
		// title will be computed
		_set_field_hidden($form['title'], '<COMPUTED>');
		// don't allow editing technique in any technique-applied form
		if ( $form['field_technique']['und']['#default_value'] != '' ) {
			_set_field_disabled($form['field_technique']['und']);
		}
		// show available heuristics depending on technique
		$form['field_heuristics_used']['und']['#options'] = ucdmanager_getRelated('heuristics', 'field_techniques', ucdmanager_getFieldValue($form, $form_state, 'field_technique'));
		// delete button only if empty
		if ( isset($node->nid) && !ucdmanager_isTechAppEmpty($node->nid)) {
			$form['actions']['delete']['#access'] = FALSE;
		}
	}
	
	if ( $is_template ) {
	
		// adapt TEMPLATE fields depending on type
		foreach ( $form as $field => $value ) {
			// print_r($field); print_r(' = '); print_r($value); // DEBUG

			if ( is_array($value) && array_key_exists('#type', $value) ) {
				// store old values
				$values = array();
				if ( $value['#type'] == 'container' ) {
					if ( array_key_exists('#type', $value['und']) ) {
						$values = $value['und'];
					} else {
						if ( array_key_exists('value', $value['und'][0]) ) {
							$values = $value['und'][0]['value'];
						} else {
							$values = $value['und'][0];
							// special case: multiple fields with empty title
							if ( !array_key_exists('#title', $values) || is_blank($values['#title']) ) {
								$values['#title'] = $value['und']['#title'];
								$values['#description'] = $value['und']['#description'];
							}
						}
					}
				} else {
					$values = $value;
				}
				
				if ( $field == 'group_audience' ) {
					$values['#type'] = 'textfield';
					$values['#description'] = null;
				} else if ( is_array($value) && array_key_exists('#attributes', $value) && array_key_exists('class', $value['#attributes']) ) {
					// other fields
					if ( 	!array_key_exists('#type', $values) ||
							$values['#type'] == 'managed_file' ||					
							in_array('field-type-entityreference', $form[$field]['#attributes']['class']) ||
							in_array('field-type-image', $form[$field]['#attributes']['class']) || 
							in_array('field-widget-text-textarea', $form[$field]['#attributes']['class']) ||
							in_array('field-widget-text-textarea-with-summary', $form[$field]['#attributes']['class']) ) {
						$values['#type'] = 'textarea';
					}
				}
				$values['#value'] = '';
				$values['#default_value'] = '';
				$values['#properties'] = null;
				$values['#extended'] = null;
				$values['#after_build'] = null;	 					
				// some formatting
				if ( array_key_exists('#title', $values) ) {
					$values['#title'] = '<h2>' . $values['#title'] . '</h2>';
				}
				if ( array_key_exists('#description', $values) ) {
					$values['#description'] = '<i>' . $values['#description'] . '</i>';
				}
				if ( array_key_exists('#process', $values) ) {
					$values['#process'] = null;
				}
				if ( array_key_exists('_weight', $values) ) {
					$values['_weight'] = null;
				}
				if ( array_key_exists('#tree', $value) ) {
					$form[$field]['#tree'] = null;
				}
				if ( array_key_exists('und', $value) ) {
					if ( array_key_exists('#theme', $value['und']) ) {
						$form[$field]['und']['#theme'] = null;
					}
					if ( array_key_exists('add_more', $value['und']) ) {
						$form[$field]['und']['add_more'] = null;
					}
					if ( array_key_exists('#theme', $value['und']) && $value['und']['#theme'] == 'file_widget_multiple' ) {
						$form[$field]['und']['#theme'] = 'field_multiple_value_form';
						$form[$field]['und']['#cardinality'] = '1';
						unset($form[$field]['und']['#theme_wrappers']);
						unset($form[$field]['und']['#process']);
					}
				}
				// store new values in template
				if ( $value['#type'] == 'container' ) {
					if ( array_key_exists('#type', $value['und']) ) {
						$form[$field]['und'] = $values;
					} else {
						if ( array_key_exists('value', $value['und'][0]) ) {
							$form[$field]['und'][0]['value'] = $values;
						} else {	
							$form[$field]['und'][0] = $values;
						}
					}
				} else {
					$form[$field] = $values;
				}
			}
		}
		
	} else switch($form_id) {
		case 'project_node_form':
			// project is always a group
			_set_field_hidden($form['group_group']['und'], '1');
			// delete button only if empty
			$gid = ucdmanager_getGroupIdFromURL();
			if ( $gid && !ucdmanager_isProjectEmpty($gid) ) {
				$form['actions']['delete']['#access'] = FALSE;
			}
			break;
		
		case 'project_item_node_form':
			$form['field_item_category']['und']['#options'] = ucdmanager_allowedValuesItemCat(ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			break;
			
		case 'issue_node_form':
			$form['field_technique_app']['und']['#options'] = ucdmanager_getRelated('tech_app_eval*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			// project items (grouped by category)
			$form['field_items']['und']['#options'] = ucdmanager_getRelated('project_item', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'), $include_titles=true, $none_option=false, $exclude_ids=null, $order=null, $groupby='field_item_category');
			// available heuristics depend on the ones specified in the technique-applied
			$techs_app = ucdmanager_getFieldValues($form, $form_state, 'field_technique_app');
			if ( !empty($techs_app) ) {
				$heuristics = ucdmanager_getNodeValues('tech_app*', 'field_heuristics_used', 'nid', $techs_app, $where_column='value');
				$options = array();
				foreach ( $heuristics as $hid ) {
					$h = ucdmanager_getRelated('single_heuristic', 'field_heuristics', $hid);
					if ( !empty($h) ) {
						$options[ucdmanager_getTitle($hid)] = $h;
					}
				}
				$form['field_single_heuristics']['und']['#options'] = $options; //ucdmanager_getRelated('single_heuristic', 'field_heuristics', $heuristics);
			}
			// available task executions (grouped by technique applied)
			$options = array();
			foreach ( $techs_app as $t ) {
				$exec = ucdmanager_getRelated('usability_test_task_exec', 'field_technique_app_single', $t);
				if ( !empty($exec) ) {
					$options[ucdmanager_getTitle($t)] = $exec;
				}
			}
			$form['field_tasks']['und']['#options'] = $options;
			break;
			
		case 'user_role_node_form':
			$form['field_technique_app']['und']['#options'] = ucdmanager_getRelated('tech_app_analysis_userroles*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			// exclude itself from role reference fields
			$exclude_ids = null;
			$node = $form_state['node'];
			if ( isset($node->nid) ) {
				$exclude_ids = $node->nid;
			}
			$form['field_role_parent']['und']['#options'] = ucdmanager_getRelated('user_role', 'field_technique_app', ucdmanager_getFieldValue($form, $form_state, 'field_technique_app'), true, true, $exclude_ids);
			$form['field_role_related']['und']['#options'] = ucdmanager_getRelated('user_role', 'field_technique_app', ucdmanager_getFieldValue($form, $form_state, 'field_technique_app'), true, true, $exclude_ids);
			break;
			
		case 'persona_node_form':
			$form['field_technique_app']['und']['#options'] = ucdmanager_getRelated('tech_app_analysis_personas*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			// available roles (grouped by technique applied)
			$techs_roles = ucdmanager_getRelated('tech_app_analysis_userroles*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			$options = array();
			foreach ( $techs_roles as $nid => $title ) {
				$roles = ucdmanager_getRelated('user_role', 'field_technique_app', $nid);
				if ( !empty($roles) ) {
					$options[$title] = $roles;
				}				
			}
			$form['field_roles']['und']['#options'] = $options;
			break;
			
		case 'abstract_use_case_node_form':
			$form['field_technique_app']['und']['#options'] = ucdmanager_getRelated('tech_app_analysis_absusecases*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			// exclude itself from reference fields
			$exclude_ids = null;
			$node = $form_state['node'];
			if ( isset($node->nid) ) {
				$exclude_ids = $node->nid;
			}
			$form['field_usecase_parent']['und']['#options'] = ucdmanager_getRelated('abstract_use_case', 'field_technique_app', ucdmanager_getFieldValue($form, $form_state, 'field_technique_app'), true, true, $exclude_ids);
			// available roles (grouped by technique applied)
			$techs_roles = ucdmanager_getRelated('tech_app_analysis_userroles*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			$options = array();
			foreach ( $techs_roles as $nid => $title ) {
				$roles = ucdmanager_getRelated('user_role', 'field_technique_app', $nid);
				if ( !empty($roles) ) {
					$options[$title] = $roles;
				}	
			}
			$form['field_roles']['und']['#options'] = $options;
			break;
			
		case 'usability_test_task_node_form':
			$form['field_technique_app']['und']['#options'] = ucdmanager_getRelated('tech_app_eval_thinkingaloud*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			// available use cases (grouped by technique applied)
			$techs_usecases = ucdmanager_getRelated('tech_app_analysis_absusecases*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			$options = array();
			foreach ( $techs_usecases as $nid => $title ) {
				$cases = ucdmanager_getRelated('abstract_use_case', 'field_technique_app', $nid);
				if ( !empty($cases) ) {
					$options[$title] = $cases;
				}	
			}
			$form['field_usecases']['und']['#options'] = $options;
			break;
			
		case 'usability_test_task_exec_node_form':
			// title will be computed
			_set_field_hidden($form['title'], '<COMPUTED>');
			$form['field_technique_app_single']['und']['#options'] = ucdmanager_getRelated('tech_app_eval_thinkingaloud*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			$form['field_final_user']['und']['#options'] = ucdmanager_getRelated('final_user', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			$form['field_task']['und']['#options'] = ucdmanager_getRelated('usability_test_task', 'field_technique_app', ucdmanager_getFieldValue($form, $form_state, 'field_technique_app_single'));
			break;
			
		case 'final_user_node_form':
			// available roles (grouped by technique applied)
			$techs_roles = ucdmanager_getRelated('tech_app_analysis_userroles*', 'group_audience', ucdmanager_getFieldValue($form, $form_state, 'group_audience'));
			$options = array();
			foreach ( $techs_roles as $nid => $title ) {
				$roles = ucdmanager_getRelated('user_role', 'field_technique_app', $nid);
				if ( !empty($roles) ) {
					$options[$title] = $roles;
				}
			}
			$form['field_roles']['und']['#options'] = $options;
			break;
			
	}
/*
  if (isset($form['type']) && isset($form['#node'])) {
    // Use this check to match node edit form for a particular content type.
    // if ('mytype_node_form' == $form_id) {
      $form['#after_build'][] = '_ucdmanager_after_build';
    // }
  }
*/
}


/*
 * List of allowed values for project item categories (from project node)
 */
function ucdmanager_allowedValuesItemCat($gid = null) {
	if ( !$gid ) {
		$gid = ucdmanager_getGroupIdFromURL();
	}
	if ( is_blank($gid) ) {
		return null;
	}
	$node = ucdmanager_getNodeFromGroupID($gid);
	$categories = ucdmanager_getEntityValues($node, 'field_item_categories');
	$options = array();
	foreach ($categories as $cat) {
		$cat_array = explode('|', $cat);
		if ( count($cat_array) == 2 ) {
			$options[$cat_array[0]] = $cat_array[0] . ' ('. $cat_array[1] . ')';
		} else {
			$options[$cat] = $cat;
		}
	}
	return $options;
}

/*
function _callback_get_field_heuristics_used($form, $form_state) {
  return $form['field_heuristics_used'];
}
*/


/**
* Custom after_build callback handler.
*/
/*
function _ucdmanager_after_build($form, &$form_state) {
  // Use this one if the field is placed on top of the form.
	if ( array_key_exists ('group_audience' , $form) ) {
	// _ucdmanager_fix_hidden($form['group_audience']['und']);
	}
  // Use this one if the field is placed inside a fieldgroup.
//  _ucdmanager_fix_readonly($form['group_mygroup']['field_myfield']);
  return $form;
}
*/



/**
 * PRESAVE (NODE) FORMS
 */

function ucdmanager_node_presave($node) {

	if ( $node->type == 'project_item' )  {
	}

	// technique-applied forms
	if ( ucdmanager_isAppTechnique($node->type) ) {
		// compute title field
		$custom_title = ucdmanager_getFirstValue($node, 'field_title');
		if ( !$custom_title || $custom_title=="" ) {		
			$title = ucdmanager_getTitle(ucdmanager_getFirstValue($node, 'field_technique'));
			if ( ! is_blank($node->field_iteration['und']) ) {
				$iteration = ucdmanager_getFirstValue($node, 'field_iteration');	
				if ( $iteration != '' ) {
					$title .= ' (iter. ' . $iteration . ')';
				}
			}
		} else {
			$title = $custom_title;
		}
		$node->title = $title;
	} elseif ( $node->type == 'usability_test_task_exec' ) {
		// compute title field
		$final_user = node_load(ucdmanager_getFirstValue($node, 'field_final_user'));
		$task = node_load(ucdmanager_getFirstValue($node, 'field_task'));
		$node->title = 'task: ' . ucdmanager_getFirstValue($task, 'field_order') . ' | user: ' . $final_user->title . ' | completed: ' . ucdmanager_getFirstValue($node, 'field_task_completed');
	}
	
}

?>
