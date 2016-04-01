<?php

// returns an array of items [column]=>value
// http://dominiquedecooman.com/blog/drupal-7-tip-get-field-values-entity-objects-nodes-users-taxonomy
function field_fetch_field_values($entity_type, $entity, $field_name, $get_delta = NULL, $get_key = NULL) {
  $values = array();
  $value = null;
  if (isset($entity->$field_name) && !is_blank($entity->$field_name)) {
    foreach (field_get_items($entity_type, $entity, $field_name) as $delta => $item) {
	  $vals = array_values($item);
	  if ( count($vals) == 1 ) {
		$value = $vals[0];

   	  /*
	  $value = $item;
      $keys = array_keys($item);
      if (count($keys)==1) {
        $key = $keys[0];
        switch ($key) {
          case 'nid':
            $value = array_shift(entity_load('node', array($item[$key])));
            break;
          case 'uid':
            $value = array_shift(entity_load('user', array($item[$key])));
            break;
          case 'tid':
            $value = array_shift(entity_load('taxonomy_term', array($item[$key])));
            break;
          case 'vid':
            $value = array_shift(entity_load('taxonomy_vocabulary', array($item[$key])));
            break;
          case 'value':
            $value = $item['value'];
            break;
        }
		*/
      } else {
        if ($get_key && isset($item[$get_key])) {
          $value = $item[$get_key];
        } elseif (array_key_exists('value', $item)) {
          $value = isset($item['safe_value']) ? $item['safe_value'] : $item['value'];
        } elseif (array_key_exists('nid', $item)) {
		  $value = $item['nid'];
		} elseif (array_key_exists('gid', $item)) {
		  $value = $item['gid'];
		} elseif (array_key_exists('target_id', $item)) {
		  $value = $item['target_id'];
		}
      }
      $values[$delta] = $value;
    }
  }
  if (is_numeric($get_delta)) {
    return isset($values[$get_delta]) ? $values[$get_delta] : NULL;
  }
  // error_log('field_fetch_field_values()=' . print_r($values,true));
  return $values;
}

function ucdmanager_getEntityValues($entity, $fieldname, $from_url=false, $entity_type='node') {
	$values = field_fetch_field_values($entity_type, $entity, $fieldname);
	//error_log('ucdmanager_getEntityValues()=' . print_r($values,true));
	if ( is_blank($values) && $from_url && array_key_exists($fieldname, $_GET) ) {
		$values = array($_GET[$fieldname]);
	}
	return $values;
}

function ucdmanager_getFirstValue($entity, $fieldname, $entity_type='node') {
	$value = ucdmanager_getEntityValues($entity, $fieldname, $entity_type);
	if ( is_blank($value) ) {
		$value = null;
	} elseif ( is_array($value) ) {
		$value = $value[0];
	} 
	return $value;
	/*
	$value = null;
	$items = field_get_items('node', $entity, $fieldname);
	if ( $items ) {
		// $value = $items[0]['value'];
		$value = array_values($items['0']);
		$value = $value[0];
	}
	return $value;
	*/
}

/**
 * Return a list of values with the specified conditions
 */
function ucdmanager_getNodeValues($content_type, $return_field, $where_field, $where_value=null, $where_column='value', $order=null) {
	$list = array();
	
	// dd('$nodeid=' . $nodeid); // debug
	
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'node')
		  ->propertyCondition('status', 1) // published nodes
		  ->addTag('DANGEROUS_ACCESS_CHECK_OPT_OUT') // this bypass access checks (for performance) !!!  http://drupal.org/node/1597378
		  ->range(0, 100);
	if ( substr($content_type, -1, 1) == '*' ) {
		// get all subtypes
		$query->entityCondition('bundle', substr($content_type, 0, -1), 'STARTS_WITH');
	} else {
		$query->entityCondition('bundle', $content_type);
	}
	
	// if $where_field is an array => multiple conditions
	if ( ! is_array($where_field) ) {
		$where_fields = array($where_field);
		$where_values = array($where_value);
		$where_columns = array($where_column);
	} else {
		if ( ( count($where_field) != count($where_value) ) or ( count($where_field) != count($where_column) ) ) {
			trigger_error('params $where_field, $where_value and $where_column must be simple values or same-length arrays');
		}
		$where_fields = $where_field;
		$where_values = $where_value;
		$where_columns = $where_column;
	}
	$c = count($where_field);
	for($i = 0; $i < $c ; $i++) {
		if (!is_blank($where_columns[$i])) {
			if ( is_array($where_values[$i]) ) {
					$operator = 'IN';
			} else {
					$operator = '=';
			}
			if ( ! ucdmanager_isField($where_fields[$i]) ) {
				$query->propertyCondition($where_fields[$i], $where_values[$i], $operator);
			} else {
				$query->fieldCondition($where_fields[$i], $where_columns[$i], $where_values[$i], $operator);
			}
		}
	}
	
	// order?	
	if ( $order ) {
		$o = array();
		if ( ! is_array($order) ) {
			// we have just the name of the field
			$o['field'] = $order;
			$o['column'] = 'value';
			$o['direction'] = 'ASC'; 
		} else {
			$o = $order;
		}
		if ( ucdmanager_isField($order) ) {
			// http://drupal.org/node/1611438
			// $query->fieldOrderBy($o['field'], $o['column'], $o['direction']);
		} else {
			$query->propertyOrderBy($o['field'], $o['direction']);
		}
	}	
	
	$results = $query->execute();

	if ( $results ) {
		$nids = array_keys($results['node']);
		// error_log('$nids: ' . print_r($nids, true));
		if ( $return_field == 'nid' ) {
			// we already have nids; we don't need to load nodes
			$list = $nids;
		} else {
			$nodes = node_load_multiple($nids);
			foreach ( $nodes as $node ) {
				// $items = field_get_items('node', $node, $return_field);
				$items = ucdmanager_getEntityValues($node, $return_field);
				if ( $items ) foreach ( $items as $item ) {
					if ( is_array($item) ) {
						$list = array_merge($list, array_values($item));
					} else {
						$list[] = $item;
					}
				}
			}
		}
	}
	return array_unique($list);
}


/**
 * Get list of published node ids related to a node-id using 'Entity reference' fields or organic groups
 * if $field_name is a multivalue field, $nodeid must be an array 
 * if $include_titles=true, the returned array includes titles: array[nodeid]=title (useful for options lists in select fields) 
 */
function ucdmanager_getRelated($content_type, $field_name, $nodeid=null, $include_titles=true, $none_option=false, $exclude_ids=null, $order='title', $groupby=null) {
	$list = array();

	// include a 'none' option?
	if ( $none_option ) {
		if ( $include_titles ) {
			$list['_none'] = '- ' . t('None') . ' -';
		} else {
			$list[] = null;
		}
	}

	if ( $field_name == 'group_audience' ) {
		$column = 'gid';
	} else {
		$column = 'target_id';
	}	  
	//error_log('$nodeid: ' . print_r($nodeid, true));
	$nodeids = ucdmanager_getNodeValues($content_type, 'nid', $field_name, $nodeid, $column, $order);

	if ($nodeids) {
		// exclude ids?
		if ( !is_blank($exclude_ids) ) {
			if ( ! is_array($exclude_ids) ) {
				$array_exclude_ids = array($exclude_ids);
			} else {
				$array_exclude_ids = $exclude_ids;
			}
			$nodeids = array_diff($nodeids, $array_exclude_ids);
		}
		if ( !$include_titles ) {
			$list = $nodeids;
		} else {
			$nodes = node_load_multiple($nodeids);
			foreach ( $nodes as $node ) {
				if ( $groupby ) {
					$groupvalues = ucdmanager_getEntityValues($node, $groupby);
					if ( is_blank($groupvalues) ) {
						// empty $groupby
						$list[''][$node->nid] = $node->title;
					} else foreach ( $groupvalues as $val ) {
						$list[$val][$node->nid] = $node->title;
					}					
				} else {
					$list[$node->nid] = $node->title;
				}
			}
		}
	}
	// error_log('ucdmanager_getRelated() = ' . print_r($list, true));
	return $list;
}

function ucdmanager_isProjectEmpty($gid) {
	$related = views_get_view_result('project_related_og_', 'all_by_project', $gid);
	return empty($related);
}

function ucdmanager_isTechAppEmpty($nid) {
	$related = views_get_view_result('project_related_og_', 'all_by_techapp', $nid);
	return empty($related);
}

/*
 * Get list of members of a given group id
 */
function ucdmanager_getGroupMembers($gid, $include_names=true) {
	$list = array();

	$query = db_select('users', 'u');
	 
	$query
		->condition('u.uid', 0, '<>')
		->condition('u.status', 1, '=')
		->fields('u', array('uid', 'name'))
		->join('og_membership', 'ogm', "ogm.gid = :gid AND u.uid = ogm.etid AND ogm.entity_type = 'user'", array(':gid' => $gid));
	$results = $query->execute();
	
	if ( $results ) {
		// error_log(print_r($results, true));
		foreach ( $results as $user ) {
			if ( ! $include_names ) {
				$list[] = $user->uid;
			} else {
				$list[$user->uid] = $user->name;
			}
		}
	}
	// error_log(print_r($list, true));
	return $list;
}

/*
 * Get title of node
 */
function ucdmanager_getTitle($nid) {
	$title = '';
	$node = node_load($nid);
	if ( $node ) {
		$title = $node->title;
	}
	return $title;
}

function ucdmanager_getNodeIdFromURL()  {
	$nodeid = null;
	if ( arg(0)=='node' && is_numeric(arg(1)) ) {
		$nodeid = arg(1);
	} 
	return $nodeid;
}

function _getGroupIdFromURL() {
	// analyze additional URLs or parameters
	$gid = null;
	if ( array_key_exists('group_audience' , $_GET) ) {
		$gid = $_GET['group_audience'];
	} elseif ( arg(0)=='group' && arg(1) =='node' && is_numeric(arg(2)) ) {
		$group = og_get_group('node', arg(2));
		if ( $group ) {
			// is a group node (project)
			$gid = $group->gid; 
		}
	}
	return $gid;
}

function ucdmanager_getGroupIdFromURL()  {
	$gid = null;
	$nid = ucdmanager_getNodeIdFromURL();
	if ( $nid ) {
		$group = og_get_group('node', $nid);
		if ( $group ) {
			// is a group node (project)
			$gid = $group->gid;
		} else {
			// is not a group node
			// get the group of current node
			$node = node_load($nid);
			$gids = og_get_entity_groups('node', $node);
			if ( $gids ) {
				$gid = implode($gids, ',');
			}
		}
	} else {
		$gid = _getGroupIdFromURL();
	}
	return $gid;
}

function ucdmanager_getNodeFromGroupID($gid) {
	$group = og_load($gid);
	if ( $group ) {	
		$node = node_load(($group->etid));
	} else {
		$node = null;
	}
	return $node;
}

function ucdmanager_getParentSingleHeuristic($node) {
	// get node for parent single heuristic
	$nodeids = ucdmanager_getNodeValues('single_heuristic', 'nid', array('field_heuristics','field_heuristic_number'), array(ucdmanager_getFirstValue($node, 'field_heuristics'), ucdmanager_getFirstValue($node, 'field_heuristic_number')), array('target_id', 'value'));
	foreach ( $nodeids as $nid ) {
		$h = node_load($nid);
		if ( is_blank(ucdmanager_getFirstValue($h, 'field_heuristic_subnumber')) ) {
			return $h;
		}
	}
	return null;
}


?>
