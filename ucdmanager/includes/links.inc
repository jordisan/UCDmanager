<?php

/*
 * Compose HTML links to export view
 */
function ucdmanager_createExportLink($nodetype, $gid, $formats, $path=null) {
	$html = '<div class="export">' . t('Export as ');
	foreach ( $formats as $format ) {
		$html .= ' <a class="export fixed-nav" href="/export/project/' . $gid;
		if ($path) foreach ($path as $key => $value) {
			$html .= '/' . $key . '/' . $value;
		}
		$html .= '/' . $nodetype . '.' . $format . '">' . $format . '</a>';
	}
	$html .= '</div>';
	
	return $html;
}

/*
 * Compose the HTML link to create a new document
 */
function ucdmanager_createAddLink($nodetype, $link_text = 'new', $arguments=null) {
	$node = menu_get_object();

	$gid = ucdmanager_getGroupIdFromURL();
			// outside a group, non-superadmin users can create only project nodes
	if ( 	(!$gid && $nodetype != 'project' && !ucdmanager_isSuperadmin()) ||
			($gid && !ucdmanager_canCreateInGroup($gid, $nodetype)) 	) {
		return null;
	}
	
	$spanclass = "";
	$href = "";
	
	if ( $nodetype == 'technique-applied') {
		// adding a technique is a special case
		// uses a view to choose the specific technique and use the right form
		$href = '/select-technique/?';
		$spanclass = 'mainlink mainlinktech';
	} else {
		$href = '/node/add/' . $nodetype . '/?';
	}
	// links to techniques (in select technique view)
	if ( ucdmanager_isAppTechnique(str_replace('-', '_', $nodetype)) ) {
		$spanclass = 'mainlink mainlinktech';
	}
	// is a group (project) content?
	if ( $gid ) {
		$href .= '&amp;group_audience=' . $gid;
	}
	switch ($nodetype) {
		case 'heuristics':
			if ( $node->type == 'technique' ) {
				$href .= '&amp;field_techniques=' . $node->nid;
			}
			break;
	
		case 'single-heuristic':
			if ( $node->type == 'heuristics' ) {
				$href .= '&amp;field_heuristics=' . $node->nid; 
				$link_text = 'create single heuristic';
			} elseif ( $node->type == 'single_heuristic' ) {
				$href .= '&amp;field_heuristics=' . ucdmanager_getFirstValue($node, 'field_heuristics'); 
				$href .= '&amp;field_heuristic_number=' . ucdmanager_getFirstValue($node, 'field_heuristic_number'); 
				$link_text = 'create subheuristic';
			}
			break;
		
		case 'project':
			$spanclass = 'mainlink mainlinkgroup';
			break;
		// projects content
		case 'issue':
		case 'user-role':
		case 'persona':
		case 'usability-test-task':
		case 'abstract-use-case':
			if ( ucdmanager_isAppTechnique($node->type) ) {
				$href .= '&field_technique_app=' . $node->nid;
			} elseif ( $node->type == 'usability_test_task_exec' ) {
				$href .= '&field_technique_app=' . ucdmanager_getFirstValue($node, 'field_technique_app_single');
				$href .= '&field_tasks=' . $node->nid;
			}
			break;
			
		case 'usability-test-task-exec':
			if ( ucdmanager_isAppTechnique($node->type) ) {
				$href .= '&field_technique_app_single=' . $node->nid;
			} elseif ( $node->type == 'usability_test_task' ) {
				$href .= '&field_technique_app_single=' . ucdmanager_getFirstValue($node, 'field_technique_app');
				$href .= '&field_task=' . $node->nid;
			} 
			break;
			
		default:	// other content types (project related)
	}
	// additional arguments
	if ($arguments) foreach ($arguments as $key => $value) {
		$href .= '&amp;' . $key . '=' . $value;
	}
	
	return '<span class="' . $spanclass . '"><a class="new new-node fixed-nav" href="' . $href . '">' . $link_text . '</a></span>';
}

/*
 * Compose the HTML links to manage project members
 */
function ucdmanager_createMembersLinks() {
	$gid = ucdmanager_getGroupIdFromURL();
			// outside a group, non-superadmin users can create only project nodes
	if ( 	(!$gid && $nodetype != 'project' && !ucdmanager_isSuperadmin()) ||
			($gid && !ucdmanager_isProjectManager($gid)) 	) {
		return null;
	}
	$nid = ucdmanager_getNodeIdFromURL();
	$html  = '<a class="new new-node fixed-nav" href="/group/node/' . $nid . '/admin/people/add-user">' . t('new member') . '</a>';
	$html .= ' | <a class="fixed-nav" href="/group/node/' . $nid . '/admin/people">' . t('manage members &amp; roles') . '</a>';
	
	return $html;
}


/**
 * Show additional info from projects
 */
function ucdmanager_getProjectSummary($gid, $link=false) {
    $html = '';
	// return $html;  //TODO: solve performance issues
	
	// error_log('$gid=' . $gid);
	$techs = ucdmanager_getRelated('tech_app*', 'group_audience', $gid, $include_titles=false);
	$html .= count($techs) . " UCD techniques applied";

	if ($link) {
		$nid = og_load($gid)->etid;
		$html = '<a href="' . url('node/' . $nid) . '">' . $html .'</a>';
	}
	return $html;
}



/**
 * Show additional info from techs applied
 */
function ucdmanager_getTechAppSummary($nodeid, $nodetype = null, $link = false) {
	$html = '';	

	// return $html;  //TODO: solve performance issues

	if (! $nodetype ) {
		$node = node_load($nodeid);
		$type = $node->type;
	} else {
		$type = $nodetype;
	}
	if (!is_blank($nodetype)) switch ($nodetype) { 
		case 'tech_app_analysis_absusecases':
			$usecases = ucdmanager_getRelated('abstract_use_case', 'field_technique_app', $nodeid, $include_titles=false);
			$html .= count($usecases) . " use cases defined";
			break;
		case 'tech_app_analysis_userroles':	
			$roles = ucdmanager_getRelated('user_role', 'field_technique_app', $nodeid, $include_titles=false);
			$html .= count($roles) . " user roles defined";
			break;
		case 'tech_app_analysis_personas':	
			$personas = ucdmanager_getRelated('persona', 'field_technique_app', $nodeid, $include_titles=false);
			$html .= count($personas) . " personas defined";
			break;
		case 'tech_app_eval_accessib':
		case 'tech_app_eval_heuristic':
			$issues = ucdmanager_getRelated('issue', 'field_technique_app', $nodeid, $include_titles=false);
			$html .= count($issues) . " issues found";
			break;
		case 'tech_app_eval_thinkingaloud':	
			$tasks = ucdmanager_getRelated('usability_test_task', 'field_technique_app', $nodeid, $include_titles=false);
			$html .= count($tasks) . " tasks defined<br />";
			$execs = ucdmanager_getRelated('usability_test_task_exec', 'field_technique_app_single', $nodeid, $include_titles=false);
			$html .= count($execs) . " task executions<br />";
			$issues = ucdmanager_getRelated('issue', 'field_technique_app', $nodeid, $include_titles=false);
			$html .= count($issues) . " issues found";
			break;
	}
	if ($link) {
		$html = '<a href="' . url('node/' . $nodeid) . '">' . $html .'</a>';
	}
	return $html;
}

/*
 * All generated links
 */
function ucdmanager_url_outbound_alter(&$path, &$options, $original_path) {
	if ( !is_blank($options['external']) ) {
		// open external links in new windows
		$options['attributes']['target'] = '_blank';
	} else {
		$options['attributes']['target'] = '_self';
		// internal links => add arguments
		/*
		$nodeid = ucdmanager_getNodeIdFromURL();
		if ( !is_blank($nodeid) ) {
		  $options['query']['fromnodeid'] = $nodeid;
		}
		*/
	}
}

?>